%{
#include <stdio.h>
int yylex();
void yyerror(char*);
FILE *yyin;
%}

%start program
%token DELIM1 DELIM2 DELIM3
%token TYPE VOID CNST
%token IF ELIF ELSE WHILE FOR LET FROM TO STEP BRCN RTRN
%token AND OR NOT BOOL
%token COMP ASGN EQ ADDT PROD
%token ID FLT INT CHR STR

%left OR
%left AND
%nonassoc COMP
%left ADDT
%left PROD
%nonassoc NOT
%left '.'

%%

program
    : DELIM1 block1 DELIM2 block2 DELIM2 block3 DELIM3
    ;
block1
    : /* epsilon */
    | block1 declaration ';'
    ;
block2
    : /* epsilon */
    | block2 ID '{' property_list '}'
    | block2 block3
    ;
block3
    : type ID '(' ')' '{' statement_list '}'
    | VOID ID '(' ')' '{' statement_list '}'
    | type ID '(' argument_list ')' '{' statement_list '}'
    | VOID ID '(' argument_list ')' '{' statement_list '}'
    ;

type
    : TYPE
    | TYPE '[' ']'
    | ID
    | ID '[' ']'
    ;
property_list
    : type ID ';'
    | type ID ';' property_list
    ;
argument_list
    : type ID
    | type ID ',' argument_list
    ;

statement_list
    : /* epsilon */
    | statement_list declaration ';'
    | statement_list if
    | statement_list if elif_list
    | statement_list while
    | statement_list for
    | statement_list BRCN ';'
    | statement_list RTRN ';'
    | statement_list RTRN expression ';'
    | statement_list assignation ';'
    | statement_list function_call ';'
    ;
elif_list
    : else
    | elif elif_list
    ;

if : IF '(' expression ')' '{' statement_list '}' ;
elif : ELIF '(' expression ')' '{' statement_list '}' ;
else : ELSE '{' statement_list '}' ;
while : WHILE '(' expression ')' '{' statement_list '}' ;

for : FOR '(' LET ID FROM expression TO expression ')' '{' statement_list '}'
    | FOR '(' LET ID FROM expression TO expression STEP expression ')' '{' statement_list '}'
    ;

declaration
    : type ID EQ expression
    | CNST TYPE ID EQ expression
    ;
assignation
    : expression EQ expression
    | expression ASGN expression
    ;

expression
    : literal
    | ID
    | expression '.' ID
    | function_call
    | '[' expression ']'
    | ID '[' expression ']'
    | '{' '}'
    | '{' literal_property_list '}'
    | NOT expression
    | expression AND expression
    | expression OR expression
    | expression COMP expression
    | expression ADDT expression
    | expression PROD expression
    | '(' expression ')'
    ;
literal
    : FLT
    | INT
    | CHR
    | STR
    | BOOL
    ;
function_call
    : ID '(' ')'
    | ID '(' call_list ')'
    ;
call_list
    : expression
    | expression ',' call_list
    ;
literal_property_list
    : ID ':' expression
    | ID ':' expression ',' literal_property_list
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
