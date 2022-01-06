#include <variant>
#include <bits/stdc++.h>
using namespace std;

struct Object {
    string name;
    vector<string> memberTypes;
    vector<string> memberNames;
};

struct Function {
    string type;
    string name;
    vector<string> argumentTypes;
    vector<string> argumentNames;
    vector<Statement> statements;
};

struct Declaration {
    string type;
    string name;
    RValue value;
};

struct Assignation {
    LValue variable;
    RValue value;
};

struct If {
    vector<RValue> conditions; // if elif elif ... elif
    vector<vector<Statement>> statements; // if elif elif ... elif else
};

struct While {
    RValue condition;
    vector<Statement> statements;
};

struct For {
    string iterator;
    RValue from;
    RValue to;
    RValue step;
    vector<Statement> statements;
};

struct MemberAccess {
    LValue object;
    string member;
};

struct ElementAccess {
    LValue array;
    RValue index;
};

struct ObjectLiteral {
    vector<string> memberNames;
    vector<RValue> memberValues;
};

struct FunctionCall {
    string name;
    vector<RValue> arguments;
};

struct BinaryExpression {
    string op;
    RValue lhs;
    RValue rhs;
};

typedef variant<MemberAccess*, ElementAccess*, string> LValue; // string = identifier
typedef variant<LValue, Literal, FunctionCall*, RValue, BinaryExpression*> RValue; // RValue = not rvalue
typedef variant<int, double, char, string, bool, RValue, ObjectLiteral*> Literal; // RValue = [rvalue]
typedef variant<Declaration*, Assignation*, FunctionCall*, If*, While*, For*, string, RValue> Statement; // string = break or continue, RValue = return rvalue (can be nullptr)
