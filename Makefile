all:
	flex -o scanner.cpp scanner.l
	bison -o parser.cpp parser.y
	g++ -std=c++17 main.cpp scanner.cpp parser.cpp -o pickle -Wno-register

clean:
	rm scanner.cpp
	rm parser.cpp
	rm parser.hpp
	rm pickle
