%{
    #include "tree.h"
    #include "lex.yy.c"

    int failed = 0;
    int error3 =0 ;
    int last_error=0;
    Node *root;
%}
%union {
    struct NODE *nodeval;
}
%locations

%token <nodeval> INT
%token <nodeval> FLOAT
%token <nodeval> SEMI
%token <nodeval> COMMA
%token <nodeval> ASSIGNOP
%token <nodeval> RELOP
%token <nodeval> PLUS MINUS
%token <nodeval> STAR DIV
%token <nodeval> OR
%token <nodeval> AND

%token <nodeval> DOT
%token <nodeval> NOT
%token <nodeval> TYPE
%token <nodeval> LP
%token <nodeval> RP
%token <nodeval> LB
%token <nodeval> RB 
%token <nodeval> LC
%token <nodeval> RC
%token <nodeval> STRUCT
%token <nodeval> RETURN
%token <nodeval> IF
%token <nodeval> ELSE
%token <nodeval> WHILE
%token <nodeval> ID
%token <nodeval> ws
%token <nodeval> newline
%token <nodeval> digit
%token <nodeval> letter
%token <nodeval> COMMENT
%token <nodeval> OTHER
%type <nodeval> Program  ExtdefList Extdef ExtDecList 
%type <nodeval> Specifier StructSpecifier OptTag Tag 
%type <nodeval> VarDec FunDec VarList ParamDec 
%type <nodeval> CompSt StmtList Stmt 
%type <nodeval> DefList Def Dec DecList  
%type <nodeval> Exp Args

%nonassoc error
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LB RB LP RP DOT

%nonassoc ELSE
%%
Program : 
    ExtdefList {
        $$ = createNode("Program", $1->lineno); root = $$;
        insertChild($$, $1); 
        /*if(failed==0 && error3==0)
            printTree(root,0);*/
    }
    ;
ExtdefList : 
    Extdef ExtdefList {
        $$ = createNode("ExtDefList", $1->lineno); 
        insertChild($$, $1); insertChild($$, $2);
    }
    |  {
        $$ = NULL;
    }
    ;
