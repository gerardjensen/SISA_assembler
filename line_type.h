#ifndef LINE_TYPE_H_INCLUDED
#define LINE_TYPE_H_INCLUDED

#include <stdlib.h>
#include <string>
#include <vector>

enum LineType{
	DIRECTIVE,VAR,INSTRUCTION,EMPTY,LABEL,
	DIRECTIVE_L, VAR_L, INSTRUCTION_L,
	NONE
};

extern LineType getLineType(std::string line);
extern std::vector<std::string> readFile(std::ifstream &file);
extern void formatFile(std::vector<std::string> &lines);

#endif
