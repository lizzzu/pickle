#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "scanner.h"
#include "parser.hpp"

namespace Pickle {
    class Interpreter {
        Scanner scanner;
        Parser parser;

    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) { }

        int parse() {
            return parser.parse();
        }

        void switchInputStream(istream& is) {
            scanner.switch_streams(&is, nullptr);
        }

        friend class Parser;
        friend class Scanner;
    };
}

#endif
