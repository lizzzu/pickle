#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "scanner.h"
#include "parser.hpp"

namespace Pickle {
    class Interpreter {
        static string red(string str) { return "\x1B[31m" + str + "\033[0m"; }
        static string green(string str) { return "\x1B[32m" + str + "\033[0m"; }

        static string getBaseType(string str) {
            if (str.back() == ']') {
                str.pop_back();
                str.pop_back();
            }
            return str;
        }

        static string getFunctionSignature(Function* function) {
            auto& [type, name, arguments, statements] = *function;
            string str = type + " " + name + "(";
            if (!arguments.empty()) {
                for (auto [type, name] : arguments)
                    str += type + " " + name + ", ";
                str.pop_back();
                str.pop_back();
            }
            str += ")";
            return str;
        }

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

        Scanner scanner;
        Parser parser;

        vector<Declaration*> declarations;
        vector<Object*> objects;
        vector<Function*> functions;

        map<string, Object*> objectMap;
        map<string, Function*> functionMap;

        set<string> types;
        vector<string> errors;

        void dfs(Node node) {
            if (node.index() == 0) {
                auto object = get<0>(node);
            }
            if (node.index() == 1) {
                auto function = get<1>(node);
                for (auto statement : function->statements)
                    dfs(statement);
            }
            if (node.index() == 2) {
                auto declaration = get<2>(node);
                dfs(declaration->value);
            }
            if (node.index() == 3) {
                auto assignation = get<3>(node);
                dfs(assignation->variable);
                dfs(assignation->value);
            }
            if (node.index() == 4) {
                auto _if = get<4>(node);
                for (auto condition : _if->conditions)
                    dfs(condition);
                for (auto& group : _if->statements)
                    for (auto statement : group)
                        dfs(statement);
            }
            if (node.index() == 5) {
                auto _while = get<5>(node);
                dfs(_while->condition);
                for (auto statement : _while->statements)
                    dfs(statement);
            }
            if (node.index() == 6) {
                auto _for = get<6>(node);
                dfs(_for->from);
                dfs(_for->to);
                dfs(_for->step);
                for (auto statement : _for->statements)
                    dfs(statement);
            }
            if (node.index() == 7) {
                auto memberAccess = get<7>(node);
                dfs(memberAccess->object);
            }
            if (node.index() == 8) {
                auto elementAccess = get<8>(node);
                dfs(elementAccess->array);
                dfs(elementAccess->index);
            }
            if (node.index() == 9) {
                auto objectLiteral = get<9>(node);
                for (auto member : objectLiteral->members)
                    dfs(member.second);
            }
            if (node.index() == 10) {
                auto functionCall = get<10>(node);
                for (auto argument : functionCall->arguments)
                    dfs(argument);
            }
            if (node.index() == 11) {
                auto unaryExpression = get<11>(node);
                dfs(unaryExpression->value);
            }
            if (node.index() == 12) {
                auto binaryExpression = get<12>(node);
                dfs(binaryExpression->lhs);
                dfs(binaryExpression->rhs);
            }
            if (node.index() == 13) {
                auto lvalue = get<13>(node)->content;
                if (lvalue.index() == 0) dfs(get<0>(lvalue));
                if (lvalue.index() == 1) dfs(get<1>(lvalue));
            }
            if (node.index() == 14) {
                auto rvalue = get<14>(node)->content;
                if (rvalue.index() == 0) dfs(get<0>(rvalue));
                if (rvalue.index() == 1) dfs(get<1>(rvalue));
                if (rvalue.index() == 2) dfs(get<2>(rvalue));
                if (rvalue.index() == 3) dfs(get<3>(rvalue));
                if (rvalue.index() == 4) dfs(get<4>(rvalue));
                if (rvalue.index() == 5) dfs(get<5>(rvalue));
            }
            if (node.index() == 15) {
                auto literal = get<15>(node)->content;
                if (literal.index() == 5) dfs(get<5>(literal));
                if (literal.index() == 6) dfs(get<6>(literal));
            }
            if (node.index() == 16) {
                auto statement = get<16>(node)->content;
                if (statement.index() == 0) dfs(get<0>(statement));
                if (statement.index() == 1) dfs(get<1>(statement));
                if (statement.index() == 2) dfs(get<2>(statement));
                if (statement.index() == 3) dfs(get<3>(statement));
                if (statement.index() == 4) dfs(get<4>(statement));
                if (statement.index() == 5) dfs(get<5>(statement));
                if (statement.index() == 7) dfs(get<7>(statement));
            }
        }

        void checkForBreakContinueErrors() {
            function<void(Node, Function*)> dfs = [&](Node node, Function* function) {
                if (node.index() == 1) {
                    auto function = get<1>(node);
                    for (auto statement : function->statements)
                        dfs(statement, function);
                }
                if (node.index() == 4) {
                    auto _if = get<4>(node);
                    for (auto& group : _if->statements)
                        for (auto statement : group)
                            dfs(statement, function);
                }
                if (node.index() == 16) {
                    auto statement = get<16>(node)->content;
                    if (statement.index() == 3)
                        dfs(get<3>(statement), function);
                    if (statement.index() == 6 && get<6>(statement) != "return")
                        errors.push_back("in function " + green(getFunctionSignature(function)) + " there is a " + green(get<6>(statement)) + " statement that is not inside a loop");
                }
            };
            for (auto function : functions)
                dfs(function, nullptr);
        }

