%{
#include <stdio.h>
int yylex();
void yyerror(char*);
FILE *yyin;
%}

%start program
%token DELIM1 DELIM2 DELIM3 TYPE CTRL RANG BRCN LOGC BOOL RTRN CNST ID FLT INT STR CHR COMP MATH

%%

program 
	: DELIM1 block1 DELIM2 block2 DELIM2 block3 DELIM3 { printf("corect\n"); }
	;

block1
	: declaration block1
	| declaration
	;

block2
	: ID
	;

block3
	: ID
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