Extdef : 
    Specifier ExtDecList SEMI { 
        $$ = createNode("ExtDef", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    } 
    | Specifier SEMI { 
        $$ = createNode("ExtDef", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | Specifier FunDec CompSt {
        $$ = createNode("ExtDef", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Specifier FunDec SEMI {
        $$ = createNode("ExtDef", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Specifier FunDec error SEMI {
        Err_new(@3.first_line);
        //printf("Error type B at Line %d: Incomplement defination of function\n", @2.first_line);
    }
    ;
ExtDecList : 
    VarDec {
        $$ = createNode("ExtDecList", $1->lineno);
        insertChild($$, $1);
    }
    | VarDec COMMA ExtDecList { 
        $$ = createNode("ExtDecList", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    ;


Specifier : 
    TYPE { 
        $$ = createNode("Specifier", $1->lineno);
        
        insertChild($$, $1);
    }
    | StructSpecifier { 
        $$ = createNode("Specifier", $1->lineno);
        insertChild($$, $1);
    }
    ;
StructSpecifier : 
    STRUCT OptTag LC DefList RC {
        $$ = createNode("StructSpecifier", $1->lineno); 
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4); insertChild($$, $5);
    }
    | STRUCT Tag  {
        $$ = createNode("StructSpecifier", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | STRUCT OptTag LC error RC {Err_new(@4.first_line);}
    ;
OptTag : 
    ID {
        $$ = createNode("OptTag", $1->lineno);
        insertChild($$, $1);
        // insertSymbol($1->strval);
    }
    | /* empty */ { 
        $$ = NULL;
    }
    ;
Tag : 
    ID { 
        $$ = createNode("Tag", $1->lineno);
        insertChild($$, $1);
    }
    ;


VarDec : 
    ID { 
        $$ = createNode("VarDec", $1->lineno);
        insertChild($$, $1);
        // insertSymbol($1->strval);
    }
    | VarDec LB INT RB { 
        $$ = createNode("VarDec", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4);
    }
    ;
FunDec : 
    ID LP VarList RP { 
        $$ = createNode("FunDec", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4);
        // insertSymbol($1->strval);
    }
    | ID LP RP { 
        $$ = createNode("FunDec", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
        // insertSymbol($1->strval);
    }
    ;
VarList : 
    ParamDec COMMA VarList { 
        $$ = createNode("VarList", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | ParamDec { 
        $$ = createNode("VarList", $1->lineno);
        insertChild($$, $1);
    }
    ;
ParamDec : 
    Specifier VarDec { 
        $$ = createNode("ParamDec", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    ;


CompSt : 
    LC DefList StmtList RC { 
        $$ = createNode("CompSt", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4);
    }
    ;
StmtList : 
    Stmt StmtList { 
        $$ = createNode("StmtList", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | /* empty */ { 
        $$ = NULL;
    }
    ;
Stmt : 
    Exp SEMI { 
        $$ = createNode("Stmt", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | Exp error SEMI {
        /* insert into tree */
        $$ = createNode("Stmt",$1->lineno);
        insertChild($$, $1); insertChild($$, $3);
        Err_new(@2.first_line); //yyerror("Missing ;", @2.first_line);
    }
    | error SEMI {
        /* no insert */
        Err_new(@1.first_line); //printf("Unexpected character\n");
    }
    | CompSt { 
        $$ = createNode("Stmt",$1->lineno);
        insertChild($$, $1);
    }
    | RETURN Exp SEMI { 
        $$ = createNode("Stmt",$1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | IF LP Exp RP Stmt {
        $$ = createNode("Stmt", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4); insertChild($$, $5);
    }
    | IF LP Exp RP Stmt ELSE Stmt { 
        $$ = createNode("Stmt", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4); 
        insertChild($$, $5); insertChild($$, $6); insertChild($$, $7); 
    }
    | WHILE LP Exp RP Stmt {
        $$ = createNode("Stmt", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4); 
        insertChild($$, $5);
    }
    | IF LP error RP Stmt {Err_new(@3.first_line);}
    | IF LP error RP Stmt ELSE Stmt {Err_new(@3.first_line);}
    | IF LP Exp RP error ELSE Stmt {Err_new(@5.first_line);}
    | WHILE LP error RP Stmt {Err_new(@3.first_line);}
    ;


DefList : 
    Def DefList { 
        $$ = createNode("DefList", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | /* empty */ { 
        $$ = NULL;
    }
    ;
Def : 
    Specifier DecList  SEMI { 
        $$ = createNode("Def", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Specifier error SEMI {
        /* no symbol declaration */
        Err_new(@2.first_line);//yyerror("Unexpected Def", @2.first_line);
    }
    | Specifier DecList error SEMI {
        /* insert into tree to get symbol info */
        $$ = createNode("Def", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $4);
        Err_new(@3.first_line); 
    }
    ;
DecList : 
    Dec  {
        $$ = createNode("DecList", $1->lineno);
        insertChild($$, $1);
    }
    | Dec COMMA DecList { 
        $$ = createNode("DecList", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    ;
Dec : 
    VarDec { 
        $$ = createNode("Dec", $1->lineno);
        insertChild($$, $1);
    }
    | VarDec ASSIGNOP Exp { 
        $$ = createNode("Dec", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    ;


Exp : 
    Exp ASSIGNOP Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp AND Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp OR Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp RELOP Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp PLUS Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp MINUS Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp STAR Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp DIV Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | LP Exp RP { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp LB Exp error RB { 
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $5);
        Err_new(@4.first_line); //yyerror("Missing ]", @4.first_line);
    }
    | MINUS Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | NOT Exp { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2);
    }
    | ID LP Args RP { 
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4);
    }
    | ID LP RP { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp LB Exp RB { 
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3); insertChild($$, $4);
    }
    | Exp DOT ID { 
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | ID { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1);
    }
    | INT { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1);
    }
    | FLOAT { // d
        $$ = createNode("Exp", $1->lineno);
        insertChild($$, $1);
    }
    | LP error RP {Err_new(@2.first_line);}
    | LP error SEMI {Err_new(@2.first_line);}
    | LP error RC {Err_new(@2.first_line);}
    | ID LP error RP {Err_new(@3.first_line);}
    | ID LP error SEMI {Err_new(@3.first_line);}
    | ID LP error RC {Err_new(@3.first_line);}
    | Exp LB error RB {Err_new(@3.first_line);}
    | Exp LB error SEMI {Err_new(@3.first_line);}
    | Exp LB error RC {Err_new(@3.first_line);}
    ;
Args : 
    Exp COMMA Args { 
        $$ = createNode("Args", $1->lineno);
        insertChild($$, $1); insertChild($$, $2); insertChild($$, $3);
    }
    | Exp { 
        $$ = createNode("Args", $1->lineno);
        insertChild($$, $1);
    }
    ;


%%