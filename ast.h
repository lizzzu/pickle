#ifndef AST_H
#define AST_H

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
struct UnaryExpression;
struct BinaryExpression;

struct LValue;
struct RValue;
struct Literal;
struct Statement;

struct Object {
    string name;
    deque<pair<string, string>> members; // type, name
};
struct Function {
    string type;
    string name;
    deque<pair<string, string>> arguments; // type, name
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
    deque<pair<string, RValue*>> members; // name, value
};
struct FunctionCall {
    string name;
    deque<RValue*> arguments;
};
struct UnaryExpression {
    RValue *value;
    string op;
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
    variant<LValue*, Literal*, FunctionCall*, RValue*, UnaryExpression*, BinaryExpression*> content;
};
struct Literal {
    variant<int, double, char, string, bool, RValue*, ObjectLiteral*> content; // RValue = [rvalue]
};
struct Statement {
    variant<Declaration*, Assignation*, FunctionCall*, If*, While*, For*, string, RValue*> content; // string = break or continue or return, RValue = return rvalue
};

typedef variant<
    Object*, Function*,
    Declaration*, Assignation*,
    If*, While*, For*,
    MemberAccess*, ElementAccess*, ObjectLiteral*, FunctionCall*, UnaryExpression*, BinaryExpression*,
    LValue*, RValue*, Literal*, Statement*
> Node;

#endif
