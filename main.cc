#include <fstream>

#include "args_getter.h"
#include "line_type.h"
#include "ram_saver.h"
#include "line_processer.h"
#include "constants.h"
#include "main.h"

int main(int argc, char** argv)
{
	char* inputFilePath, *outputFilePath, *labelsFilePath;
	char* t_mem_pos_str, *d_mem_pos_str;
	uint8_t flags;
	uint16_t d_mem_pos,t_mem_pos;
	uint8_t RAM[0x10000] = {0x00};

	getArgs(argc, argv, inputFilePath, outputFilePath, t_mem_pos_str, d_mem_pos_str,labelsFilePath, flags);		

	if(flags & D_MEM_SET)
	{
		std::string d_mem_pos_str_hex = num2hex(d_mem_pos_str,false);
		if(d_mem_pos_str_hex == "NaN" || *d_mem_pos_str == '-')
		{
			printf("%s: invalid .data address\n",prog_name);
			return 1;
		}
		d_mem_pos = hex2u16(d_mem_pos_str_hex);
	} 
	else d_mem_pos = 0x0000;
	if(flags & T_MEM_SET)
	{
		std::string t_mem_pos_str_hex = num2hex(t_mem_pos_str,false);
		if(t_mem_pos_str_hex == "NaN" || *t_mem_pos_str == '-')
		{
			printf("%s: invalid .text address\n",prog_name);
			return 1;
		}
		t_mem_pos = hex2u16(t_mem_pos_str_hex);
	} else t_mem_pos = 0x0000; // unused value 

	std::ifstream inputFile(inputFilePath);
	std::vector<std::string> orig_lines = readFile(inputFile);	

	std::vector<std::string> input_lines = orig_lines;
	formatFile(input_lines);

	if(flags & T_MEM_SET && !(flags & D_MEM_SET)) // Start .data after .text, so append after instructions
	{											  // if here, d_mem_pos is 0
		d_mem_pos = t_mem_pos;
		for(int i = 0; i<input_lines.size(); i++)
		{
			LineType t = getLineType(input_lines[i]);
			if(t != INSTRUCTION && t != INSTRUCTION_L) continue;
			if(d_mem_pos + 2 >= 0x10000)
			{
				d_mem_pos = 0;
				printf("%s: Warning! memory position overflow with line %d: %s\n\tStart the .data before or make the .text and/or .data smaller\n",prog_name,i+1,orig_lines[i].c_str());
			}
			else d_mem_pos+=2;
		}
	}

	std::vector<GVar> gvars;

	int GLOBAL = -1;
	int DATA = 0;
	int TEXT = 1;
	int END = 2;
	int state = -1;

	int line_c = 0;
	uint16_t mem_pos;

	int n = orig_lines.size();
	for(int i = 0; i<n; i++)
	{
		std::string line = input_lines[i];
		std::string oline = orig_lines[i];
		line_c++;
		LineType type = getLineType(line);
		
		if(state == END && type != EMPTY)
		{
			printf("%s: invalid content at line %d: %s\n\tNo instructions allowed after .end\n",prog_name,line_c,oline.c_str());
			exit(1);
		}

		switch(type)
		{
			case DIRECTIVE:
directive_check:
				if(state == TEXT)
				{
					if(line == ".end") state = END;
					else
					{
						printf("%s: invalid directive at line %d: %s\n\tDirectives don't apply in .text\n",prog_name,line_c,oline.c_str());
						exit(1);	
					}
				}
				else if(state == GLOBAL)
				{
					if(line == ".data")
					{
						state = DATA;
						mem_pos = d_mem_pos;
					}else if(line.rfind(".set ") == 0)
						processDirective(line,line_c,oline,gvars,mem_pos,RAM);
					else
					{
						printf("%s: invalid directive at line %d: %s\n\tFirst directive must be .data\n\t(Excluding .set)\n", prog_name, line_c, oline.c_str());
						exit(1);
					}
				}
				else
				{
					if(line == ".text")
					{
						state = TEXT;
						if(flags & T_MEM_SET) mem_pos = t_mem_pos;
					}
					else
						processDirective(line,line_c,oline,gvars,mem_pos,RAM);
				}
				break;
			case VAR:
				if(state != GLOBAL)
				{
					printf("%s: invalid global variable at line %d: %s\n\tGlobal variables must be placed before .data and .text\n",prog_name,line_c,oline.c_str());
					exit(1);
				}
				processVariable(line,line_c,oline,gvars);
				break;
			case INSTRUCTION:
instruction_check:
				if(state != TEXT)
				{
					printf("%s: invalid instruction at line %d: %s\n\tInstructions must be placed after the .text directive\n",prog_name,line_c,oline.c_str());
					exit(1);
				}
				processIntruction(line,line_c,oline,gvars,mem_pos,RAM);
				break;
			case LABEL:
				processLabel(line,line_c,oline,gvars,mem_pos);
				break;
			case EMPTY:
				continue;
				break;
			case NONE:
				printf("%s: Invalid instruction at line %d: %s\n",prog_name,line_c,oline.c_str());
				exit(1);
				break;
			case DIRECTIVE_L:
				processLabel(line,line_c,oline,gvars,mem_pos);
				line = exclude_label(line);
				goto directive_check;
				break;
			case VAR_L:
				printf("%s: Invalid instruction at line %d: %s\n\tGlobal variables can't have labels\n",prog_name,line_c,oline.c_str());
				exit(1);
				break;
			case INSTRUCTION_L:
				processLabel(line,line_c,oline,gvars,mem_pos);
				line = exclude_label(line);
				goto instruction_check;
				break;
		}
	}

	if(state != END)
	{
		printf("%s: Missing .end directive\n",prog_name);
		exit(1);
	}
	
	saveRam(RAM, 0x10000, (flags & O_FILE_SET) ? outputFilePath : (char*)defalut_output_file_path, flags);
	
	if((flags & PRINT_LABELS) == 0) return 0;

	std::ofstream outputFileLabels(labelsFilePath);
	if(!outputFileLabels)
	{
		printf("%s: Couldn't write to labels output file\n", prog_name);
		exit(1);
	}

	for(int i = 0; i<gvars.size(); i++)
	{
		if(!gvars[i].isLabel) continue;
		char buff[100];
		snprintf(buff,sizeof(buff),"'%s': 0x%04X\n", gvars[i].name.c_str(), gvars[i].value);
		outputFileLabels << buff;
	}
	
	outputFileLabels.flush();
	outputFileLabels.close();

	return 0;
}
