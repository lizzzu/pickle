%{
#include <stdio.h>
int yylex();
void yyerror(char*);
FILE *yyin;
%}

%start program
%token DELIM1 DELIM2 DELIM3
%token TYPE VOID CNST
%token IF ELIF ELSE WHILE FOR LET FROM TO STEP
%token BRCN RTRN
%token LOGC BOOL
%token COMP ASGN EQ MATH
%token ID FLT INT CHR STR

%%

program
    : DELIM1 block1 DELIM2 block2 DELIM2 block3 DELIM3 { printf("correct\n"); }
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
    | TYPE ID EQ literal
	| argument_list ',' type ID
    | argument_list ',' TYPE ID EQ literal
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
    : ID EQ expression
    | ID ASGN expression
    ;

literal
    : FLT
    | INT
    | CHR
    | STR
    | BOOL
    ;
expression
    : literal
    | ID '.' ID
    | ID '(' ')'
    | ID '(' call_list ')'
    | ID '[' expression ']'
    | TYPE '[' expression ']'
    | '{' '}'
    | '{' literal_property_list '}'
    | expression LOGC expression
    | expression COMP expression
    | expression MATH expression
    | '(' expression ')'
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
