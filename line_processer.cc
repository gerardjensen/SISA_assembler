#include <cstring>
#include <fstream>
#include <iomanip> // for int_to_hex
#include <regex>

#include "line_processer.h"
#include "constants.h"

uint16_t hexval(char c)
{
	if(c >= '0' && c<= '9') return c - '0';
	if(c >= 'A' && c<= 'F') return 10 + c - 'A';
	return 10 + c - 'a';
}

uint16_t hex2u16(std::string s)
{
	uint16_t sum = 0;
	uint16_t prod = 1;
	for(int i = s.length()-1; i>=2; i--)
	{
		sum += hexval(s[i]) * prod;
		prod*=0x10;
	}
	return sum;
}

bool isHex(const char* &s) 
{
	int n = strlen(s);
	if(n < 3 || n > 6 || !(s[0] == '0' && s[1] == 'x')) return false;
    
    for(int i = 2; i < n; i++) 
    { 
        char ch = s[i]; 
        if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'F') && (ch < 'a' || ch > 'f'))  
            return false; 
    }
	return true;
}

bool isDec(const char* &s, const bool &can_be_negative)
{
	int n = strlen(s); 

	if(n > 6 || n == 0 || (*s == '-' && !can_be_negative)) return false;
    for(int i = 0; i < n; i++) 
    { 
        char ch = s[i]; 
        if (!((ch >= '0' && ch <= '9') || (ch == '-' && i == 0)))  
            return false; 
    }
	return true;	
}

template< typename T >
std::string int_to_hex( T i ,const int &digits)
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(digits) << std::uppercase 
         << std::hex << i;
  return stream.str();
}

std::string num2hex(const char* s, const bool &can_be_negative)
{
	if(isHex(s))
		return s;
	
	if(isDec(s,can_be_negative))
	{
		int32_t a = std::stoi(s);
		if(a > 0xFFFF || a < -0x10000) goto invalid;

		if(a < 0)
			a += 0x10000;
		
		return int_to_hex(a,4);
	}

invalid:
	return "NaN";
}

std::string num2hex_uint8(const char* s, const bool &can_be_negative)
{
	if(isHex(s) && strlen(s) <= 4)
		return s;
	
	if(isDec(s,can_be_negative))
	{
		int32_t a = std::stoi(s);
		if(a > 0xFF || a < -0x100) goto invalid;

		if(a < 0)
			a += 0x100;
		
		return int_to_hex(a,2);
	}

invalid:
	return "NaN";
}

std::string exclude_label(const std::string &line)
{
	std::string::size_type pos = line.find_first_of(':'); // never returns std::string::npos 
	return line.substr(pos+1);
}

std::string get_label(const std::string &line)
{
	std::string::size_type pos = line.find_first_of(':'); // never returns std::string::npos 
	return line.substr(0,pos);
}

std::vector<std::string> split(const std::string &str, const char &separator) {
	std::vector<std::string> strings;
	int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++) {
        if (str[i] == separator || i == str.size()) {
            endIndex = i;
			std::string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }

	return strings;
}

void increment_mem_pos_safe(uint16_t &mem_pos, uint16_t n, const int &line_c, const char* oline)
{
	if(mem_pos + n >= 0x10000)
		printf("%s: Warning! memory position overflow with line %d: %s\n",prog_name,line_c,oline);
	
	mem_pos+=n;
}

