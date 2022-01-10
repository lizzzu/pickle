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
        set<string> primitiveTypes;

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

        void dfsDeclaration(Declaration* declaration) {
            dfsRValue(declaration->value);
        }

        void dfsAssignation(Assignation* assignation) {
            dfsLValue(assignation->variable);
            dfsRValue(assignation->value);
        }

        void dfsIf(If* _if) {
            for (auto condition : _if->conditions)
                dfsRValue(condition);
            for (auto& group : _if->statements)
                for (auto statement : group)
                    dfsStatement(statement);
        }

        void dfsWhile(While* _while) {
            dfsRValue(_while->condition);
            for (auto statement : _while->statements)
                dfsStatement(statement);
        }

        void dfsFor(For* _for) {
            dfsRValue(_for->from);
            dfsRValue(_for->to);
            dfsRValue(_for->step);
            for (auto statement : _for->statements)
                dfsStatement(statement);
        }

        void dfsMemberAccess(MemberAccess* memberAccess) {
            dfsLValue(memberAccess->object);
        }

        void dfsElementAccess(ElementAccess* elementAccess) {
            dfsLValue(elementAccess->array);
            dfsRValue(elementAccess->index);
        }

        void dfsObjectLiteral(ObjectLiteral* objectLiteral) {
            for (auto member : objectLiteral->members)
                dfsRValue(member.second);
        }

        void dfsFunctionCall(FunctionCall* functionCall) {
            for (auto argument : functionCall->arguments)
                dfsRValue(argument);
        }

        void dfsUnaryExpression(UnaryExpression* unaryExpression) {
            dfsRValue(unaryExpression->value);
        }

        void dfsBinaryExpression(BinaryExpression* binaryExpression) {
            dfsRValue(binaryExpression->lhs);
            dfsRValue(binaryExpression->rhs);
        }

        void dfsLValue(LValue* lvalue) {
            auto var = lvalue->content;
            if (var.index() == 0) dfsMemberAccess(get<0>(var));
            if (var.index() == 1) dfsElementAccess(get<1>(var));
        }

        void dfsRValue(RValue* rvalue) {
            auto var = rvalue->content;
            if (var.index() == 0) dfsLValue(get<0>(var));
            if (var.index() == 1) dfsLiteral(get<1>(var));
            if (var.index() == 2) dfsFunctionCall(get<2>(var));
            if (var.index() == 3) dfsRValue(get<3>(var));
            if (var.index() == 4) dfsUnaryExpression(get<4>(var));
            if (var.index() == 5) dfsBinaryExpression(get<5>(var));
        }

        void dfsLiteral(Literal* literal) {
            auto var = literal->content;
            if (var.index() == 5) dfsRValue(get<5>(var));
            if (var.index() == 6) dfsObjectLiteral(get<6>(var));
        }

        void dfsStatement(Statement* statement) {
            auto var = statement->content;
            if (var.index() == 0) dfsDeclaration(get<0>(var));
            if (var.index() == 1) dfsAssignation(get<1>(var));
            if (var.index() == 2) dfsFunctionCall(get<2>(var));
            if (var.index() == 3) dfsIf(get<3>(var));
            if (var.index() == 4) dfsWhile(get<4>(var));
            if (var.index() == 5) dfsFor(get<5>(var));
            if (var.index() == 7) dfsRValue(get<7>(var));
        }

/////////////////////////////////////////////////////////////////////////////////////////////////

        string dfs1If(If* _if) {
            for (auto& group : _if->statements)
                for (auto statement : group) {
                    const string res = dfs1Statement(statement);
                    if (res != "") return res;
                }
            return "";
        }

        string dfs1Statement(Statement* statement) {
            auto var = statement->content;
            if (var.index() == 3) {
                const string res = dfs1If(get<3>(var));
                if (res != "") return res;
            }
            if (var.index() == 6)
                return get<6>(var) == "return" ? "" : get<6>(var);
            return "";
        }

/////////////////////////////////////////////////////////////////////////////////////////////////

        string dfs3Declaration(Declaration* declaration) {
            string type = declaration->type;
            if (type.back() == ']') {
                type.pop_back();
                type.pop_back();
            }
            if (find_if(objects.begin(), objects.end(), [&](Object* object) {
                return object->name == type;
            }) == objects.end() && !primitiveTypes.count(type))
                return type;
            return "";
        }

        string dfs3If(If* _if) {
            for (auto& group : _if->statements)
                for (auto statement : group) {
                    const string res = dfs3Statement(statement);
                    if (res != "") return res;
                }
            return "";
        }

        string dfs3While(While* _while) {
            for (auto statement : _while->statements) {
                const string res = dfs3Statement(statement);
                if (res != "") return res;
            }
            return "";
        }

        string dfs3For(For* _for) {
            for (auto statement : _for->statements) {
                const string res =  dfs3Statement(statement);
                if (res != "") return res;
            }
            return "";
        }

        string dfs3Statement(Statement* statement) {
            auto var = statement->content;
            if (var.index() == 0) {
                const string res = dfs3Declaration(get<0>(var));
                if (res != "") return res;
            }
            if (var.index() == 3) {
                const string res = dfs3If(get<3>(var));
                if (res != "") return res;
            }
            if (var.index() == 4) {
                const string res = dfs3While(get<4>(var));
                if (res != "") return res;
            }
            if (var.index() == 5) {
                const string res = dfs3For(get<5>(var));
                if (res != "") return res;
            }
            return "";
        }

