%{
#include <stdio.h>
int yylex();
void yyerror(char*);
FILE *yyin;
%}

%start program
%token DELIM1 DELIM2 DELIM3 TYPE LET FOR WHILE IF ELIF ELSE FROM TO STEP BRCN LOGC BOOL RTRN CNST ID FLT INT STR CHR COMP MATH

%%

program 
	: DELIM1 block1 DELIM2 block2 DELIM2 block3 DELIM3 { printf("corect\n"); }
	;

block1
	: /* epsilon */
	| declaration block1
	;

block2
	: /* epsilon */
	| ID '{' block1 '}' block2
	| TYPE ID '(' ')' '{' statements '}' block2
	| TYPE ID '(' arguments ')' '{' statements '}' block2
	;

block3
	: /* epsilon */
	| ID
	;

declaration
	: TYPE ID
	| CNST TYPE ID
	| TYPE '[' INT ']' ID
	| CNST TYPE '[' INT ']' ID
	| TYPE ID '=' rvalue
	| CNST TYPE ID '=' rvalue
	;

rvalue
	: INT
	| FLT
	| BOOL
	| STR
	| CHR
	| ID
	;

arguments
	: TYPE ID
	| CNST TYPE ID
	| TYPE '[' ']' ID
	| CNST TYPE '[' ']' ID
	| TYPE ID ',' arguments
	| CNST TYPE ID ',' arguments
	| TYPE '[' ']' ID ',' arguments
	| CNST TYPE '[' ']' ID ',' arguments
	;

statements
	: /* epsilon */
	| RTRN statements
	| RTRN expression statements
	| BRCN statements
	| FOR '(' LET ID FROM INT TO INT ')' '{' statements '}' statements
	| FOR '(' LET ID FROM INT TO INT STEP INT ')' '{' statements '}' statements
	| WHILE '(' logical_expression ')' '{' statements '}' statements
	| if_condition statements
	| if_condition if_list statements
	| IF '(' logical_expression ')' '{' statements '}' statements
	| IF '(' logical_expression ')' '{' statements '}'  statements
	;

if_list
	: elif_condition if_list
	| else_condition
	;

if_condition : IF '(' logical_expression ')' '{' statements '}' ;
elif_condition : ELIF '(' logical_expression ')' '{' statements '}' ;
else_condition : ELSE '{' statements '}' ;

logical_expression
	:
	;

expression
	:
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
