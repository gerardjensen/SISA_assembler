simplemake: main.cc constants.cc info.cc args_getter.cc line_type.cc  ram_saver.cc line_processer.cc
	g++ -ansi -O2 -DNDEBUG -D_GLIBCXX_DEBUG -Wall -Wextra -Werror -Wno-sign-compare -Wshadow -std=c++11 main.cc constants.cc info.cc args_getter.cc line_type.cc  ram_saver.cc  line_processer.cc -o sisasm
