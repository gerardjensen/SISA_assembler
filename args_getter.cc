#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "constants.h"
#include "args_getter.h"
#include "info.h"

const uint8_t MEM_DIV      = 0b00000001;
//const uint8_t RAW_HEX      = 0b00000010;
const uint8_t PRINT_LABELS = 0b00000100;
const uint8_t T_MEM_SET    = 0b10000000;
const uint8_t D_MEM_SET    = 0b01000000;
const uint8_t O_FILE_SET   = 0b00100000;

const char *help_flag = "-h";

void getArgs(const int &argc, char** &argv,
			  char* &inputFilePath,
			  char* &outputFilePath,
			  char* &t_mem_pos_str,
			  char* &d_mem_pos_str,
			  char* &labelsFilePath,
			  uint8_t &flags)
{
	flags = 0;
	
	if(argc == 1)
	{
		printInfo();
		exit(0);
	}

	else if(argc == 2){
		if(strcmp(argv[1],help_flag) == 0)
		{
			printHelp();
			exit(0);
		}
		else unrecognisedFlag(argv[1]);
	}

	bool input_set = false;
	char* arg;
	for(int i = 1; i<argc; i++)
	{
		arg = argv[i];
		if(arg[0] != '-') unrecognisedFlag(arg);

		switch(arg[1]){
			case 'p':
				if(input_set) goto repeated_flags;
				inputFilePath = next_arg_secure(i, argc, argv);
				input_set = true;
				break;
			case 'o':
				if(flags & O_FILE_SET) goto repeated_flags;
				outputFilePath = next_arg_secure(i, argc, argv);
				flags = flags | O_FILE_SET;
				break;
			case 't':
				if(flags & T_MEM_SET) goto repeated_flags;
				t_mem_pos_str = next_arg_secure(i, argc, argv);
				flags = flags | T_MEM_SET;
				break;
			case 'd':
				if(flags & D_MEM_SET) goto repeated_flags;
				d_mem_pos_str = next_arg_secure(i, argc, argv);
				flags = flags | D_MEM_SET;
				break;
			case 'l':
				if(flags & PRINT_LABELS) goto repeated_flags;
				labelsFilePath = next_arg_secure(i, argc, argv);
				flags = flags | PRINT_LABELS;
				break;
			case 'v':
				if(flags & MEM_DIV) goto repeated_flags;
				flags = flags | MEM_DIV;
				break;
			//case 'r':
			//	flags = flags | RAW_HEX;
			//	break;

			default:
				unrecognisedFlag(arg);
				break;
		}
	}
	if(!input_set){ 
		printf("%s: Missing input file\n", prog_name);
		exit(1);
	}
	return;

repeated_flags:
	printf("%s: repeated flags\n",prog_name);
	exit(1);
}

char* next_arg_secure(int &i, const int &argc, char** &argv)
{
	if(i < argc-1) {
		i++;
		return argv[i];
	}
	printf("%s: Missing arguments for %s\n", prog_name, argv[i]);
	exit(1);
}

void unrecognisedFlag(char* &c)
{
	printf("%s: The flag %s is unrecognised or arguments missing\n",prog_name,c);
	exit(1);
}
