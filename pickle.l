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

(int|float|char|string|bool|void)	{ return TYPE; }
(for|while|if|elif|else)		 	{ return CTRL; }
(from|to|step)						{ return RANG; }
(break|continue)					{ return BRCN; }
(and|or|not|true|false)			 	{ return LOGC; }
return								{ return RTRN; }
const 								{ return CNST; }

[_a-zA-Z][_a-zA-Z0-9]*				{ return ID; }
[+-]?[1-9]+(0-9)*\.(0-9)+			{ return FLT; }
[+-]?[1-9]+(0-9)*					{ return INT; }

=									{ return ASGN; }
(==|<>|>|>=|<|<=)					{ return COMP; }
(\+|-|\*|\/|%)						{ return MATH; }

[\r\n]								{ }
.									{ return yytext[0]; }

%%
