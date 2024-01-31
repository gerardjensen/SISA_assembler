#include <cstdio>
#include "constants.h"
#include "info.h"

void printInfo(void)
{
	printf("SISA Assembler v%s for subject %s\nType -h for help\n", version, subject);
}

void printHelp(void)
{
	printf("Usage: %s [options] [flags] \n",prog_name);
	
	printf("\nOptions:\n");
	printf("  -p    Specifies the .asm file path\n");
	printf("  -h    Displays the help message\n");
	printf("  -o    Specifies the output file\n");
	printf("  -t    Specifies the memory position of the first .text element\n");
	printf("  -d    Specifies the memory position of the first .data element\n");
	printf("          if -d not provided, placing .data after .text\n");
	printf("          if -t not provided, placing .text after .data\n");
	printf("          if none of -t and -d provided default -d 0x0000\n");
	printf("  -l    Specifies the path for the labels output file\n");

	printf("\nFlags:\n");
	printf("  -v    Divides the output memory files in two (MEM0 and MEM1)\n");
	//printf("  -r    Prints the output as raw hex (written in ASCII) .hex file\n");
	//printf("        instead of a .bin file. This option ignores \n");

}
