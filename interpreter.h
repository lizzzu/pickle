#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "scanner.h"
#include "parser.hpp"

namespace Pickle {
    class Interpreter {
        Scanner scanner;
        Parser parser;

        vector<Declaration*> declarations;
        vector<Object*> objects;
        vector<Function*> functions;

    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) { }

        int parse() {
            const int res = parser.parse();
            if (res) return res;
            return 0;
        }

        void switchInputStream(istream& is) {
            scanner.switch_streams(&is, nullptr);
        }

        void pushDeclaration(Declaration* declaration) {
            declarations.push_back(declaration);
        }

        void pushObject(Object* object) {
            objects.push_back(object);
        }

        void pushFunction(Function* function) {
            functions.push_back(function);
        }

        friend class Parser;
        friend class Scanner;
    };
}

#endif
