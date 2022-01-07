all:
	flex -o scanner.cpp scanner.l
	bison -o parser.cpp parser.y
	g++ main.cpp scanner.cpp parser.cpp -o pickle

clean:
	rm scanner.cpp
	rm parser.cpp
	rm parser.hpp
	rm pickle
