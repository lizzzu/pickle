#ifndef AST_H
#define AST_H

#include <variant>
#include <bits/stdc++.h>
using namespace std;

struct Object;
struct Function;

struct Declaration;
struct Assignation;

struct If;
struct While;
struct For;

struct MemberAccess;
struct ElementAccess;
struct ObjectLiteral;
struct FunctionCall;
struct BinaryExpression;

struct LValue;
struct RValue;
struct Literal;
struct Statement;

struct Object {
    string name;
    deque<string> memberTypes;
    deque<string> memberNames;
};
struct Function {
    string type;
    string name;
    deque<string> argumentTypes;
    deque<string> argumentNames;
    deque<Statement*> statements;
};

struct Declaration {
    string type;
    string name;
    RValue *value;
    bool constant;
};
struct Assignation {
    LValue *variable;
    RValue *value;
    string op;
};

struct If {
    deque<RValue*> conditions; // if elif elif ... elif
    deque<deque<Statement*>> statements; // if elif elif ... elif else
};
struct While {
    RValue *condition;
    deque<Statement*> statements;
};
struct For {
    string iterator;
    RValue *from;
    RValue *to;
    RValue *step;
    deque<Statement*> statements;
};

struct MemberAccess {
    LValue *object;
    string member;
};
struct ElementAccess {
    LValue *array;
    RValue *index;
};
struct ObjectLiteral {
    deque<string> memberNames;
    deque<RValue*> memberValues;
};
struct FunctionCall {
    string name;
    deque<RValue*> arguments;
};
struct BinaryExpression {
    RValue *lhs;
    RValue *rhs;
    string op;
};

struct LValue {
    variant<MemberAccess*, ElementAccess*, string> content; // string = identifier
};
struct RValue {
    variant<LValue*, Literal*, FunctionCall*, RValue*, BinaryExpression*> content; // RValue = not rvalue
};
struct Literal {
    variant<int, double, char, string, bool, RValue*, ObjectLiteral*> content; // RValue = [rvalue]
};
struct Statement {
    variant<Declaration*, Assignation*, FunctionCall*, If*, While*, For*, string, RValue*> content; // string = break or continue or return, RValue = return rvalue
};

#endif
