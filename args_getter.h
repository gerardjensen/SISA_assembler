#ifndef ARGS_H_INCLUDED
#define ARGS_H_INCLUDED

#include <stdint.h>

extern void getArgs(const int &argc, char** &argv,
			  char* &inputFilePath,
			  char* &outputFilePath,
			  char* &t_mem_pos_str,
			  char* &d_mem_pos_str,
			  char* &labelsFilePath,
			  uint8_t &flags);
extern void unrecognisedFlag(char* &c);
extern char* next_arg_secure(int &i, const int &argc, char** &argv);

extern const uint8_t MEM_DIV;
extern const uint8_t RAW_HEX;
extern const uint8_t PRINT_LABELS;
extern const uint8_t T_MEM_SET;
extern const uint8_t D_MEM_SET;
extern const uint8_t O_FILE_SET;

#endif