void processDirective(const std::string &line, const int &line_c, const std::string &oline,std::vector<GVar> &gvars, uint16_t &mem_pos, uint8_t* mem)
{
	std::string::size_type pos = line.find_first_of(' ');
	std::string directive,args_raw; 
	if(pos == std::string::npos)
	{
		directive = line;
	}
	else
	{
		directive = line.substr(0,pos);
		args_raw = line.substr(pos+1);
	}

	if(directive == ".byte")
	{
		std::vector<std::string> args = split(args_raw,',');
		
		for(int i = 0; i<args.size(); i++)
		{
			std::string arg_hex = num2hex_uint8(args[i].c_str(),true);
			if(arg_hex == "NaN")
			{
				printf("%s: invalid byte '%s' at line %d: %s\n",prog_name,args[i].c_str(),line_c,oline.c_str());
				exit(1);
			}

			uint8_t b = (uint8_t)hex2u16(arg_hex);
			mem[(mem_pos+i) % 0x10000] = b;
		}
		increment_mem_pos_safe(mem_pos,args.size(), line_c, oline.c_str());
	}
	else if(directive == ".word")
	{
		std::vector<std::string> args = split(args_raw,',');

		if(mem_pos % 2 == 1) printf("%s: Warning! Writing word at odd memory position (0x%04X) at line %d: %s\n\tAdd .even directive before .word\n",prog_name,mem_pos,line_c,oline.c_str());
		
		for(int i = 0; i<args.size(); i++)
		{
			std::string arg_hex = num2hex(args[i].c_str(),true);
			if(arg_hex == "NaN")
			{
				printf("%s: invalid word '%s' at line %d: %s\n",prog_name,args[i].c_str(),line_c,oline.c_str());
				exit(1);
			}

			uint16_t wd = hex2u16(arg_hex);
			mem[(mem_pos+i*2) % 0x10000] = wd & 0xFF;
			mem[(mem_pos+i*2+1) % 0x10000] = wd >> 8;
		}
		increment_mem_pos_safe(mem_pos,args.size()*2, line_c, oline.c_str());
	} 
	else if(directive == ".even")  
			increment_mem_pos_safe(mem_pos, mem_pos % 2, line_c, oline.c_str());
	else if(directive == ".space")
	{
		std::vector<std::string> args = split(args_raw,',');
		if(args.size() > 2)
		{
			printf("%s: Too much arguments at line %d: %s\n",prog_name,line_c,oline.c_str());
			exit(1);
		}
		uint16_t n;
		int arg0_res = eval_expression(args[0], false,true, gvars, n);

		std::string arg1hex = "";
		if(args.size() == 2)
			arg1hex = num2hex_uint8(args[1].c_str(),true);

		if(arg0_res != 0 || arg1hex == "NaN")
		{
			printf("%s: Invalid arguments at line %d: %s\n",prog_name,line_c,oline.c_str());
			exit(1);
		}

		uint8_t b = args.size() == 1 ? 0x00 : (uint8_t)hex2u16(arg1hex);
		for(int i = 0; i<n; i++)
			mem[(mem_pos + i) % 0x10000] = b;
		
		increment_mem_pos_safe(mem_pos, n, line_c, oline.c_str());
	}
	else if (directive == ".set")
	{
		std::vector<std::string> args = split(args_raw,',');
		if(args.size() > 2)
		{
			printf("%s: Too many arguments at line %d: %s\n",prog_name,line_c,oline.c_str());
			exit(1);
		}
		else if(args.size() < 2)
		{
			printf("%s: Too few arguments at line %d: %s\n",prog_name,line_c,oline.c_str());
			exit(1);
		}

		attemptAddVariable(line_c,oline,gvars,args[0],args[1]);
	}
	else
	{
		printf("%s: Invalid directive '%s' at line %d: %s\n",prog_name,directive.c_str(),line_c,oline.c_str());
		exit(1);
	}
}

bool contains_GVar(const std::vector<GVar> &v, const GVar &o)
{
	int n = v.size();
	for(int i = 0; i<n; i++)
		if(v[i].name == o.name) return true;

	return false;
}

bool validGVarName(const std::string &s)
{
	const std::regex pattern("^[a-zA-Z_][a-zA-Z0-9_]*$"); // alphanumeric with _ also avaliable and first charcater not being a number
	if(s.empty() || s == "lo" || s == "h") return false;
	return std::regex_match(s, pattern);
}

int addGVar(std::vector<GVar> &gvars, const GVar &var)
{
	if(!validGVarName(var.name))
		return -1;

	if(contains_GVar(gvars, var))
		return -2;

	gvars.push_back(var);

	return 0;
}

