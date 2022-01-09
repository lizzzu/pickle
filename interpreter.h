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

        static string red(string str) { return "\x1B[31m" + str + "\033[0m"; }
        static string green(string str) { return "\x1B[32m" + str + "\033[0m"; }

        static vector<string> checkForCycles(set<string>& nodes, map<string, vector<string>>& graph) {
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

        void createTables() {
            ofstream foutId("symbol_table.txt");
            ofstream foutFn("symbol_table_functions.txt");

            for (auto function : functions) {
                auto [type, name, arguments, statements] = *function;
                foutFn << type << ' ' << name << '(';
                if (!arguments.empty())
                    foutFn << arguments[0].first << ' ' << arguments[0].second;
                for (int i = 1; i < int(arguments.size()); i++) {
                    foutFn << ", " << arguments[i].first << ' ' << arguments[i].second;
                }
                foutFn << ")\n";
            }

            for (auto declaration : declarations) {
                auto [type, name, value, constant] = *declaration;
                if (value->content.index() == 1) {
                    if (constant)
                        foutId << "const ";
                    auto val = get<1>(value->content);
                    if (val->content.index() == 0)
                        foutId << type << ' ' << name << " = " << get<0>(val->content) << '\n';
                    else if (val->content.index() == 1)
                        foutId << type << ' ' << name << " = " << get<1>(val->content) << '\n';
                    else if (val->content.index() == 2)
                        foutId << type << ' ' << name << " = " << get<2>(val->content) << '\n';
                    else if (val->content.index() == 3)
                        foutId << type << ' ' << name << " = " << get<3>(val->content) << '\n';
                    else if (val->content.index() == 4)
                        foutId << type << ' ' << name << " = " << get<4>(val->content) << '\n';
                }
            }

            foutId << '\n';

            for (auto object : objects) {
                auto [name, members] = *object;
                foutId << name << ": ";
                if (!members.empty())
                    foutId << members[0].first << ' ' << members[0].second;
                for (int i = 1; i < int(members.size()); i++) {
                    foutId << ", " << members[i].first << ' ' << members[i].second;
                }
                foutId << '\n';
            }
        }


    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) { 
                deque<pair<string, string>> dq;
                dq.push_back(make_pair("string", "arg1"));
                dq.push_back(make_pair("int", "arg2"));
                functions.push_back(new Function{"void", "print", dq, deque<Statement*>()});
            }
        
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
