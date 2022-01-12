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
            string str = name + "(";
            if (!arguments.empty()) {
                for (auto [type, name] : arguments)
                    str += type + ", ";
                str.pop_back();
                str.pop_back();
            }
            str += ")";
            return str;
        }

        static vector<string> findCycle(map<string, vector<string>>& graph) {
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
            for (auto& [node, nghbs] : graph)
                if (father[node] == "")
                    dfs(node, "$");
            return cycle;
        }

        Scanner scanner;
        Parser parser;

        vector<Declaration*> declarations;
        vector<Object*> objects;
        vector<Function*> functions;

        map<string, Declaration*> declarationMap;
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

        void checkForCyclicDependencies() {
            map<string, vector<string>> graph;
            for (auto object : objects) {
                auto [name, members] = *object;
                for (auto [mType, mName] : members)
                    graph[name].push_back(mType);
            }
            auto cycle = findCycle(graph);
            if (!cycle.empty()) {
                string error = "types [";
                for (string node : cycle)
                    error += green(node) + ", ";
                error.pop_back();
                error.pop_back();
                error += "] form a cycle of dependencies";
                errors.push_back(error);
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
                            errors.push_back("member " + green(name) + " of type " + green(object->name) + " is of undefined type " + green(type));
                }
                if (node.index() == 1) {
                    auto function = get<1>(node);
                    auto& [type, name, arguments, statements] = *function;
                    const string signature = getFunctionSignature(function);
                    if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                        errors.push_back("function " + green(signature) + " has undefined return type " + green(type));
                    for (auto [type, name] : arguments)
                        if (type == "void")
                            errors.push_back("argument " + green(name) + " of function " + green(signature) + " is of type " + green(type));
                        else if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                            errors.push_back("argument " + green(name) + " of function " + green(signature) + " is of undefined type " + green(type));
                    for (auto statement : statements)
                        dfs(statement, function);
                }
                if (node.index() == 2) {
                    auto declaration = get<2>(node);
                    auto& [type, name, value, constant] = *declaration;
                    if (!function) {
                        if (type == "void")
                            errors.push_back("global variable " + green(name) + " is of type " + green(type));
                        else if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                            errors.push_back("global variable " + green(name) + " is of undefined type " + green(type));
                    }
                    else {
                        const string signature = getFunctionSignature(function);
                        if (type == "void")
                            errors.push_back("variable " + green(name) + " of function " + green(signature) + " is of type " + green(type));
                        else if (!types.count(getBaseType(type)) && !objectMap.count(getBaseType(type)))
                            errors.push_back("variable " + green(name) + " of function " + green(signature) + " is of undefined type " + green(type));
                    }
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
                if (node.index() == 16) {
                    auto statement = get<16>(node)->content;
                    if (statement.index() == 0) dfs(get<0>(statement), function);
                    if (statement.index() == 3) dfs(get<3>(statement), function);
                    if (statement.index() == 4) dfs(get<4>(statement), function);
                    if (statement.index() == 5) dfs(get<5>(statement), function);
                }
            };
            for (auto declaration : declarations)
                dfs(declaration, nullptr);
            for (auto object : objects)
                dfs(object, nullptr);
            for (auto function : functions)
                dfs(function, nullptr);
        }

        void checkForTypeErrors() {
            map<string, string> global;
            vector<map<string, string>> local;
            auto findVariable = [&](string name) -> string {
                for (int i = int(local.size()) - 1; i >= 0; i--)
                    if (local[i].count(name))
                        return local[i][name];
                if (global.count(name))
                    return global[name];
                errors.push_back("undefined variable " + green(name));
                return "?";
            };

            string scope;
            function<string(Node)> dfs = [&](Node node) -> string {
                auto check = [&](string type, ObjectLiteral* objectLiteral) {
                    if (!objectMap.count(type))
                        return false;
                    auto object = objectMap[type];
                    auto& [members] = *objectLiteral;
                    for (auto [name, value] : members) {
                        const string _name = name;
                        auto it = find_if(object->members.begin(), object->members.end(), [&](pair<string, string> member) {
                            return member.second == _name;
                        });
                        if (it == object->members.end())
                            return false;
                        if (it->first != dfs(value))
                            return false;
                    }
                    return true;
                };

                if (node.index() == 1) {
                    auto function = get<1>(node);
                    auto& [type, name, arguments, statements] = *function;
                    local.emplace_back();
                    for (auto [type, name] : arguments)
                        local.back()[name] = type;
                    for (auto statement : statements)
                        dfs(statement);
                    local.pop_back();
                    return "";
                }
                if (node.index() == 2) {
                    auto declaration = get<2>(node);
                    auto& [type, name, value, constant] = *declaration;
                    const string valueType = dfs(value);
                    if (type.back() == ']' && valueType == "$array")
                        return "";
                    if (value->content.index() == 1 && get<1>(value->content)->content.index() == 6 && check(type, get<6>(get<1>(value->content)->content)))
                        return "";
                    if (type != valueType)
                        errors.push_back("variable " + green(name) + " is of type " + green(type) + " and is assigned an expression of type " + green(valueType));
                    if (scope == "global")
                        global[name] = type;
                    else {
                        if (local.back().count(name))
                            errors.push_back("variable " + green(name) + " has already been defined");
                        local.back()[name] = type;
                    }
                    return "";
                }
                if (node.index() == 3) {
                    auto assignation = get<3>(node);
                    auto& [variable, value, op] = *assignation;
                    const string variableType = dfs(variable);
                    const string valueType = dfs(value);
                    if (variableType.back() == ']' && valueType == "$array")
                        return "";
                    if (value->content.index() == 1 && get<1>(value->content)->content.index() == 6 && check(variableType, get<6>(get<1>(value->content)->content)))
                        return "";
                    if (variableType != valueType)
                        errors.push_back("lvalue is of type " + green(variableType) + " but rvalue is of type " + green(valueType));
                    return "";
                }
                if (node.index() == 4) {
                    auto _if = get<4>(node);
                    auto& [conditions, statements] = *_if;
                    int index = 0;
                    for (auto condition : conditions) {
                        const string conditionType = dfs(condition);
                        if (conditionType != "bool")
                            errors.push_back("expression inside " + green(index ? "elif" : "if") + " is of type " + green(conditionType));
                        index++;
                    }
                    for (auto& group : statements) {
                        local.emplace_back();
                        for (auto statement : group)
                            dfs(statement);
                        local.pop_back();
                    }
                    return "";
                }
                if (node.index() == 5) {
                    auto _while = get<5>(node);
                    auto& [condition, statements] = *_while;
                    const string conditionType = dfs(condition);
                    if (conditionType != "bool")
                        errors.push_back("expression inside " + green("while") + " is of type " + green(conditionType));
                    local.emplace_back();
                    for (auto statement : statements)
                        dfs(statement);
                    local.pop_back();
                    return "";
                }
                if (node.index() == 6) {
                    auto _for = get<6>(node);
                    auto& [iterator, from, to, step, statements] = *_for;
                    const string fromType = dfs(from);
                    const string toType = dfs(to);
                    const string stepType = dfs(step);
                    if (fromType != "int")
                        errors.push_back(green("from") + " expression inside " + green("for") + " is of type " + green(fromType));
                    if (toType != "int")
                        errors.push_back(green("to") + " expression inside " + green("for") + " is of type " + green(toType));
                    if (stepType != "int")
                        errors.push_back(green("step") + " expression inside " + green("for") + " is of type " + green(stepType));
                    local.emplace_back();
                    local.back()[iterator] = "int";
                    for (auto statement : statements)
                        dfs(statement);
                    local.pop_back();
                    return "";
                }
                if (node.index() == 7) {
                    auto memberAccess = get<7>(node);
                    auto& [object, member] = *memberAccess;
                    const string objectType = dfs(object);
                    if (!objectMap.count(objectType)) {
                        errors.push_back(green(objectType) + " is not a user-defined type");
                        return "?";
                    }
                    for (auto [type, name] : objectMap[objectType]->members)
                        if (name == member)
                            return type;
                    errors.push_back("member " + green(member) + " is not defined for type " + green(objectType));
                    return "?";
                }
                if (node.index() == 8) {
                    auto elementAccess = get<8>(node);
                    auto& [array, index] = *elementAccess;
                    const string arrayType = dfs(array);
                    const string indexType = dfs(index);
                    if (arrayType.back() != ']') {
                        errors.push_back(green(arrayType) + " is not an array type");
                        return "?";
                    }
                    if (indexType != "int") {
                        errors.push_back(green(indexType) + " is not a subscript type");
                        return "?";
                    }
                    return arrayType.substr(0, int(arrayType.size()) - 2);
                }
                if (node.index() == 9) {
                    auto objectLiteral = get<9>(node);
                    auto& [members] = *objectLiteral;
                    set<string> memberNames;
                    for (auto [name, value] : members) {
                        if (memberNames.count(name)) {
                            errors.push_back("redefinition of member " + green(name) + " inside object literal");
                            return "?";
                        }
                        memberNames.insert(name);
                    }
                    return "$object";
                }
                if (node.index() == 10) {
                    auto functionCall = get<10>(node);
                    auto& [name, arguments] = *functionCall;
                    string signature = name + "(";
                    for (auto argument : arguments)
                        signature += dfs(argument) + ", ";
                    if (!arguments.empty()) {
                        signature.pop_back();
                        signature.pop_back();
                    }
                    signature += ")";
                    if (!functionMap.count(signature)) {
                        errors.push_back("there is no function of signature " + green(signature) + " to be called");
                        return "?";
                    }
                    return functionMap[signature]->type;
                }
                if (node.index() == 11) {
                    auto unaryExpression = get<11>(node);
                    auto& [value, op] = *unaryExpression;
                    const string valueType = dfs(value);
                    if (op == "not") {
                        if (valueType != "bool") {
                            errors.push_back(green(op) + " expects a " + green("bool") + " operand");
                            return "?";
                        }
                        return "bool";
                    }
                    return "?";
                }
                if (node.index() == 12) {
                    auto binaryExpression = get<12>(node);
                    auto& [lhs, rhs, op] = *binaryExpression;
                    const string lhsType = dfs(lhs);
                    const string rhsType = dfs(rhs);
                    if (op == "and" || op == "or") {
                        if (lhsType != "bool" || rhsType != "bool") {
                            errors.push_back(green(op) + " expects operands of type " + green("bool"));
                            return "?";
                        }
                        return "bool";
                    }
                    if (op == "==" || op == "<>") {
                        if (lhsType != rhsType) {
                            errors.push_back(green(op) + " expects operands of the same type");
                            return "?";
                        }
                        return "bool";
                    }
                    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
                        if (lhsType != rhsType) {
                            errors.push_back(green(op) + " expects operands of the same type");
                            return "?";
                        }
                        if (lhsType != "int" && lhsType != "float") {
                            errors.push_back(green(op) + " expects operands of type " + green("int") + " or " + green("float"));
                            return "?";
                        }
                        return "bool";
                    }
                    if (op == "+" || op == "-" || op == "*" || op == "/") {
                        if (lhsType != rhsType) {
                            errors.push_back(green(op) + " expects operands of the same type");
                            return "?";
                        }
                        if (lhsType != "int" && lhsType != "float") {
                            errors.push_back(green(op) + " expects operands of type " + green("int") + " or " + green("float"));
                            return "?";
                        }
                        return lhsType;
                    }
                    if (op == "%") {
                        if (lhsType != "int" || rhsType != "int") {
                            errors.push_back(green(op) + " expects operands of type " + green("int"));
                            return "?";
                        }
                        return "int";
                    }
                    return "?";
                }
                if (node.index() == 13) {
                    auto lvalue = get<13>(node)->content;
                    if (lvalue.index() == 0) return dfs(get<0>(lvalue));
                    if (lvalue.index() == 1) return dfs(get<1>(lvalue));
                    if (lvalue.index() == 2) return findVariable(get<2>(lvalue));
                }
                if (node.index() == 14) {
                    auto rvalue = get<14>(node)->content;
                    if (rvalue.index() == 0) return dfs(get<0>(rvalue));
                    if (rvalue.index() == 1) return dfs(get<1>(rvalue));
                    if (rvalue.index() == 2) return dfs(get<2>(rvalue));
                    if (rvalue.index() == 3) return dfs(get<3>(rvalue));
                    if (rvalue.index() == 4) return dfs(get<4>(rvalue));
                    if (rvalue.index() == 5) return dfs(get<5>(rvalue));
                }
                if (node.index() == 15) {
                    auto literal = get<15>(node)->content;
                    if (literal.index() == 0) return "int";
                    if (literal.index() == 1) return "float";
                    if (literal.index() == 2) return "char";
                    if (literal.index() == 3) return "string";
                    if (literal.index() == 4) return "bool";
                    if (literal.index() == 5) {
                        const string lengthType = dfs(get<5>(literal));
                        if (lengthType != "int") {
                            errors.push_back("array literal expects a length of type " + green("int"));
                            return "?";
                        }
                        return "$array";
                    }
                    if (literal.index() == 6) return dfs(get<6>(literal));
                }
                if (node.index() == 16) {
                    auto statement = get<16>(node)->content;
                    if (statement.index() == 0) return dfs(get<0>(statement));
                    if (statement.index() == 1) return dfs(get<1>(statement));
                    if (statement.index() == 2) return dfs(get<2>(statement));
                    if (statement.index() == 3) return dfs(get<3>(statement));
                    if (statement.index() == 4) return dfs(get<4>(statement));
                    if (statement.index() == 5) return dfs(get<5>(statement));
                    if (statement.index() == 7) return dfs(get<7>(statement));
                }
                return "";
            };
            scope = "global";
            for (auto declaration : declarations)
                dfs(declaration);
            scope = "local";
            for (auto function : functions)
                dfs(function);
        }

        void createTables() {
            ofstream fout("symbols.txt");
            function<void(Node, int)> dfs = [&](Node node, int level) {
                if (node.index() == 1) {
                    auto function = get<1>(node);
                    auto& [type, name, arguments, statements] = *function;
                    for (auto statement : statements)
                        dfs(statement, level + 1);
                }
                if (node.index() == 2) {
                    auto declaration = get<2>(node);
                    auto& [type, name, value, constant] = *declaration;
                    for (int i = 0; i < level; i++)
                        fout << '\t';
                    if (constant) fout << "const ";
                    fout << type << ' ' << name;
                    auto rvalue = value->content;
                    if (rvalue.index() == 1) {
                        auto literal = get<1>(rvalue)->content;
                        if (literal.index() == 0) fout << " = " << get<0>(literal);
                        if (literal.index() == 1) fout << " = " << get<1>(literal);
                        if (literal.index() == 2) fout << " = '" << get<2>(literal) << '\'';
                        if (literal.index() == 3) fout << " = \"" << get<3>(literal) << '\"';
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
                if (node.index() == 4) {
                    auto _if = get<4>(node);
                    for (auto& group : _if->statements)
                        for (auto statement : group)
                            dfs(statement, level + 1);
                }
                if (node.index() == 5) {
                    auto _while = get<5>(node);
                    for (auto statement : _while->statements)
                        dfs(statement, level + 1);
                }
                if (node.index() == 6) {
                    auto _for = get<6>(node);
                    auto& [iterator, from, to, step, statements] = *_for;
                    for (int i = 0; i < level + 1; i++)
                        fout << '\t';
                    fout << "int " << iterator;
                    auto rvalue = from->content;
                    if (rvalue.index() == 1) {
                        auto literal = get<1>(rvalue)->content;
                        if (literal.index() == 0)
                            fout << " = " << get<0>(literal) << ";\n";
                    }
                    for (auto statement : statements)
                        dfs(statement, level + 1);
                }
                if (node.index() == 16) {
                    auto statement = get<16>(node)->content;
                    if (statement.index() == 0) dfs(get<0>(statement), level);
                    if (statement.index() == 3) dfs(get<3>(statement), level);
                    if (statement.index() == 4) dfs(get<4>(statement), level);
                    if (statement.index() == 5) dfs(get<5>(statement), level);
                }
            };

            fout << "GLOBAL VARIABLES\n";
            fout << "================\n\n";
            for (auto declaration : declarations)
                dfs(declaration, 0);

            fout << "\n\nUSER-DEFINED TYPES";
            fout << "\n==================\n\n";
            for (auto object : objects) {
                auto& [name, members] = *object;
                fout << name << " { ";
                for (auto [type, name] : members)
                    fout << type << ' ' << name << "; ";
                fout << "}\n";
            }

            fout << "\n\nFUNCTIONS";
            fout << "\n=========\n\n";
            for (int i = 5; i < int(functions.size()); i++) {
                auto& [type, name, arguments, statements] = *(functions[i]);
                string str = type + " " + name + "(";
                if (!arguments.empty()) {
                    for (auto [type, name] : arguments)
                        str += type + " " + name + ", ";
                    str.pop_back();
                    str.pop_back();
                }
                str += ");";
                fout << str << '\n';
                dfs(functions[i], 0);
            }
        }

        void checkForErrors() {
            checkForCyclicDependencies();
            checkForBreakContinueErrors();
            checkForUndefinedTypeErrors();
            checkForTypeErrors();
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
                    dq.emplace_back("string", "message");
                    dq.emplace_back(arg2, "value");
                    functions.push_back(new Function{"void", "print", dq, deque<Statement*>()});
                }
            }

        int parse() {
            if (parser.parse())
                return 1;
            for (auto declaration : declarations) {
                const string id = declaration->name;
                if (declarationMap.count(id))
                    errors.push_back("global variable " + green(id) + " has already been defined");
                declarationMap[id] = declaration;
            }
            for (auto object : objects) {
                const string id = object->name;
                if (objectMap.count(id))
                    errors.push_back("type " + green(id) + " has already been defined");
                objectMap[id] = object;
                set<string> members;
                for (auto [type, name] : object->members) {
                    if (members.count(name))
                        errors.push_back("member " + green(name) + " has already been defined inside type " + green(object->name));
                    members.insert(name);
                }
            }
            for (auto function : functions) {
                const string id = getFunctionSignature(function);
                if (functionMap.count(id))
                    errors.push_back("function " + green(id) + " has already been defined");
                functionMap[id] = function;
                set<string> arguments;
                for (auto [type, name] : function->arguments) {
                    if (arguments.count(name))
                        errors.push_back("argument " + green(name) + " has already been defined inside function " + green(function->name));
                    arguments.insert(name);
                }
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
