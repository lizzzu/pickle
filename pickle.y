%{
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "pickle.h"
int yylex();
void yyerror(char*);
FILE *yyin;
%}

%union {
    int intVal;
    double floatVal;
    char charVal;
    char *stringVal;
    bool boolVal;
    char *type;
    char *identifier;
    char *operator;
}

%type <type> type

%start program
%token DELIM1 DELIM2 DELIM3
%token <type> TYPE
%token IF ELIF ELSE WHILE FOR LET FROM TO STEP
%token CONST BREAK CONTINUE RETURN
%token <operator> AND OR NOT EQNE LTGT ADDT PROD EQ ASGN
%token <identifier> ID
%token <intVal> INT
%token <floatVal> FLOAT
%token <charVal> CHAR
%token <stringVal> STRING
%token <boolVal> BOOL

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
    : /* empty */
    | block1 declaration ';'
    ;
block2
    : /* empty */
    | block2 object
    | block2 function
    ;
block3
    : function
    ;

object
    : ID '{' '}'                { printf("new object type: %s\n", $1); }
    | ID '{' property_list '}'  { printf("new object type: %s\n", $1); }
    ;
property_list
    : type ID ';'
    | type ID ';' property_list
    ;

function
    : type ID '(' ')' '{' statement_list '}'                { printf("new function of type %s: %s\n", $1, $2); }
    | type ID '(' argument_list ')' '{' statement_list '}'  { printf("new function of type %s: %s\n", $1, $2); }
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
    : /* empty */
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
    : ID
    | lvalue '.' ID
    | ID '[' rvalue ']'
    | lvalue '.' ID '[' rvalue ']'
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
    : TYPE          { $$ = strdup($1); }
    | TYPE '[' ']'  { $$ = strdup($1); strcat($$, "[]"); }
    | ID            { $$ = strdup($1); }
    | ID '[' ']'    { $$ = strdup($1); strcat($$, "[]"); }
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

int main() {
    yyin = fopen("input.pickle", "r");
    yyparse();
    fclose(yyin);
    return 0;
}

void yyerror(char* msg) {
    fprintf(stderr, "YACC: %s\n", msg);
}
