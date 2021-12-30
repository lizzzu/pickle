%{
#include <stdio.h>
int yylex();
void yyerror(char*);
FILE *yyin;
%}

%start S
%token DELIM1 DELIM2 DELIM3 TYPE CTRL RANG BRCN LOGC RTRN CNST ID FLT INT ASGN COMP MATH

%%

S : DELIM1 block DELIM2 block DELIM2 block DELIM3 { printf("corect\n"); }
  ;

block
	: ID { printf("yey\n"); }
	;

%%

int main() {
	yyin = fopen("input.pickle", "r");
	yyparse();
	fclose(yyin);
	return 0;
}

void yyerror(char *msg) {
	fprintf(stderr, "YACC: %s\n", msg);
}
