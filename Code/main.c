#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "hashtable.h"
#include "ir.h"
extern FILE* yyin;
extern int yylineno;
extern int yydebug;
extern Node *root;
extern int failed;
extern int struct_flag;
extern int last_error;
int main(int argc, char *argv[]) {
    if(argc < 2) return 1;
    FILE *f = fopen(argv[1], "r");
    if(!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yylineno = 1;
    //yydebug=1;
    yyparse();
    if(failed==0)
    {
        semantic_check(root);
    }
    
    CodeList codelisthead = Intercode(root); // 中间代码生成
    if(struct_flag==0)
    {
        FILE *ff;
        if(argv[2] == NULL)
            {ff = fopen("a.ir", "w");}
        else
            ff =fopen(argv[2],"w"); // 构造输出中间代码文件
        print_IR(ff,codelisthead); // 写入生成的中间代码
        fclose(ff); // 关闭文件
    }
    // FILE* asmff;
    // if(argv[2] == NULL)asmff = fopen("output.s", "w");
    // else asmff =fopen(argv[2], "w");
    // // 目标代码生成
    // generate_asm(codelisthead, asmff);
    // fclose(asmff);
    return 0;
}

void yyerror(const char *msg) {
    printf("Error type B at Line %d: %s\n", yylineno, msg);
} 

// 语义分析入口函数
void semantic_check(Node* r){
    //初始化工作，如全局变量初始化;
    // 遍历语法树中的ExtDefList节点
    for(int i=0;i<r->cnt;i++)
    {
        ExtDefList(r->ch[i]);
    }
}
int Err_new(int errorLineno){
    //printf("%d\n",errorLineno);
	if(last_error != errorLineno){
		failed = 1;
		last_error = errorLineno;
		return 1;
  	}else {
		return 0;
	}
}