/////////////////////////////////////////////////////////////////////////////////////////////////

        string dfs4Declaration(Declaration* declaration) {
            string type = declaration->type;
            if (type.back() == ']') {
                type.pop_back();
                type.pop_back();
            }
            if (!primitiveTypes.count(type))
                return declaration->name;
            return "";
        }

        string dfs4Assignation(Assignation* assignation) {
            const string res = dfs4RValue(assignation->value);
            return "";
        }

        string dfs4If(If* _if) {
            for (auto condition : _if->conditions) {
                const string res1 = dfs4RValue(condition);
                if (res1 != "") return res1;
            }
            for (auto& group : _if->statements)
                for (auto statement : group) {
                    const string res2 = dfs4Statement(statement);
                    if (res2 != "") return res2;
                }
            return "";
        }

        string dfs4While(While* _while) {
            const string res1 = dfs4RValue(_while->condition);
            if (res1 != "") return res1;
            for (auto statement : _while->statements) {
                const string res2 = dfs4Statement(statement);
                if (res2 != "") return res2;
            }
            return "";
        }

        string dfs4For(For* _for) {
            for (auto statement : _for->statements) {
                const string res =  dfs4Statement(statement);
                if (res != "") return res;
            }
            return "";
        }

        string dfs4FunctionCall(FunctionCall* functionCall) {
            if (find_if(functions.begin(), functions.end(), [&](Function* function) {
                return function->name == functionCall->name;
            }) == functions.end())
                return functionCall->name;
            return "";
        }

        string dfs4UnaryExpression(UnaryExpression* unaryExpression) {
            const string res = dfs4RValue(unaryExpression->value);
            if (res != "") return res;
            return "";
        }

        string dfs4BinaryExpression(BinaryExpression* binaryExpression) {
            const string res = dfs4RValue(binaryExpression->rhs);
            if (res != "") return res;
            return "";
        }

        string dfs4RValue(RValue* rvalue) {
            auto var = rvalue->content;
            if (var.index() == 2) {
                const string res = dfs4FunctionCall(get<2>(var));
                if (res != "") return res;
            }
            if (var.index() == 3) {
                const string res = dfs4RValue(get<3>(var));
                if (res != "") return res;
            }
            if (var.index() == 4) {
                const string res = dfs4UnaryExpression(get<4>(var));
                if (res != "") return res;
            }
            if (var.index() == 5) {
                const string res = dfs4BinaryExpression(get<5>(var));
                if (res != "") return res;
            }
            return "";
        }

        string dfs4Statement(Statement* statement) {
            auto var = statement->content;
            if (var.index() == 0) {
                const string res = dfs4Declaration(get<0>(var));
                if (res != "") return res;
            }
            if (var.index() == 1) {
                const string res = dfs4Assignation(get<1>(var));
                if (res != "") return res;
            }
            if (var.index() == 2) {
                const string res = dfs4FunctionCall(get<2>(var));
                if (res != "") return res;
            }
            if (var.index() == 3) {
                const string res = dfs4If(get<3>(var));
                if (res != "") return res;
            }
            if (var.index() == 4) {
                const string res = dfs4While(get<4>(var));
                if (res != "") return res;
            }
            if (var.index() == 5) {
                const string res = dfs4For(get<5>(var));
                if (res != "") return res;
            }
            if (var.index() == 7) {
                const string res = dfs4RValue(get<7>(var));
                if (res != "") return res;
            }
            return "";
        }

/////////////////////////////////////////////////////////////////////////////////////////////////

        string idk() {
            for (auto function : functions) {
                auto& [type, name, arguments, statements] = *function;
                for (auto statement : statements)
                    dfsStatement(statement);
            }
            return "";
        }

        string checkForBreakContinueErrors() {
            for (auto function : functions) {
                auto& [type, name, arguments, statements] = *function;
                for (auto statement : statements) {
                    const string res = dfs1Statement(statement);
                    if (res != "") return "in function " + green(name) + " there is a " + green(res) + " statement that is not inside a loop";
                }
            }
            return "";
        }

        string checkForUndefinedObjects() {
            for (auto object : objects)
                for (auto [type, name] : object->members)
                    if (find_if(objects.begin(), objects.end(), [&](Object* object) {
                        return object->name == type;
                    }) == objects.end() && !primitiveTypes.count(type))
                        return "type " + green(type) + " is not defined";
            for (auto function : functions) {
                auto& [type, name, arguments, statements] = *function;
                for (auto statement : statements) {
                    const string res = dfs3Statement(statement);
                    if (res != "") return "type " + green(res) + " is not defined";
                }
            }
            return "";
        }

        string checkForUndefinedFunctions() {
            for (auto function : functions) {
                auto& [type, name, arguments, statements] = *function;
                for (auto statement : statements) {
                    const string res = dfs4Statement(statement);
                    if (res != "") return "function " + green(res) + " is not defined";
                }
            }
            return "";
        }

        string checkForErrors() {
            const string error1 = checkForObjectErrors(); if (error1 != "") return error1;
            const string error2 = idk(); if (error2 != "") return error2;
            const string error3 = checkForBreakContinueErrors(); if (error3 != "") return error3;
            const string error4 = checkForUndefinedObjects(); if (error4 != "") return error4;
            const string error5 = checkForUndefinedFunctions(); if (error5 != "") return error5;
            return "";
            // checkForUndefinedMembers();
            // checkForUndefinedFunctions();
            // checkForUndefinedVariables();
            // checkForAlreadyDefinedFunctions();
            // checkForAlreadyDefinedVariables();
            // checkForCyclicDependenciesBetweenFunctions();
            // checkForTypeErrors();
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
                primitiveTypes.insert("int");
                primitiveTypes.insert("float");
                primitiveTypes.insert("char");
                primitiveTypes.insert("string");
                primitiveTypes.insert("bool");
                primitiveTypes.insert("void");

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
