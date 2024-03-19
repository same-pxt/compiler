%{
    #include "syntax.tab.h"
    extern int error3;
    extern int Err_new(int errorLineno);
    extern int last_error;
%}

%option yylineno

ws      [ \t\r]+
newline [\n]
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE int|float
LP "\("
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
digit   [0-9]
letter  [A-Za-z]
ID      (_|{letter})(_|{letter}|{digit})*
INT [+-]?(0|[1-9]{digit}*)
FLOAT   [+-]?({digit}+\.{digit}+((E|e)(\+|-)?{digit}+)?)|(\.{digit}+(E|e)(\+|-)?{digit}+)|({digit}+\.(E|e)(\+|-)?{digit}+)
OTHER .

%%
"//" {printf("Error type A at Line %d: Mysterious character \"%s\".\n",yylineno,yytext);char ch; while((ch = input()) != '\n');error3=1;Err_new(yylineno);}
"/*"  {
    printf("Error type A at Line %d: Mysterious character \"%s\".\n",yylineno,yytext);
    char ch; int done = 0; 
    while(done < 2 && ch != EOF) {
        ch = input();
        if(ch == '*') done = 1;
        else if(ch == '/' && done == 1) done = 2;
        else done = 0;
    }
    error3=1;
    Err_new(yylineno);
}
{STRUCT} { yylval.nodeval = createLeaf("STRUCT", yylineno, yytext); return STRUCT; }
{RETURN} { yylval.nodeval = createLeaf("RETURN", yylineno, yytext); return RETURN; }
{IF} { yylval.nodeval = createLeaf("IF", yylineno, yytext); return IF; }
{ELSE} { yylval.nodeval = createLeaf("ELSE", yylineno, yytext); return ELSE; }
{WHILE} { yylval.nodeval = createLeaf("WHILE", yylineno, yytext); return WHILE; }
{TYPE} { yylval.nodeval = createLeaf("TYPE", yylineno, yytext); return TYPE; }
{ID} { yylval.nodeval = createLeaf("ID", yylineno, yytext); return ID; }
{INT} { yylval.nodeval = createLeaf("INT", yylineno, yytext); return INT; }
{FLOAT} { yylval.nodeval = createLeaf("FLOAT", yylineno, yytext); return FLOAT; }

{ws}  /* 忽略空白字符 */
{newline} {} /* 忽略换行符 */
{SEMI} { yylval.nodeval = createLeaf("SEMI", yylineno, yytext); return SEMI; }
{COMMA} { yylval.nodeval = createLeaf("COMMA", yylineno, yytext); return COMMA; }
{ASSIGNOP} { yylval.nodeval = createLeaf("ASSIGNOP", yylineno, yytext); return ASSIGNOP; }
{RELOP} { yylval.nodeval = createLeaf("RELOP", yylineno, yytext); return RELOP; }
{PLUS} { yylval.nodeval = createLeaf("PLUS", yylineno, yytext); return PLUS; }
{MINUS} { yylval.nodeval = createLeaf("MINUS", yylineno, yytext); return MINUS; }
{STAR} { yylval.nodeval = createLeaf("STAR", yylineno, yytext); return STAR; }
{DIV} { yylval.nodeval = createLeaf("DIV", yylineno, yytext); return DIV; }
{AND} { yylval.nodeval = createLeaf("AND", yylineno, yytext); return AND; }
{OR} { yylval.nodeval = createLeaf("OR", yylineno, yytext); return OR; }
{DOT} { yylval.nodeval = createLeaf("DOT", yylineno, yytext); return DOT; }
{NOT} { yylval.nodeval = createLeaf("NOT", yylineno, yytext); return NOT; }
{LP} { yylval.nodeval = createLeaf("LP", yylineno, yytext); return LP; }
{RP} { yylval.nodeval = createLeaf("RP", yylineno, yytext); return RP; }
{LB} { yylval.nodeval = createLeaf("LB", yylineno, yytext); return LB; }
{RB} { yylval.nodeval = createLeaf("RB", yylineno, yytext); return RB; }
{LC} { yylval.nodeval = createLeaf("LC", yylineno, yytext); return LC; }
{RC} { yylval.nodeval = createLeaf("RC", yylineno, yytext); return RC; }


{OTHER} {printf("Error type A at Line %d: Mysterious character \"%s\".\n",yylineno,yytext);Err_new(yylineno);return OTHER;}
%%
