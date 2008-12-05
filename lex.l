%{
/*	
*	Lexical analyzer for the Toy language
*/
#include	<string.h>	/* for strcmp, strdup & friends */
#include	<stdlib.h>	/* for atoi() */
#include        "globals.h"
#include	"toy+.tab.h"	/* token type definitions from .y file */
extern int	lineno;		/* defined in toy+.y */
%}

alpha			[A-Za-z]
digit			[0-9]
alphanum		[A-Za-z0-9]

%%
[ \t]			break;		/* ignore white space */
"\n" 			++lineno; 

{alpha}{alphanum}*	{
                        if (strcmp(yytext,"write") == 0) 
                                { return WRITE; }
                        else 
                             if (strcmp(yytext,"read") == 0) 
                                { return READ; }
			else
                             if (strcmp(yytext,"while") == 0)
                                { return WHILE; }
                        else
                             if (strcmp(yytext,"if") == 0)
                                { return IF; }
                        else
                             if (strcmp(yytext,"else") == 0)
                                { return ELSE; }
			else 
                             if (strcmp(yytext,"call") == 0) 
                                { return CALL; }
                        else
                             if (strcmp(yytext,"proc") == 0)
                                { return PROC; }
                        else 
                             if (strcmp(yytext,"declare") == 0) 
                                { return DECLARE; }
                        else 
                                { return NAME; }
			}

{digit}+		{
			return NUMBER;
			}

"("			return LPAREN;
")"			return RPAREN;
"{"			return LBRACE;
"}"			return RBRACE;
"="			return ASSIGN;
";"			return SEMICOLON;
"+"			return PLUS;
"-"			return MINUS;
","			return COMMA;

.			{
			fprintf(stderr,
				"Illegal character \'%s\' on line #%d\n",
				yytext,lineno);
			exit(1);
			}

%%

int
yywrap()
{
return 1; /* tell scanner no further files need to be processed */
}