void processLabel(const std::string &line, const int &line_c, const std::string &oline,std::vector<GVar> &gvars, uint16_t &mem_pos)
{
	std::string label = get_label(line);

	GVar var;
	var.name = label;
	var.value = mem_pos;
	var.isLabel = true;

	int result = addGVar(gvars, var);
	
	switch(result)
	{
		case -1:
			printf("%s: Invalid label name '%s' at line %d: %s\n\tOnly alphanumerical and '_' charcters allowed\n",prog_name,label.c_str(),line_c,oline.c_str());
			exit(1);
			break;
		case -2:
			printf("%s: %s already exists at line %d: %s\n",prog_name,var.isLabel ? "Label" : "Variable",line_c,oline.c_str());
			exit(1);
			break;
		default:
			break;
	}
}

int eval_gvar(std::string var_name, const std::vector<GVar> &gvars, uint16_t &res)
{
	int n = gvars.size();
	for(int i = 0; i<n; i++)
		if(gvars[i].name == var_name)
		{
			res = gvars[i].value;
			return 0;
		}
	return -1;
}

int get_label(const std::string &label_name, bool &is_label, uint16_t &value, const std::vector<GVar> &gvars)
{
	int n = gvars.size();
	for(int i = 0; i<n; i++)
		if(gvars[i].name == label_name)
		{
			value = gvars[i].value;
			is_label = gvars[i].isLabel;
			return 0;
		}
	return -1;
}

int next_op(const std::string &s,int c_pos)
{
	int n = s.size();
	for(int i = c_pos+1; i<n; i++)
	{
		if(s[i] == '*' || s[i] == '+') return i;
	}
	return n;
}

int prev_op(const std::string &s,int c_pos)
{
	for(int i = c_pos-1; i>=0; i--)
	{
		if(s[i] == '*' || s[i] == '+') return i;
	}
	return -1;
}

int eval_expression(std::string str, const bool &can_be_negative, const bool &apply_operators, const std::vector<GVar> &gvars, uint16_t &res)
{
	if(str.find_first_of("lo(") == 0)
	{
		std::string sub_expression = str.substr(3,str.size()-4);
		uint16_t value;
		int result = eval_expression(sub_expression, true, false, gvars, value);
		if(result != 0) return -1;
		
		res = (uint8_t)(value & 0xFF);
		return 0;
	} 
	else if(str.find_first_of("hi(") == 0)
	{
		std::string sub_expression = str.substr(3,str.size()-4);
		uint16_t value;
		int result = eval_expression(sub_expression, true, false, gvars, value);
		if(result != 0) return -1;
		
		res = (uint8_t)(value >> 8);
		return 0;
	}

	if(apply_operators)
	{
		std::string::size_type mult_pos = str.find_first_of('*');

		while(mult_pos != std::string::npos)
		{
			int init_A = prev_op(str, mult_pos);
			int fin_B = next_op(str,mult_pos);

			std::string argA = str.substr(init_A+1,mult_pos-init_A-1);
			std::string argB = str.substr(mult_pos+1,fin_B-mult_pos-1);
			
			uint16_t evalA,evalB;
			int res_codeA,res_codeB;
			
			res_codeA = eval_expression(argA, can_be_negative, true, gvars, evalA);
			res_codeB = eval_expression(argB, can_be_negative, true, gvars, evalB);
			
			if(res_codeA == 0 && res_codeB == 0)
			{
				std::string prev_str = str.substr(0,init_A+1);
				std::string post_str = str.substr(fin_B);
				str = prev_str + std::to_string(evalA * evalB) + post_str;
				mult_pos = str.find_first_of('*');
			}
			else return -1;
		}

		std::string::size_type add_pos= str.find_first_of('+');
		
		while(add_pos != std::string::npos)
		{
			int init_A = prev_op(str, add_pos);
			int fin_B = next_op(str,add_pos);

			std::string argA = str.substr(init_A+1,add_pos-init_A-1);
			std::string argB = str.substr(add_pos+1,fin_B-add_pos-1);
			
			uint16_t evalA,evalB;
			int res_codeA,res_codeB;
			
			res_codeA = eval_expression(argA, can_be_negative, true, gvars, evalA);
			res_codeB = eval_expression(argB, can_be_negative, true, gvars, evalB);
			
			if(res_codeA == 0 && res_codeB == 0)
			{
				std::string prev_str = str.substr(0,init_A+1);
				std::string post_str = str.substr(fin_B);
				str = prev_str + std::to_string(evalA + evalB) + post_str;
				add_pos = str.find_first_of('+');
			}
			else return -1;
		}
	}

	const char* s = str.c_str();
	if(isDec(s,can_be_negative))
	{
		res = (uint16_t)std::stoul(s);
		return 0;
	} else if (isHex(s))
	{
		res = (uint16_t)std::stoul(s, nullptr, 0);
		return 0;
	}

	uint16_t gvar_val;
	int gvar_res = eval_gvar(str,gvars,gvar_val);
	if(gvar_res == 0 && ((int16_t)gvar_val >= 0 || can_be_negative)) 
	{
		res = gvar_val;
		return 0;
	}
	return -1;
}

