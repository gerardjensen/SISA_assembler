#ifndef LINE_PROCESSER_H
#define LINE_PROCESSER_H

#include <stdlib.h>
#include <string>
#include <vector>

struct GVar{
	std::string name;
	uint16_t value;
	bool isLabel;
};

void processIntruction(const std::string &line, const int &line_c, const std::string &oline,const std::vector<GVar> &gvars, uint16_t &mem_pos, uint8_t* mem);
void processVariable(const std::string &line, const int &line_c, const std::string &oline,std::vector<GVar> &gvars);
void attemptAddVariable(const int &line_c, const std::string &oline,std::vector<GVar> &gvars, const std::string &var_name, const std::string &expression);
int eval_expression(std::string str, const bool &can_be_negative, const bool &apply_operators, const std::vector<GVar> &gvars, uint16_t &res);
std::string exclude_label(const std::string &line);
void processLabel(const std::string &line, const int &line_c, const std::string &oline,std::vector<GVar> &gvars, uint16_t &mem_pos);
void processDirective(const std::string &line, const int &line_c, const std::string &oline,std::vector<GVar> &gvars, uint16_t &mempos, uint8_t* mem);
std::string num2hex(const char* s, const bool &can_be_negative);
uint16_t hex2u16(std::string s);

#endif
