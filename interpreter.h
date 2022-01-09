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

        function<void(Statement*)> dfsStatement = [&](Statement* statement) {
            auto var = statement->content;
            if (var.index() == 0) dfsDeclaration(get<0>(var));
            if (var.index() == 1) dfsAssignation(get<1>(var));
            if (var.index() == 2) dfsFunctionCall(get<2>(var));
            if (var.index() == 3) dfsIf(get<3>(var));
            if (var.index() == 4) dfsWhile(get<4>(var));
            if (var.index() == 5) dfsFor(get<5>(var));
            if (var.index() == 7) dfsRValue(get<7>(var));
            // ceva
        };

        void dfsDeclaration() { };

        void dfsAssignation() { };

        void dfsIf() { };

        void dfsWhile() { };

        void dfsFor() { };

        void dfsMemberAccess() { };

        void dfsElementAccess() { };

        void dfsObjectLiteral() { };

        void dfsFunctionCall() { };

        void dfsUnaryExpression() { };

        void dfsBinaryExpression() { };

        void dfsLValue() { };

        void dfsRValue() { };

        void dfsLiteral() { };

        void dfsStatement() { };

        string idk() {
            for (auto function : functions) {
                auto& [type, name, arguments, statements] = *function;
                for (auto statement : statements)
                    dfsStatement(statement);
            }
            return "";
        }

        string checkForErrors() {
            const string error1 = checkForObjectErrors(); if (error1 == "") return error1;
            const string error2 = idk(); if (error2 == "") return error2;
            return "";
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
                for (int i = 1; i < int(arguments.size()); i++)
                    foutFn << ", " << arguments[i].first << ' ' << arguments[i].second;
                foutFn << ")\n";
            }

            for (auto declaration : declarations) {
                auto [type, name, value, constant] = *declaration;
                if (constant) foutId << "const ";
                foutId << type << ' ' << name;
                if (value->content.index() == 1) {
                    auto val = get<1>(value->content);
                    if (val->content.index() == 0) foutId << " = " << get<0>(val->content);
                    if (val->content.index() == 1) foutId << " = " << get<1>(val->content);
                    if (val->content.index() == 2) foutId << " = " << get<2>(val->content);
                    if (val->content.index() == 3) foutId << " = " << get<3>(val->content);
                    if (val->content.index() == 4) foutId << " = " << get<4>(val->content);
                    if (val->content.index() == 5) {
                        auto rvalue = get<5>(val->content);
                        if (rvalue->content.index() == 1) {
                            auto literal = get<1>(rvalue->content);
                            if (literal->content.index() == 0)
                                foutId << " = [" << get<0>(literal->content) << ']';
                        }
                    }
                }
                foutId << '\n';
            }
            foutId << '\n';

            for (auto object : objects) {
                auto [name, members] = *object;
                foutId << name << " { ";
                for (auto [type, name] : members)
                    foutId << type << ' ' << name << "; ";
                foutId << "}\n";
            }
        }

    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) {
                deque<pair<string, string>> dq1;
                dq1.push_back(make_pair("string", "arg1"));
                dq1.push_back(make_pair("int", "arg2"));
                functions.push_back(new Function{"void", "print", dq1, deque<Statement*>()});

                deque<pair<string, string>> dq2;
                dq2.push_back(make_pair("string", "arg1"));
                dq2.push_back(make_pair("float", "arg2"));
                functions.push_back(new Function{"void", "print", dq2, deque<Statement*>()});

                deque<pair<string, string>> dq3;
                dq3.push_back(make_pair("string", "arg1"));
                dq3.push_back(make_pair("char", "arg2"));
                functions.push_back(new Function{"void", "print", dq3, deque<Statement*>()});

                deque<pair<string, string>> dq4;
                dq4.push_back(make_pair("string", "arg1"));
                dq4.push_back(make_pair("string", "arg2"));
                functions.push_back(new Function{"void", "print", dq4, deque<Statement*>()});

                deque<pair<string, string>> dq5;
                dq5.push_back(make_pair("string", "arg1"));
                dq5.push_back(make_pair("bool", "arg2"));
                functions.push_back(new Function{"void", "print", dq5, deque<Statement*>()});
            }

        int parse() {
            const int res = parser.parse();
            createTables();
            if (res) return 1;
            const string error = checkForErrors();
            if (error != "") {
                cerr << red("PICKLE: ") << error << '\n';
                return 1;
            }
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
