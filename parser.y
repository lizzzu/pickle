%require "3.8"
%skeleton "lalr1.cc"

%defines
%define api.namespace { Pickle }
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include "ast.h"
    #include <bits/stdc++.h>
    using namespace std;
    namespace Pickle {
        class Scanner;
        class Interpreter;
    }
}

%code top {
    #include "scanner.h"
    #include "parser.hpp"
    #include "interpreter.h"
    using namespace Pickle;
    static Pickle::Parser::symbol_type yylex(Pickle::Scanner& scanner, Pickle::Interpreter& driver) {
        return scanner.get_next_token();
    }
}

%lex-param { Pickle::Scanner& scanner }
%lex-param { Pickle::Interpreter& driver }
%parse-param { Pickle::Scanner& scanner }
%parse-param { Pickle::Interpreter& driver }

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

%token DELIM1 DELIM2 DELIM3
%token <string> TYPE
%token IF ELIF ELSE WHILE FOR LET FROM TO STEP
%token CONST BREAK CONTINUE RETURN
%token <string> AND OR NOT EQNE LTGT ADDT PROD EQ ASGN
%token <string> ID
%token <int> INT
%token <double> FLOAT
%token <char> CHAR
%token <string> STRING
%token <bool> BOOL

%start program
%type <Object*> object
%type <Object*> property_list
%type <Function*> function
%type <Function*> argument_list
%type <Declaration*> declaration
%type <Assignation*> assignation
%type <deque<Statement*>*> statement_list
%type <If*> elif_list
%type <pair<RValue*, deque<Statement*>*>> if
%type <pair<RValue*, deque<Statement*>*>> elif
%type <deque<Statement*>*> else
%type <While*> while
%type <For*> for
%type <LValue*> lvalue
%type <RValue*> rvalue
%type <string> type
%type <Literal*> literal
%type <ObjectLiteral*> object_literal
%type <ObjectLiteral*> property_list_values
%type <FunctionCall*> function_call
%type <FunctionCall*> argument_list_values

%left ','
%left OR
%left AND
%left EQNE
%left LTGT
%left ADDT
%left PROD
%right NOT
%left '.' '(' ')' '[' ']'

%%

program
    : DELIM1 block1 DELIM2 block2 DELIM2 block3 DELIM3
    ;
block1
    :
    | block1 declaration ';'    { driver.pushDeclaration($2); }
    ;
block2
    :
    | block2 object             { driver.pushObject($2); }
    | block2 function           { driver.pushFunction($2); }
    ;
block3
    : function                  { driver.pushFunction($1); if ($1->type != "void" || $1->name != "main" || !$1->arguments.empty()) Pickle::Parser::error("the third block should contain the " + Pickle::Interpreter::green("main") + " function"); }
    ;

object
    : ID '{' '}'                   { $$ = new Object; $$->name = $1; }
    | ID '{' property_list '}'     { $$ =         $3; $$->name = $1; }
    ;
property_list
    : type ID ';'                  { $$ = new Object; $$->members.emplace_front($1, $2); }
    | type ID ';' property_list    { $$ =         $4; $$->members.emplace_front($1, $2); }
    ;

function
    : type ID '(' ')' '{' statement_list '}'                  { $$ = new Function; $$->type = $1; $$->name = $2; $$->statements = *$6; }
    | type ID '(' argument_list ')' '{' statement_list '}'    { $$ =           $4; $$->type = $1; $$->name = $2; $$->statements = *$7; }
    ;
argument_list
    : type ID                                                 { $$ = new Function; $$->arguments.emplace_front($1, $2); }
    | type ID ',' argument_list                               { $$ =           $4; $$->arguments.emplace_front($1, $2); }
    ;

declaration
    : type ID EQ rvalue          { $$ = new Declaration{$1, $2, $4, false}; }
    | CONST TYPE ID EQ rvalue    { $$ = new Declaration{$2, $3, $5, true}; }
    ;
assignation
    : lvalue EQ rvalue           { $$ = new Assignation{$1, $3, $2}; }
    | lvalue ASGN rvalue         { $$ = new Assignation{$1, $3, $2}; }
    ;

statement_list
    :                                     { $$ = new deque<Statement*>; }
    | statement_list declaration ';'      { $$ = $1; $$->push_front(new Statement{$2}); }
    | statement_list assignation ';'      { $$ = $1; $$->push_front(new Statement{$2}); }
    | statement_list function_call ';'    { $$ = $1; $$->push_front(new Statement{$2}); }
    | statement_list if                   { $$ = $1; If *i = new If; i->conditions.push_front($2.first); i->statements.push_front(*($2.second)); $$->push_front(new Statement{i}); }
    | statement_list if elif_list         { $$ = $1; If *i =     $3; i->conditions.push_front($2.first); i->statements.push_front(*($2.second)); $$->push_front(new Statement{i}); }
    | statement_list while                { $$ = $1; $$->push_front(new Statement{$2}); }
    | statement_list for                  { $$ = $1; $$->push_front(new Statement{$2}); }
    | statement_list BREAK ';'            { $$ = $1; $$->push_front(new Statement{"break"}); }
    | statement_list CONTINUE ';'         { $$ = $1; $$->push_front(new Statement{"continue"}); }
    | statement_list RETURN ';'           { $$ = $1; $$->push_front(new Statement{"return"}); }
    | statement_list RETURN rvalue ';'    { $$ = $1; $$->push_front(new Statement{$3}); }
    ;
