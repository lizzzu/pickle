#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "utils.h"
#include "scanner.h"
#include "parser.hpp"

namespace Pickle {
    class Interpreter {
        Scanner scanner;
        Parser parser;

        vector<Declaration*> declarations;
        vector<Object*> objects;
        vector<Function*> functions;

        string checkForObjectErrors() {
            set<string> objectNames;
            map<string, vector<string>> graph;
            for (auto object : objects) {
                auto [name, members] = *object;
                if (objectNames.count(name))
                    return "type " + green(name) + " has already been defined";
                objectNames.insert(name);
                set<string> memberNames;
                for (auto [mType, mName] : members) {
                    if (memberNames.count(mName))
                        return "member " + green(mName) + " has already been defined inside type " + green(name);
                    memberNames.insert(mName);
                    graph[name].push_back(mType);
                }
            }
            auto cycle = checkForCycles(objectNames, graph);
            if (!cycle.empty()) {
                string error = "types [";
                for (string node : cycle)
                    error += green(node) + ", ";
                error.pop_back();
                error.pop_back();
                error += "] form a cycle of dependencies";
                return error;
            }
            return "";
        }

        bool checkForErrors() {
            const string error = checkForObjectErrors();
            if (error == "") return false;
            cerr << red("PICKLE: ") << error << '\n';
            return true;
            // checkForUndefinedObjects();
            // checkForUndefinedMembers();
            // checkForUndefinedFunctions();
            // checkForUndefinedVariables();
            // checkForAlreadyDefinedObjects();
            // checkForAlreadyDefinedFunctions();
            // checkForAlreadyDefinedVariables();
            // checkForCyclicDependenciesBetweenObjects();
            // checkForCyclicDependenciesBetweenFunctions();
            // checkForTypeErrors();
            // checkForBreakContinueErrors();
        }

    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) { }

        int parse() {
            const int res = parser.parse();
            if (res) return 1;
            if (checkForErrors()) return 1;
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
