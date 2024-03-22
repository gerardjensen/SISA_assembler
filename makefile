OPTIONS = -ansi -O2 -DNDEBUG -D_GLIBCXX_DEBUG -Wall -Wextra -Werror -Wno-sign-compare -Wshadow -std=c++11

sisasm: main.o constants.o info.o args_getter.o line_type.o  ram_saver.o line_processer.o
        g++ $(OPTIONS) -o sisasm $^

%.o: %.cc
        g++ -c $(OPTIONS) $<

clean:
        rm -f *.o sisasm
