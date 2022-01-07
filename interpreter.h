#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "scanner.h"
#include "parser.hpp"

namespace Pickle {
    class Interpreter {
        Scanner scanner;
        Parser parser;

        vector<Object*> objects;
        vector<Function*> functions;

    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) { }

        int parse() {
            const int res = parser.parse();
            if (res) return res;
            for (auto object : objects) {
                auto [name, memberTypes, memberNames] = *object;
                cout << "OBJECT: " << name << '\n';
                for (int i = 0; i < int(memberTypes->size()); i++) {
                    cout << memberTypes->at(i) << ' ';
                    cout << memberNames->at(i) << '\n';
                }
            }
            return 0;
        }

        void switchInputStream(istream& is) {
            scanner.switch_streams(&is, nullptr);
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
