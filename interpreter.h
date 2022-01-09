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

        vector<string> checkForCycles(set<string>& nodes, map<string, vector<string>>& graph) {
            map<string, string> father;
            vector<string> cycle;
            function<void(string, string)> dfs = [&](string node, string fath) {
                father[node] = fath;
                for (string nghb : graph[node])
                    if (father[nghb] == "")
                        dfs(nghb, node);
                    else if (cycle.empty()) {
                        while (node != "$") {
                            cycle.push_back(node);
                            node = father[node];
                        }
                        reverse(cycle.begin(), cycle.end());
                    }
            };
            for (string node : nodes)
                if (father[node] == "")
                    dfs(node, "$");
            return cycle;
        }

        string idk() {
            map<string, vector<string>> graph;
            set<string> objectNames;
            for (auto object : objects) {
                auto [name, memberTypes, memberNames] = *object;
                if (objectNames.count(name))
                    return "type " + name + " has already been defined";
                objectNames.insert(name);
                set<string> members;
                for (int i = 0; i < int(memberTypes.size()); i++) {
                    if (members.count(memberNames[i]))
                        return "member " + memberNames[i] + " has already been defined inside type " + name;
                    members.insert(memberNames[i]);
                    graph[name].push_back(memberTypes[i]);
                }
            }
            auto cycle = checkForCycles(objectNames, graph);
            if (!cycle.empty()) {
                string error = "types [";
                for (string node : cycle)
                    error += node + ", ";
                error.pop_back();
                error.pop_back();
                error += "] form a cycle of dependencies";
                return error;
            }
            return "";
        }

        bool checkForErrors() {
            string error = idk();
            if (error == "") return false;
            cerr << "\x1B[31mERROR:\033[0m " << error << '\n';
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
