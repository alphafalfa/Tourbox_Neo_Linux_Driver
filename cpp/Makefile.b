tourbox: tourbox.cpp
	gcc -g -O1 -Wall -Wextra -Wpedantic -Werror -lconfuse -lfltk -o tourbox tourbox.cpp

# uinput_helper: uinput_helper.hpp
# 	gcc -g -O1 -Wall -Wextra -Wpedantic -Werror -lconfuse -lfltk -o uinput_helper.o uinput_helper.h

clean:
	rm -fv *.o
