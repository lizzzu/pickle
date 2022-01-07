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
%type <pair<deque<string>*, deque<string>*>> property_list
%type <LValue*> lvalue
%type <RValue*> rvalue;
%type <string> type

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
    | block1 declaration ';'
    ;
block2
    :
    | block2 object
    | block2 function
    ;
block3
    : function
    ;

object
    : ID '{' '}'                { $$ = new Object; $$->name = $1; driver.pushObject($$); }
    | ID '{' property_list '}'  { $$ = new Object; $$->name = $1; tie($$->memberTypes, $$->memberNames) = $3; driver.pushObject($$); }
    ;
property_list
    : type ID ';'                { $$ = make_pair(new deque<string>(1, $1), new deque<string>(1, $2)); }
    | type ID ';' property_list  { $$ = $4; $$.first->push_front($1); $$.second->push_front($2); }
    ;

function
    : type ID '(' ')' '{' statement_list '}'                { cout << "new function of type " << $1 << ": " << $2 << '\n'; }
    | type ID '(' argument_list ')' '{' statement_list '}'  { cout << "new function of type " << $1 << ": " << $2 << '\n'; }
    ;
argument_list
    : type ID
    | type ID ',' argument_list
    ;

declaration
    : type ID EQ rvalue
    | CONST TYPE ID EQ rvalue
    ;
assignation
    : lvalue EQ rvalue
    | lvalue ASGN rvalue
    ;

statement_list
    :
    | statement_list declaration ';'
    | statement_list assignation ';'
    | statement_list function_call ';'
    | statement_list if
    | statement_list if elif_list
    | statement_list while
    | statement_list for
    | statement_list BREAK ';'
    | statement_list CONTINUE ';'
    | statement_list RETURN ';'
    | statement_list RETURN rvalue ';'
    ;
elif_list
    : else
    | elif elif_list
    ;

if : IF '(' rvalue ')' '{' statement_list '}' ;
elif : ELIF '(' rvalue ')' '{' statement_list '}' ;
else : ELSE '{' statement_list '}' ;
while : WHILE '(' rvalue ')' '{' statement_list '}' ;
for : FOR '(' LET ID FROM rvalue TO rvalue ')' '{' statement_list '}'
    | FOR '(' LET ID FROM rvalue TO rvalue STEP rvalue ')' '{' statement_list '}'
    ;

lvalue
    : ID                            { $$ = new LValue; $$->content = "str"; }
    | lvalue '.' ID                 { $$ = new LValue; $$->content = new MemberAccess; $$->content.object = $1; $$->content.member = $3; }
    | ID '[' rvalue ']'             { $$ = new LValue; $$->content = new ElementAccess; $$->content.array = $1; $$->content.index = $3; }
    | lvalue '.' ID '[' rvalue ']'  { $$ = new LValue; $$->content = new ElementAccess; $$->content.array = new LValue; $$->content.array->content = new MemberAccess; $$->content.array->content.object = $1; $$->content.array->content.member = $3; $$->content.index = $5; }
    ;
rvalue
    : lvalue
    | literal
    | function_call
    | NOT rvalue
    | rvalue AND rvalue
    | rvalue OR rvalue
    | rvalue EQNE rvalue
    | rvalue LTGT rvalue
    | rvalue ADDT rvalue
    | rvalue PROD rvalue
    | '(' rvalue ')'
    ;

type
    : TYPE          { $$ = $1; }
    | TYPE '[' ']'  { $$ = $1 + "[]"; }
    | ID            { $$ = $1; }
    | ID '[' ']'    { $$ = $1 + "[]"; }
    ;
literal
    : INT
    | FLOAT
    | CHAR
    | STRING
    | BOOL
    | '[' rvalue ']'
    | object_literal
    ;

object_literal
    : '{' '}'
    | '{' property_list_values '}'
    ;
property_list_values
    : ID ':' rvalue
    | ID ':' rvalue ',' property_list_values
    ;

function_call
    : ID '(' ')'
    | ID '(' argument_list_values ')'
    ;
argument_list_values
    : rvalue
    | rvalue ',' argument_list_values
    ;

%%

void Pickle::Parser::error(const string& message) {
    cerr << "YACC: " << message << '\n';
}