void processVariable(const std::string &line, const int &line_c, const std::string &oline,std::vector<GVar> &gvars)
{
	std::string::size_type eq_pos = line.find_first_of('='); // Can't return std::string::npos
	std::string var_name = line.substr(0,eq_pos);
	std::string expression = line.substr(eq_pos+1);

	attemptAddVariable(line_c,oline,gvars,var_name,expression);
}

void attemptAddVariable(const int &line_c, const std::string &oline,std::vector<GVar> &gvars, const std::string &var_name, const std::string &expression)
{
	uint16_t value;
	int res = eval_expression(expression, true, true, gvars, value);
	if(res != 0)
	{
		printf("%s: Invalid variable expression at line %d: %s\n",prog_name,line_c,oline.c_str());
		exit(1);
	}
	GVar var;
	var.name = var_name;
	var.value = value;
	var.isLabel = false;
	res = addGVar(gvars, var);
	
	switch(res)
	{
		case -1:
			printf("%s: Invalid variable name '%s' at line %d: %s\n\tOnly alphanumerical and '_' charcters allowed\n",prog_name,var_name.c_str(),line_c,oline.c_str());
			exit(1);
			break;
		case -2:
			printf("%s: %s already exists at line %d: %s\n",prog_name,var.isLabel ? "Label" : "Variable",line_c,oline.c_str());
			exit(1);
			break;
		default:
			break;
	}
}

bool isRegister(const std::string &s)
{
	return s.length() == 2 && s[0] == 'R' && s[1] >='0' && s[1] < '8';
}

const uint8_t I11_9_SHIFT = 9;
const uint8_t I8_6_SHIFT = 6;
const uint8_t I5_3_SHIFT = 3;

uint16_t get_R(const std::string &s, const uint8_t &shift)
{
	uint8_t r = s[1] - '0';
	return (uint16_t)(r << shift);
}

void assemble3R(uint16_t &assembled, const std::vector<std::string> &args, const uint8_t &f, const uint8_t &I_15_12, const int &line_c, const std::string &oline)
{
	if(args.size() != 3)
	{
		printf("%s: incorrect arguments at line %d: %s\n\tShould only be 3\n", prog_name,line_c,oline.c_str());
		exit(1);
	}

	assembled = assembled | (uint16_t)(I_15_12 << 12);
	std::string Rd = args[0];
	std::string Ra = args[1];
	std::string Rb = args[2];

	if(!isRegister(Rd) || !isRegister(Ra) || !isRegister(Rb))
	{
		printf("%s: incorrect arguments at line %d: %s\n\tArguments are not registers\n", prog_name,line_c,oline.c_str());
		exit(1);
	}
	
	assembled = assembled | get_R(Rd,I5_3_SHIFT);
	assembled = assembled | get_R(Ra,I11_9_SHIFT);
	assembled = assembled | get_R(Rb,I8_6_SHIFT);
	assembled = assembled | f;
}