        void checkForUndefinedTypeErrors() {
            function<void(Node, Function*)> dfs = [&](Node node, Function* function) {
                if (node.index() == 0) {
                    auto object = get<0>(node);
                    for (auto [type, name] : object->members)
                        if (type == "void")
                            errors.push_back("member " + green(name) + " of type " + green(object->name) + " is of type " + green(type));
                        else if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                            errors.push_back("member " + green(name) + " of type " + green(object->name) + " is of type " + green(type) + " which is undefined");
                }
                if (node.index() == 1) {
                    auto function = get<1>(node);
                    auto& [type, name, arguments, statements] = *function;
                    const string signature = getFunctionSignature(function);
                    if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                        errors.push_back("function " + green(signature) + " has return type " + green(type) + " which is undefined");
                    for (auto [type, name] : arguments)
                        if (type == "void")
                            errors.push_back("argument " + green(name) + " of function " + green(signature) + " is of type " + green(type));
                        else if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                            errors.push_back("argument " + green(name) + " of function " + green(signature) + " is of type " + green(type) + " which is undefined");
                    for (auto statement : statements)
                        dfs(statement, function);
                }
                if (node.index() == 2) {
                    auto declaration = get<2>(node);
                    auto& [type, name, value, constant] = *declaration;
                    const string signature = getFunctionSignature(function);
                    if (type == "void")
                        errors.push_back("variable " + green(name) + " of function " + green(signature) + " is of type " + green(type));
                    else if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                        errors.push_back("variable " + green(name) + " of function " + green(signature) + " is of type " + green(type) + " which is undefined");
                }
                if (node.index() == 4) {
                    auto _if = get<4>(node);
                    for (auto& group : _if->statements)
                        for (auto statement : group)
                            dfs(statement, function);
                }
                if (node.index() == 5) {
                    auto _while = get<5>(node);
                    for (auto statement : _while->statements)
                        dfs(statement, function);
                }
                if (node.index() == 6) {
                    auto _for = get<6>(node);
                    for (auto statement : _for->statements)
                        dfs(statement, function);
                }
            };
            for (auto object : objects)
                dfs(object, nullptr);
            for (auto function : functions)
                dfs(function, nullptr);
        }

        void createTables() {
            ofstream fout("symbols.txt");
            for (auto declaration : declarations) {
                auto& [type, name, value, constant] = *declaration;
                if (constant) fout << "const ";
                fout << type << ' ' << name;
                auto rvalue = value->content;
                if (rvalue.index() == 1) {
                    auto literal = get<1>(rvalue)->content;
                    if (literal.index() == 0) fout << " = " << get<0>(literal);
                    if (literal.index() == 1) fout << " = " << get<1>(literal);
                    if (literal.index() == 2) fout << " = " << get<2>(literal);
                    if (literal.index() == 3) fout << " = " << get<3>(literal);
                    if (literal.index() == 4) fout << " = " << get<4>(literal);
                    if (literal.index() == 5) {
                        auto rvalue = get<5>(literal)->content;
                        if (rvalue.index() == 1) {
                            auto literal = get<1>(rvalue)->content;
                            if (literal.index() == 0)
                                fout << " = [" << get<0>(literal) << ']';
                        }
                    }
                }
                fout << ";\n";
            }
            fout << '\n';
            for (auto object : objects) {
                auto& [name, members] = *object;
                fout << name << " { ";
                for (auto [type, name] : members)
                    fout << type << ' ' << name << "; ";
                fout << "}\n";
            }
            fout << '\n';
            for (auto function : functions)
                fout << getFunctionSignature(function) << ";\n";
        }

        void checkForErrors() {
            checkForBreakContinueErrors();
            checkForUndefinedTypeErrors();
        }

    public:
        Interpreter() :
            scanner(*this),
            parser(scanner, *this) {
                types.insert("int");
                types.insert("float");
                types.insert("char");
                types.insert("string");
                types.insert("bool");
                types.insert("void");
                for (string arg2 : {"int", "float", "char", "string", "bool"}) {
                    deque<pair<string, string>> dq;
                    dq.emplace_back("string", "arg1");
                    dq.emplace_back(arg2, "arg2");
                    functions.push_back(new Function{"void", "print", dq, deque<Statement*>()});
                }
            }

        int parse() {
            if (parser.parse())
                return 1;
            for (auto object : objects) {
                const string id = object->name;
                if (objectMap.count(id))
                    errors.push_back("type " + green(id) + " has already been defined");
                objectMap[id] = object;
            }
            for (auto function : functions) {
                const string id = getFunctionSignature(function);
                if (functionMap.count(id))
                    errors.push_back("function " + green(id) + " has already been defined");
                functionMap[id] = function;
            }
            createTables();
            checkForErrors();
            if (!errors.empty()) {
                for (string error : errors)
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