elif_list
    : else                                { $$ = new If; $$->statements.push_front(*$1); }
    | elif elif_list                      { $$ = $2; $$->conditions.push_front($1.first); $$->statements.push_front(*($1.second)); }
    ;

if : IF '(' rvalue ')' '{' statement_list '}'                                        { $$ = make_pair($3, $6); } ;
elif : ELIF '(' rvalue ')' '{' statement_list '}'                                    { $$ = make_pair($3, $6); } ;
else : ELSE '{' statement_list '}'                                                   { $$ = $3; } ;
while : WHILE '(' rvalue ')' '{' statement_list '}'                                  { $$ = new While{$3, *$6}; } ;
for : FOR '(' LET ID FROM rvalue TO rvalue ')' '{' statement_list '}'                { $$ = new For{$4, $6, $8, new RValue{new Literal{1}}, *$11}; } ;
    | FOR '(' LET ID FROM rvalue TO rvalue STEP rvalue ')' '{' statement_list '}'    { $$ = new For{$4, $6, $8, $10, *$13}; } ;
    ;

lvalue
    : ID                              { $$ = new LValue{$1}; }
    | lvalue '.' ID                   { $$ = new LValue{new MemberAccess{$1, $3}}; }
    | ID '[' rvalue ']'               { $$ = new LValue{new ElementAccess{new LValue{$1}, $3}}; }
    | lvalue '.' ID '[' rvalue ']'    { $$ = new LValue{new ElementAccess{new LValue{new MemberAccess{$1, $3}}, $5}}; }
    ;
rvalue
    : lvalue                          { $$ = new RValue{$1}; }
    | literal                         { $$ = new RValue{$1}; }
    | function_call                   { $$ = new RValue{$1}; }
    | NOT rvalue                      { $$ = new RValue{new UnaryExpression{$2, $1}}; }
    | rvalue AND rvalue               { $$ = new RValue{new BinaryExpression{$1, $3, $2}}; }
    | rvalue OR rvalue                { $$ = new RValue{new BinaryExpression{$1, $3, $2}}; }
    | rvalue EQNE rvalue              { $$ = new RValue{new BinaryExpression{$1, $3, $2}}; }
    | rvalue LTGT rvalue              { $$ = new RValue{new BinaryExpression{$1, $3, $2}}; }
    | rvalue ADDT rvalue              { $$ = new RValue{new BinaryExpression{$1, $3, $2}}; }
    | rvalue PROD rvalue              { $$ = new RValue{new BinaryExpression{$1, $3, $2}}; }
    | '(' rvalue ')'                  { $$ = $2; }
    ;

type
    : TYPE              { $$ = $1; }
    | TYPE '[' ']'      { $$ = $1 + "[]"; if ($$ == "void[]") Pickle::Parser::error("type " + Pickle::Interpreter::green("void[]") + " does not exist"); }
    | ID                { $$ = $1; }
    | ID '[' ']'        { $$ = $1 + "[]"; }
    ;
literal
    : INT               { $$ = new Literal{$1}; }
    | FLOAT             { $$ = new Literal{$1}; }
    | CHAR              { $$ = new Literal{$1}; }
    | STRING            { $$ = new Literal{$1}; }
    | BOOL              { $$ = new Literal{$1}; }
    | '[' rvalue ']'    { $$ = new Literal{$2}; }
    | object_literal    { $$ = new Literal{$1}; }
    ;

object_literal
    : '{' '}'                                   { $$ = new ObjectLiteral; }
    | '{' property_list_values '}'              { $$ = $2; }
    ;
property_list_values
    : ID ':' rvalue                             { $$ = new ObjectLiteral; $$->members.emplace_front($1, $3); }
    | ID ':' rvalue ',' property_list_values    { $$ =                $5; $$->members.emplace_front($1, $3); }
    ;

function_call
    : ID '(' ')'                         { $$ = new FunctionCall; $$->name = $1; }
    | ID '(' argument_list_values ')'    { $$ =               $3; $$->name = $1; }
    ;
argument_list_values
    : rvalue                             { $$ = new FunctionCall; $$->arguments.push_front($1); }
    | rvalue ',' argument_list_values    { $$ =               $3; $$->arguments.push_front($1); }
    ;

%%

void Pickle::Parser::error(const string& message) {
    cerr << Pickle::Interpreter::red("YACC: ") << message << '\n';
}