void assemble2R(uint16_t &assembled, const std::string &Ra,const std::string &Rbd,const std::string &n_str, const uint8_t &I_15_12, const int &line_c, const std::string &oline, const std::vector<GVar> &gvars)
{
	if(!isRegister(Ra) || !isRegister(Rbd))
	{
		printf("%s: incorrect arguments at line %d: %s\n\tArguments are not registers\n", prog_name,line_c,oline.c_str());
		exit(1);
	}

	uint16_t n;
	int res = eval_expression(n_str, true, false, gvars, n);
	if(res != 0)
	{
		printf("%s: Invalid number at line %d: %s\n",prog_name,line_c,oline.c_str());
		exit(1);
	}
	if((int16_t)n > 0x1F || (int16_t)n < -0x20)
	{
		printf("%s: Invalid number at line %d: %s\n\tNumbers must be bewteen -32 and 31\n",prog_name,line_c,oline.c_str());
		exit(1);
	}

	assembled = assembled | (uint16_t)(I_15_12 << 12);
	assembled = assembled | get_R(Ra,I11_9_SHIFT);
	assembled = assembled | get_R(Rbd,I8_6_SHIFT);
	assembled = assembled | (n & 0x3F);
}

void assemble1R(uint16_t &assembled, std::vector<std::string> &args, const uint8_t &I_15_12, const uint8_t &I8, const int &line_c, const std::string &oline, const std::vector<GVar> &gvars, const uint16_t &mem_pos)
{
	if(args.size() != 2)
	{
		printf("%s: incorrect arguments at line %d: %s\n\tShould only be 2\n", prog_name,line_c,oline.c_str());
		exit(1);
	}

	if(I_15_12 == 0b1010 && I8 == 1) std::swap(args[0],args[1]); // 1010xxx1 -> OUT

	if(!isRegister(args[0]))
	{
		printf("%s: incorrect arguments at line %d: %s\n\tFirst argument is not a register\n", prog_name,line_c,oline.c_str());
		exit(1);
	}
	uint16_t n;
	int res = eval_expression(args[1], true, false, gvars, n);
	if(res != 0)
	{
		printf("%s: incorrect arguments at line %d: %s\n\tSecond argument is not a valid number\n", prog_name,line_c,oline.c_str());
		exit(1);
	}

	bool is_label;
	uint16_t value;
	get_label(args[1], is_label, value, gvars);
	if(is_label && (I_15_12 == 0b1000))n = (value-mem_pos)/2-1; // 1000 -> BZ/BNZ
	
	if(I_15_12 == 0b1010 && n > 255) // 1010 -> IN or OUT
	{
		printf("%s: Invalid number at line %d: %s\n\tPorts available from 0 to 255\n", prog_name,line_c,oline.c_str());
		exit(1);
	}
	else if((int16_t)n < -128 || (int16_t)n > (I_15_12 == 0b1001 ? 255 : 127))
	{
		printf("%s: Invalid number at line %d: %s\n\tNumber too big or too small\n", prog_name,line_c,oline.c_str());
		exit(1);
	}
	
	n&=0xFF;

	assembled = assembled | (uint16_t)(I_15_12 << 12);
	assembled = assembled | get_R(args[0],I11_9_SHIFT);
	assembled = assembled | (I8 & 0x01) <<  8; 
	assembled = assembled | n;
}

