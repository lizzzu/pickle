%{
#include <stdio.h>
#include "pickle.tab.h"
// extern int yylval;
%}

%option noyywrap

%%

\+\+\+								{ return DELIM1; }
~~~									{ return DELIM2; }
\-\-\-								{ return DELIM3; }

(int|float|char|string|bool|void)	{ ECHO; printf("\n"); return TYPE; }
(for|while|if|elif|else)		 	{ ECHO; printf("\n"); return CTRL; }
(from|to|step)						{ ECHO; printf("\n"); return RANG; }
(break|continue)					{ ECHO; printf("\n"); return BRCN; }
(and|or|not)						{ ECHO; printf("\n"); return LOGC; }
(true|false)			 			{ ECHO; printf("\n"); return BOOL; }
return								{ ECHO; printf("\n"); return RTRN; }
const 								{ ECHO; printf("\n"); return CNST; }

[_a-zA-Z][_a-zA-Z0-9]*				{ ECHO; printf("\n"); return ID; }
[+-]?[1-9][0-9]*\.[0-9]+			{ ECHO; printf("\n"); return FLT; }
[+-]?[1-9][0-9]*					{ ECHO; printf("\n"); return INT; }
\"[^\r\n\"]*\"						{ ECHO; printf("\n"); return STR; }
'[^\r\n'"]'							{ ECHO; printf("\n"); return CHR; }

(==|<>|>|>=|<|<=)					{ ECHO; printf("\n"); return COMP; }
(\+|-|\*|\/|%)						{ ECHO; printf("\n"); return MATH; }

[ \r\n]								{ }
.									{ return yytext[0]; }

%%
