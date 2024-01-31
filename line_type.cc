#include <fstream>
#include <regex>

#include "line_type.h"
#include "constants.h"

LineType getLineType(std::string line){
	if(line == "" || line == " ") return EMPTY;
	if(line[0] == '.') return DIRECTIVE;

	size_t colon_pos = line.find_first_of(':');
	size_t colon_pos_last = line.find_last_of(':');
	size_t fst_space = line.find_first_of(' ');

	if(colon_pos != colon_pos_last) return NONE; // more than one :, also to prevent innecesary recursion

	if(colon_pos != std::string::npos)
	{
		if(fst_space == std::string::npos || fst_space > colon_pos) {
		// label; 
			line = line.substr(colon_pos+1);
			LineType pre_type = getLineType(line);
			switch(pre_type){
				case DIRECTIVE:
					return DIRECTIVE_L;
				case INSTRUCTION:
					return INSTRUCTION_L;
				case VAR:
					return VAR_L;
				case EMPTY:
					return LABEL;
				case NONE:
				default:
					return NONE;
			}
		} else return NONE;
	}

	size_t equal_pos = line.find_first_of('=');
	if(equal_pos != std::string::npos)
	{
		if(equal_pos == line.length()-1) return NONE;
		return VAR;
	}

	return INSTRUCTION;
}

void removeSideWhitespaces(std::string &s, const std::string &c){
	std::string c_safe = c;
	if(c == ")") c_safe = "\\)";
	if(c == "+") c_safe = "\\+";
	if(c == "*") c_safe = "\\*";
	std::string str_pattern = "\\s*" + c_safe + "\\s*";
	std::regex pattern(str_pattern);
    s = std::regex_replace(s, pattern, c);
}

std::string format(std::string &s)
{
	std::regex pattern("\\s+");
    s = std::regex_replace(s, pattern, " ");

	removeSideWhitespaces(s, ",");
	removeSideWhitespaces(s, "=");
	removeSideWhitespaces(s, "*");
	removeSideWhitespaces(s, "+");
	removeSideWhitespaces(s, ":");
	removeSideWhitespaces(s, ")");

	pattern = std::regex("\\(\\s+");
    s = std::regex_replace(s, pattern, "(");

	pattern = std::regex("hi\\s+\\(");
    s = std::regex_replace(s, pattern, "hi(");

	pattern = std::regex("lo\\s+\\(");
    s = std::regex_replace(s, pattern, "lo(");

    // Remove leading and trailing spaces
	size_t semi_colon = s.find_first_of(';');
	if(semi_colon != std::string::npos) s = s.substr(0,semi_colon);
    
	size_t firstNonSpace = s.find_first_not_of(" ");
    size_t lastNonSpace = s.find_last_not_of(" ");
     
    if (firstNonSpace != std::string::npos && lastNonSpace != std::string::npos) s = s.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
 
	return s;
}

std::vector<std::string> readFile(std::ifstream &file)
{
	if(!file) {
		printf("%s: Input file not found\n", prog_name);
		exit(1);
	}

	std::vector<std::string> lines;
	std::string line;
	while(getline(file,line))
		lines.push_back(line);

	file.close();

	return lines;
}

void formatFile(std::vector<std::string> &lines)
{
	for(int i = 0; i<lines.size(); i++)
		lines[i] = format(lines[i]);
}