void processIntruction(const std::string &line, const int &line_c, const std::string &oline, const std::vector<GVar> &gvars, uint16_t &mem_pos, uint8_t* mem)
{
	if(mem_pos % 2 == 1) printf("%s: Warning! Writing instruction at odd memory position (0x%04X) at line %d: %s\n\tAdd .even directive before .text or start .text in an even position\n",prog_name,mem_pos,line_c,oline.c_str());
	std::string::size_type pos = line.find_first_of(' ');
	std::string instruction,args_raw; 
	instruction = line.substr(0,pos);
	args_raw = line.substr(pos+1);
	std::vector<std::string> args = split(args_raw,',');
	
	uint16_t assembled = 0x0000;
	uint8_t b0;
	uint8_t b1;
	
	if(instruction == "AND") assemble3R(assembled, args, 0b000,0b0000,line_c,oline);
	else if(instruction == "OR") assemble3R(assembled, args, 0b001,0b0000,line_c,oline);
	else if(instruction == "XOR") assemble3R(assembled, args, 0b010,0b0000,line_c,oline);
	else if(instruction == "NOT") assemble3R(assembled, args, 0b011,0b0000,line_c,oline);
	else if(instruction == "ADD") assemble3R(assembled, args, 0b100,0b0000,line_c,oline);
	else if(instruction == "SUB") assemble3R(assembled, args, 0b101,0b0000,line_c,oline);
	else if(instruction == "SHA") assemble3R(assembled, args, 0b110,0b0000,line_c,oline);
	else if(instruction == "SHL") assemble3R(assembled, args, 0b111,0b0000,line_c,oline);
	else if(instruction == "CMPLT")	assemble3R(assembled, args, 0b000,0b0001,line_c,oline);
	else if(instruction == "CMPLE")	assemble3R(assembled, args, 0b001,0b0001,line_c,oline);
	else if(instruction == "CMPEQ")	assemble3R(assembled, args, 0b011,0b0001,line_c,oline);
	else if(instruction == "CMPLTU") assemble3R(assembled, args, 0b100,0b0001,line_c,oline);
	else if(instruction == "CMPLEU") assemble3R(assembled, args, 0b101,0b0001,line_c,oline);
	else if(instruction == "ADDI")
	{
		if(args.size() != 3) goto invalid_syntax;
		assemble2R(assembled, args[1], args[0], args[2], 0b0010,line_c,oline,gvars);
	} else if(instruction == "LD" || instruction == "LDB")
	{
		if(args.size() != 2) goto invalid_syntax;
		
		std::string Ra,n_str;
		std::string::size_type fpr = args[1].find_first_of('(');
		std::string::size_type lpr = args[1].find_first_of(')');
		if(fpr == std::string::npos || lpr == std::string::npos || lpr < args[1].length()-1)
			goto invalid_syntax;

		Ra = args[1].substr(fpr+1,2);
		n_str = args[1].substr(0,fpr);

		assemble2R(assembled, Ra, args[0], n_str, instruction == "LD" ? 0b0011 : 0b0101, line_c,oline,gvars);
	} else if(instruction == "ST" || instruction == "STB")
	{
		if(args.size() != 2) goto invalid_syntax;
		
		std::string Ra,n_str;
		std::string::size_type fpr = args[0].find_first_of('(');
		std::string::size_type lpr = args[0].find_first_of(')');
		if(fpr == std::string::npos || lpr == std::string::npos || lpr < args[1].length()-1)
			goto invalid_syntax;

		Ra = args[0].substr(fpr+1,2);
		n_str = args[0].substr(0,fpr);

		assemble2R(assembled, Ra, args[1], n_str, instruction == "ST" ? 0b0100 : 0b0110, line_c,oline,gvars);
	} else if(instruction == "JALR")
	{
		if(args.size() != 2 || !isRegister(args[1]) || !isRegister(args[0])) goto invalid_syntax;
		assemble2R(assembled, args[1], args[0], "0x0B", 0b0111,line_c,oline,gvars); // n can be random
	} else if(instruction == "BZ") assemble1R(assembled, args, 0b1000, 0,line_c,oline,gvars,mem_pos);
	else if(instruction == "BNZ") assemble1R(assembled, args, 0b1000, 1,line_c,oline,gvars,mem_pos);
	else if(instruction == "MOVI") assemble1R(assembled, args, 0b1001, 0,line_c,oline,gvars,mem_pos);
	else if(instruction == "MOVHI") assemble1R(assembled, args, 0b1001, 1,line_c,oline,gvars,mem_pos);
	else if(instruction == "IN") assemble1R(assembled, args, 0b1010, 0,line_c,oline,gvars,mem_pos);
	else if(instruction == "OUT") assemble1R(assembled, args, 0b1010, 1,line_c,oline,gvars,mem_pos);
	else 
	{
		printf("%s: unknown instruction '%s' at line %d\n", prog_name,instruction.c_str(),line_c);
	}
	b0 = assembled & 0xFF;
	b1 = (uint8_t) (assembled >> 8);
	
	mem[mem_pos] = b0;
	increment_mem_pos_safe(mem_pos, 1, line_c, oline.c_str());
	mem[mem_pos] = b1;
	increment_mem_pos_safe(mem_pos, 1, line_c, oline.c_str());
	return;

invalid_syntax:
	printf("%s: Invalid syntax at line %d: %s\n",prog_name,line_c,oline.c_str());
	exit(1);
}
