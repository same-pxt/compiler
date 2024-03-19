#include "ir.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
int var_num, label_num, temp_num; // 新变量/新标签/新临时变量编号
CodeList code_head, code_tail; // 双链表的首部和尾部
Variable var_head, var_tail; // 变量表的首部和尾部
char*last_array_name;
Operand last_array_base=NULL;
Operand last_array_offset=NULL;
int struct_flag=0;
int min(int a,int b)
{
    if(a<b)
        return a;
    return b;
}
CodeList Intercode(Node* Root) {
    // 首先判断AST是否为空
    // 接着判断AST结构是否正确（满足C—文法要求）
    if (Root == NULL) {
        return NULL;
    }
    if (strcmp(Root->name, "Program") != 0) {
        return NULL;
    }

    // 初始化全局变量中间代码变量等列表
    code_head = code_tail = NULL;
    var_tail = var_head = NULL;

    // 设定变量、标签、临时变量标号
    var_num = 1;
    label_num = 1;
    temp_num = 1;

    code_head=translate_ExtDefList(Root->ch[0]);

    return code_head;
}
CodeList translate_ExtDefList(Node* ExtDefList)
{
    if(ExtDefList==NULL)
        return NULL;
    flag("ExtDefList");
    CodeList c1=translate_ExtDef(ExtDefList->ch[0]);
    CodeList c2=translate_ExtDefList(ExtDefList->ch[1]);
    return concatenate(c1,c2);
}
CodeList translate_ExtDef(Node* ExtDef) {
    // 检查语法树是否为空
    if (ExtDef == NULL) {
        return NULL;
    }
    flag("Extdef");
    // Specifier SEMI
    if (strcmp(ExtDef->ch[1]->name, "SEMI") == 0) {
        return NULL;
    }
    // 检查是否有全局标识符定义
    //Specifier ExtDecList SEMI
    if (strcmp(ExtDef->ch[1]->name,"ExtDecList") == 0) {
        return NULL;
    }

    // 依次检查是否是函数定义
    // 实验三不包含全局变量，结构体定义也已经在实验二中处理并添加到符号表中了
    //  Specifier FunDec CompSt
    if (strcmp(ExtDef->ch[0]->name,"Specifier")==0 && 
        strcmp(ExtDef->ch[1]->name,"FunDec")==0 && 
        strcmp(ExtDef->ch[2]->name,"CompSt")==0 ) 
    {
        CodeList c1 = translate_FunDec(ExtDef->ch[1]);
        CodeList c2 = translate_CompSt(ExtDef->ch[2]);
        
        // 合并函数声明和函数体
        return concatenate(c1, c2);
    } else {
        fprintf(stderr, "error ExtDef!\n");
        return NULL;
    }
}

CodeList translate_FunDec(Node* FunDec) {
    if((FunDec==NULL))
        return NULL;
    flag("FunDec");
    if (strcmp(FunDec->ch[0]->name,"ID")==0 && 
        strcmp(FunDec->ch[1]->name,"LP")==0 && 
        strcmp(FunDec->ch[2]->name,"RP")==0 ) 
    { // 处理无参函数
        // 初始化函数信息，获取函数名
        InterCode ic = new_InterCode(IC_FUNC);
        ic->u.func = FunDec->ch[0]->strval;
        CodeList c1 = new_CodeList(ic);
        return c1;
    } else if (strcmp(FunDec->ch[0]->name,"ID")==0 && 
        strcmp(FunDec->ch[1]->name,"LP")==0 && 
        strcmp(FunDec->ch[2]->name,"VarList")==0 &&
        strcmp(FunDec->ch[3]->name,"RP")==0){
        // 初始化函数信息，获取函数名
        InterCode ic = new_InterCode(IC_FUNC);
        ic->u.func = FunDec->ch[0]->strval;
        CodeList c1 = new_CodeList(ic);
        // 获取参数列表
        HashNode t=find(FunDec->ch[0]->strval);
        FieldList params = t->type->u.function->param;
        while (params != NULL) {
            // 构造函数参数列表判断参数类型（选做内容中需要支持结构体和数组作为参数）
            if (params->type->kind == BASIC) {
                CodeList c2=genInterCode(IC_PARAM,new_Operand(Em_VARIABLE,params->name));
                c1=concatenate(c1,c2);
                params=params->tail;
            } 
            else { // 处理数组或者结构体
                // ...
                if(params->type->kind==STRUCTURE)
                {
                    printf("Cannot translate: Code contains variables or parameters of structure type\n");
                    assert(0);
                    struct_flag=1;
                    return NULL;
                }
                else if(params->type->kind==ARRAY)
                {
                    if(params->type->u.array.elem->kind==STRUCTURE)
                    {
                        printf("Cannot translate: Code contains variables or parameters of structure type\n");
                        assert(0);
                        struct_flag=1;
                        return NULL;
                    }
                    //printf("check %s is addr\n",params->name);
                    HashNode Y=find(params->name);
                    Y->isaddr=1;
                    CodeList c2=genInterCode(IC_PARAM,new_Operand(Em_ADDRESS,params->name));
                    c1=concatenate(c1,c2);
                    params=params->tail;
                }
                
            }
        }
        return c1;
    } else {
        fprintf(stderr, "error FunDec!\n");
        return NULL;
    }
}

CodeList translate_CompSt(Node* CompSt)
{
    // LC DefList StmtList RC
    if (CompSt == NULL)
        return NULL;
    flag("CompSt");
    if (CompSt->ch[1]!=NULL&&strcmp(CompSt->ch[1]->name, "RC") == 0)
        return NULL;

    // 处理DefList节点
    CodeList c1 = NULL;
    // 处理StmtList节点
    CodeList c2 = NULL;
    if (CompSt->ch[1]!=NULL&&strcmp(CompSt->ch[1]->name, "DefList") == 0)
    {
        c1 = translate_DefList(CompSt->ch[1]);
    }
    
    if (CompSt->cnt==4 && strcmp(CompSt->ch[2]->name, "StmtList") == 0)
    {
        c2 = translate_StmtList(CompSt->ch[2]);
    }
    // 合并c1和c2
    return concatenate(c1, c2);
}

CodeList translate_DefList(Node* DefList)
{
    if(DefList==NULL)
        return NULL;
    flag("Deflist");
    //Def DefList
    CodeList c1=translate_Def(DefList->ch[0]);
    CodeList c2=translate_DefList(DefList->ch[1]);
    return concatenate(c1,c2);
}

CodeList translate_Def(Node* Def)
{
    if(Def==NULL)
        return;
    flag("Def");
    //Specifier DecList SEMI
    return translate_DecList(Def->ch[1]);
}

CodeList translate_DecList(Node* DecList)
{
    if(DecList==NULL)
        return NULL;
    flag("Declist");
    if(DecList->cnt==1)
    {
        //Dec
        return translate_Dec(DecList->ch[0]);
    }
    else if(DecList->cnt==3)
    {
        //Dec COMA DecList
        CodeList c1=translate_Dec(DecList->ch[0]);
        CodeList c2=translate_DecList(DecList->ch[2]);
        return concatenate(c1,c2);
    }
}
CodeList translate_Dec(Node* Dec)
{
    if(Dec==NULL)
        return NULL;
    flag("Dec");
    if(Dec->cnt==1)
    {
        //VarDec
        return translate_VarDec(Dec->ch[0],NULL);
    }
    else if(Dec->cnt==3)
    {
        //VarDec ASSIGNOP Exp
        Operand t1=new_temp();
        CodeList c1= translate_VarDec(Dec->ch[0],t1);
        Operand t2=new_temp();
        CodeList c2=translate_Exp(Dec->ch[2],t2);
        return concatenate(concatenate(c1,c2),genInterCode(IC_ASSIGN,t1,t2));
    }
}

CodeList translate_StmtList(Node* StmtList)
{
    //Stmt Stmtlist
    if(StmtList==NULL)
        return NULL;
    flag("StmtList");
    CodeList c1=translate_Stmt(StmtList->ch[0]);
    CodeList c2=translate_StmtList(StmtList->ch[1]);
    return concatenate(c1,c2);
}
CodeList translate_Stmt(Node* Stmt)
{
    if(Stmt==NULL)
        return NULL;
    flag("Stmt");
    // Stmt Exp SEMI
    //     | CompSt
    //     | RETURN Exp SEMI
    //     | IF LP Exp RP Stmt
    //     | IF LP Exp RP Stmt ELSE Stmt
    //     | WHILE LP Exp RP Stmt
    if(strcmp(Stmt->ch[0]->name,"Exp")==0)
    {
        return translate_Exp(Stmt->ch[0],NULL);
    }
    else if(strcmp(Stmt->ch[0]->name,"CompSt")==0)
    {
        return translate_CompSt(Stmt->ch[0]);
    }
    else if(strcmp(Stmt->ch[0]->name,"RETURN")==0)
    {
        Operand t1 = new_temp();
        CodeList c1=translate_Exp(Stmt->ch[1], t1);
        CodeList c2=genInterCode(IC_RETURN, t1);
        return concatenate(c1,c2);
    }
    else if(strcmp(Stmt->ch[0]->name,"IF")==0)
    {
        if(Stmt->cnt==5)
        {
            //IF LP Exp RP Stmt
            Operand lable1=new_label();
            Operand label2=new_label();
            CodeList c1=translate_Cond(Stmt->ch[2],lable1,label2);         
            CodeList c2=genInterCode(IC_LABEL,lable1);
            CodeList c3=translate_Stmt(Stmt->ch[4]);
            CodeList c4=genInterCode(IC_LABEL,label2);
            CodeList c5=concatenate(c1,c2);
            CodeList c6=concatenate(c3,c4);
            CodeList c7=concatenate(c5,c6);
            return c7;
        }
        else if(Stmt->cnt==7)
        {
            //IF LP Exp RP Stmt ELSE Stmt
            Operand lable1=new_label();
            Operand label2=new_label();
            CodeList c1=translate_Cond(Stmt->ch[2],lable1,label2);
            CodeList c2=genInterCode(IC_LABEL,lable1);
            CodeList c3=translate_Stmt(Stmt->ch[4]);
            Operand label3=new_label();
            CodeList c4=genInterCode(IC_GOTO,label3);
            CodeList c5=genInterCode(IC_LABEL,label2);
            CodeList c6=translate_Stmt(Stmt->ch[6]);
            CodeList c7=genInterCode(IC_LABEL,label3);
            CodeList c8=concatenate(c1,c2);
            CodeList c9=concatenate(c3,c4);
            CodeList c10=concatenate(c5,c6);
            CodeList c11=concatenate(c8,c9);
            CodeList c12=concatenate(c11,c10);
            CodeList c13=concatenate(c12,c7);
            return c12;
        }
    }
    else if(strcmp(Stmt->ch[0]->name,"WHILE")==0)
    {
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();

        CodeList c1=genInterCode(IC_LABEL, label1);
        CodeList c2=translate_Cond(Stmt->ch[2], label2, label3);
        CodeList c3=genInterCode(IC_LABEL, label2);
        CodeList c4=translate_Stmt(Stmt->ch[4]);
        CodeList c5=genInterCode(IC_GOTO, label1);
        CodeList c6=genInterCode(IC_LABEL, label3);
        return concatenate(concatenate(c1,c2),concatenate(concatenate(c3,c4),concatenate(c5,c6)));
    }
}
CodeList translate_Exp(Node* Exp, Operand place)
{
    if(Exp==NULL)
        return NULL;
    flag("Exp");
    // Exp  Exp ASSIGNOP Exp
    //     | Exp AND Exp
    //     | Exp OR Exp
    //     | Exp RELOP Exp
    //     | Exp PLUS Exp
    //     | Exp MINUS Exp
    //     | Exp STAR Exp
    //     | Exp DIV Exp
    //     | LP Exp RP
    //     | MINUS Exp
    //     | NOT Exp
    //     | ID LP Args RP
    //     | ID LP RP
    //     | Exp LB Exp RB
    //     | Exp DOT ID
    //     | ID
    //     | INT
    //     | FLOAT

    if(strcmp(Exp->ch[0]->name,"INT")==0)
    {
        //printf("INT\n");
        int val = Exp->ch[0]->intval;
        InterCode ic = new_InterCode(IC_ASSIGN,place,new_Operand(Em_CONSTANT,val));
        return new_CodeList(ic);
    }
    else if(strcmp(Exp->ch[0]->name,"ID")==0)
    {
        //printf("ID\n");
        if(Exp->cnt==1)
        {
            HashNode t=find(Exp->ch[0]->strval);
            if(t!=NULL)
            {
                if(t->type->kind==BASIC)
                {
                    set_newOPerand(place,Em_VARIABLE,(void*)newstring(Exp->ch[0]->strval));
                }
                else if(t->type->kind==ARRAY)
                {
                    if(t->type->u.array.elem->kind==STRUCTURE)
                    {
                        printf("Cannot translate: Code contains variables or parameters of structure type\n");
                        assert(0);
                        struct_flag=1;
                        set_newOPerand(place,Em_STRUCT,(void*)newstring(Exp->ch[0]->strval));
                    }
                    if(t->isaddr==0)
                    {
                        if(t->type->u.array.elem->kind==ARRAY)
                            set_newOPerand(place,Em_ARR,(void*)newstring(Exp->ch[0]->strval));
                        else 
                            set_newOPerand(place,Em_VARIABLE,(void*)newstring(Exp->ch[0]->strval));
                    }
                    else
                    {
                        set_newOPerand(place,Em_ARR,(void*)newstring(Exp->ch[0]->strval));
                    }
                    last_array_name=Exp->ch[0]->strval;
                }
                else if(t->type->kind==STRUCTURE)
                {
                    printf("Cannot translate: Code contains variables or parameters of structure type.\n");
                    assert(0);
                    struct_flag=1;
                    set_newOPerand(place,Em_STRUCT,(void*)newstring(Exp->ch[0]->strval));
                }
                place->type=t->type;
                return NULL;
            }
            return NULL;
        }
        else if(strcmp(Exp->ch[1]->name,"LP")==0)
        {
            if(Exp->cnt==3)
            {
                Operand funcTemp =new_Operand(Em_FUNC, newstring(Exp->ch[0]->strval));
                if (strcmp(Exp->ch[0]->strval, "read")==0) 
                {
                    return genInterCode(IC_READ, place);
                } 
                else 
                {
                    if (place) {
                        return genInterCode(IC_CALL, place, funcTemp);
                    } else {
                        Operand temp = new_temp();
                        return genInterCode(IC_CALL, temp, funcTemp);
                    }
                }
            }
            else if (Exp->cnt==4)
            {
                Operand funcTemp =new_Operand(Em_FUNC, newstring(Exp->ch[0]->strval));
                ArgList *temp=(ArgList)malloc(sizeof(struct _ArgList));
                CodeList c1=translate_Args(Exp->ch[2],temp);
                if(strcmp(Exp->ch[0]->strval, "write")==0)
                {
                    return concatenate(c1,genInterCode(IC_WRITE,(*temp)->args));
                }
                else
                {
                    CodeList c2=NULL;
                    ArgList arglist=*temp;
                    while (arglist)
                    {
                        if(arglist->args->kind==Em_VARIABLE)
                        {
                            c2=concatenate(c2,genInterCode(IC_ARG,arglist->args));
                        }
                        else 
                        {
                            c2=concatenate(c2,genInterCode(IC_ARG,arglist->args));
                        }
                        arglist=arglist->next;
                    }


                    if (place) {
                        c2=concatenate(c2,genInterCode(IC_CALL, place, funcTemp));
                    } else {
                        Operand temp = new_temp();
                        c2=concatenate(c2,genInterCode(IC_CALL, temp, funcTemp));
                    }
                    // printf("*****output2****\n");
                    // FILE *ff = fopen("output2", "a");
                    // print_IR(ff,c2);
                    // fclose(ff);
                    // printf("finised\n");
                    return concatenate(c1,c2);
                }
                
            }   
            
        }
    }
    else if(strcmp(Exp->ch[0]->name,"LP")==0)
    {
        //LP Exp RP
        return translate_Exp(Exp->ch[1],place);
    }
    else if(strcmp(Exp->ch[0]->name,"Exp")==0)
    {
        if(strcmp(Exp->ch[1]->name,"ASSIGNOP")==0)
        {
            Operand t1=new_temp();
            CodeList c1=translate_Exp(Exp->ch[0],t1);
            Operand t2=new_temp();
            CodeList c2=translate_Exp(Exp->ch[2],t2);
            if((strcmp(Exp->ch[0]->ch[0]->name,"ID")==0) && (strcmp(Exp->ch[2]->ch[0]->name,"ID")==0))
            {
                HashNode p1=find(t1->u.name);
                HashNode p2=find(t2->u.name);
                if(p1->type->kind==ARRAY&& p2->type->kind==ARRAY)
                {
                    int idx1=p1->type->u.array.size;
                    int idx2=p2->type->u.array.size;

                    Type tp1=p1->type->u.array.elem;
                    Type tp2=p2->type->u.array.elem;

                    Operand Width1=new_Operand(Em_CONSTANT, getSize(tp1));
                    Operand Width2=new_Operand(Em_CONSTANT, getSize(tp2));
                    Operand target1 = new_temp();
                    CodeList q1=genInterCode(IC_GET_ADDR, target1, t1);
                    Operand target2 =new_temp();
                    q1=concatenate(q1,genInterCode(IC_GET_ADDR, target2, t2));
                    int cnt=0;
                    CodeList q2=NULL;
                    while(cnt<min(idx1,idx2))
                    {
                        Operand offset1=new_temp();
                        Operand offset2=new_temp();

                        Operand idxt=new_Operand(Em_CONSTANT, cnt);
                        CodeList z3=(concatenate(genInterCode(IC_MUL, offset1, idxt, Width1),genInterCode(IC_MUL, offset2, idxt, Width2)));

                        Operand w1=new_temp();
                        Operand w2=new_temp();
                        w1->kind=Em_ADDRESS;
                        w2->kind=Em_ADDRESS;
                        CodeList z4=(concatenate(genInterCode(IC_PLUS, w1, target1, offset1),genInterCode(IC_PLUS, w2, target2, offset2)));
                        CodeList z5=genInterCode(IC_ASSIGN,w1,w2);
                        
                        q2=concatenate(q2,concatenate(z3,concatenate(z4,z5)));
                        cnt++;
                    }
                    CodeList q3=concatenate(q1,q2);
                    return q3;
                }
                else 
                {
                    CodeList q1=concatenate(c1,c2);
                    CodeList q2=concatenate(q1,genInterCode(IC_ASSIGN,t1,t2));
                    return q2;
                }
            }
            else
            {
                CodeList q1=concatenate(c1,c2);
                CodeList q2=concatenate(q1,genInterCode(IC_ASSIGN,t1,t2));
                return q2;
            }
        }
        else if(strcmp(Exp->ch[1]->name,"AND")==0 ||strcmp(Exp->ch[1]->name,"OR")==0 || strcmp(Exp->ch[1]->name,"RELOP")==0)
        {
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand true_num = new_Operand(Em_CONSTANT, 1);
            Operand false_num = new_Operand(Em_CONSTANT, 0);
            CodeList c1=genInterCode(IC_ASSIGN, place, false_num);
            CodeList c2=translate_Cond(Exp, label1, label2);
            CodeList c3=genInterCode(IC_LABEL, label1);
            CodeList c4=genInterCode(IC_ASSIGN, place, true_num);
            return concatenate(concatenate(c1,c2),concatenate(c3,c4));
        }
        else if(strcmp(Exp->ch[1]->name,"LB")==0)
        {
            Operand base = new_temp();
            CodeList c2=translate_Exp(Exp->ch[0], base);
            Operand idx = new_temp();
            CodeList c1=translate_Exp(Exp->ch[2], idx);
            Operand width=NULL;
            Operand offset = new_temp();
            Operand target=NULL;
            CodeList c4=NULL;
            HashNode item = find(base->u.name);
            if(item->type->kind==STRUCTURE)
            {
                printf("Cannot translate: Code contains variables or parameters of structure type\n");
                assert(0);
                set_newOPerand(place,Em_STRUCT,(void*)item->name);
                return NULL;
            }
            if (base->kind == Em_VARIABLE ) {
                
                HashNode item = find(base->u.name);
                target = new_temp();
                c4=genInterCode(IC_GET_ADDR, target, base);
                
                if(item==NULL)
                {
                    item=find(last_array_name);
                }
                width = new_Operand(Em_CONSTANT, getSize(item->type->u.array.elem));
                CodeList c3=genInterCode(IC_MUL, offset, idx, width);
                CodeList c5=genInterCode(IC_PLUS, place, target, offset);
                place->kind = Em_ADDRESS;
                return concatenate(c1,concatenate(c2,concatenate(c3,concatenate(c4,c5))));
            } 
            else
            {
                HashNode item = find(base->u.name);
                if(item==NULL)
                {
                    item=find(last_array_name);
                }
                if(item->type->kind==STRUCTURE)
                {
                    printf("Cannot translate: Code contains variables or parameters of structure type\n");
                    assert(0);
                    return NULL;
                }
                if(item->isaddr==1)
                {
                    target = new_Operand(base->kind,base->u.name);
                    width = new_Operand(Em_CONSTANT, getSize(item->type->u.array.elem));
                    CodeList c3=genInterCode(IC_MUL, offset, idx, width);
                    CodeList c5=genInterCode(IC_PLUS, place, target, offset);
                    place->kind = Em_ADDRESS;
                    return concatenate(c1,concatenate(c2,concatenate(c3,concatenate(c4,c5))));
                }
                else if(base->type->kind==ARRAY && base->type->u.array.elem->kind==ARRAY)
                {
                    width = new_Operand(Em_CONSTANT, getSize(base->type->u.array.elem));
                    CodeList c3=genInterCode(IC_MUL, offset, idx, width);
                    last_array_offset=offset;
                    last_array_base=base;
                    place->kind =Em_ARR;
                    place->type=base->type->u.array.elem;
                    place->u.name=base->u.name;
                    return concatenate(c1,concatenate(c2,concatenate(c3,c4)));
                }
                else if(base->type->u.array.elem->kind==BASIC)
                {
                    width = new_Operand(Em_CONSTANT, getSize(base->type->u.array.elem));
                    CodeList c3=genInterCode(IC_MUL, offset, idx, width);
                    Operand w=new_temp();

                    //two dim array
                    c4=genInterCode(IC_PLUS, w, last_array_offset, offset);
                    last_array_base->kind=Em_VARIABLE;
                    Operand r=new_temp();
                    CodeList c6=genInterCode(IC_GET_ADDR,r,last_array_base);
                    place->kind=Em_ADDRESS;
                    CodeList c5=genInterCode(IC_PLUS, place, r, w);
                    Type y=(Type)malloc(sizeof(struct Type_));
                    y->kind=ARRAY;
                    y->u.array.elem=base;
                    y->u.array.size=base->type->u.array.size;
                    place=base;
                    place->type=y;
                    return concatenate(c1,concatenate(c2,concatenate(c3,concatenate(c4,concatenate(c6,c5)))));
                }
                else 
                {
                    printf("Cannot translate: Code contains variables or parameters of structure type\n");
                    assert(0);
                    return NULL;
                }
            }
        }
        else if(strcmp(Exp->ch[1]->name,"DOT")==0)
        {
            printf("Cannot translate: Code contains variables or parameters of structure type\n");
            assert(0);
            struct_flag=1;
            Operand t=new_temp();
            translate_Exp(Exp->ch[0],t);
            set_newOPerand(place,Em_STRUCT,(void*)t->u.name);
            return NULL;
        }
        else
        {
            Operand t1=new_temp();
            Operand t2=new_temp();
            CodeList c1=translate_Exp(Exp->ch[0],t1);
            CodeList c2=translate_Exp(Exp->ch[2],t2);
            CodeList c3=NULL;
            if(strcmp(Exp->ch[1]->name,"PLUS")==0)
            {
                c3= genInterCode(IC_PLUS,place,t1,t2);
            }
            else if(strcmp(Exp->ch[1]->name,"MINUS")==0)
            {
                c3= genInterCode(IC_SUB,place,t1,t2);
            }
            else if(strcmp(Exp->ch[1]->name,"STAR")==0)
            {
                c3= genInterCode(IC_MUL,place,t1,t2);
            }
            else if(strcmp(Exp->ch[1]->name,"DIV")==0)
            {
                c3= genInterCode(IC_DIV,place,t1,t2);
            }
            return concatenate(concatenate(c1,c2),c3);
        }
    }
    else if(strcmp(Exp->ch[0]->name,"MINUS")==0)
    {
        Operand t1 = new_temp();
        CodeList c1=translate_Exp(Exp->ch[1], t1);
        Operand zero = new_Operand(Em_CONSTANT, 0);
        CodeList c2=genInterCode(IC_SUB, place, zero, t1);
        return concatenate(c1,c2);
    }
    else if(strcmp(Exp->ch[0]->name,"NOT")==0)
    {
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand true_num = new_Operand(Em_CONSTANT, 1);
        Operand false_num = new_Operand(Em_CONSTANT, 0);
        CodeList c1=genInterCode(IC_ASSIGN, place, false_num);
        CodeList c2=translate_Cond(Exp, label1, label2);
        CodeList c3=genInterCode(IC_LABEL, label1);
        CodeList c4=genInterCode(IC_ASSIGN, place, true_num);
        return concatenate(concatenate(c1,c2),concatenate(c3,c4));
    }
}

CodeList translate_VarDec(Node* VarDec,Operand place)
{
    if(VarDec==NULL)
        return NULL;
    flag("VarDec");
    //ID
    if(strcmp(VarDec->ch[0]->name,"ID")==0)
    {
        HashNode temp=find(VarDec->ch[0]->strval);
        if(temp!=NULL)
        {
            Type t=temp->type;
            if(t->kind==BASIC)
            {
                if(place)
                {
                    set_newOPerand(place,Em_VARIABLE,(void*)newstring(temp->name));
                    return NULL;
                }
                return NULL;
            }
            else if(t->kind==ARRAY)
            {
                if (t->u.array.elem->kind == ARRAY) {
                    return genInterCode(IC_DEC,new_Operand(Em_VARIABLE, newstring(temp->name)),getSize(t));
                } 
                else if(t->u.array.elem->kind==STRUCTURE)
                {
                    
                    printf("Cannot translate: Code contains variables or parameters of structure type\n");
                    assert(0);
                    struct_flag=1;
                    return NULL;
                }
                else 
                {
                    //printf("%s\n",temp->param->name);
                    return genInterCode(IC_DEC,new_Operand(Em_VARIABLE, newstring(temp->name)),getSize(t));
                }

            }
            else if(t->kind==STRUCTURE)
            {
                printf("Cannot translate: Code contains variables or parameters of structure type\n");
                assert(0);
                struct_flag=1;
                return NULL;
            }
        }
        else
        {
            //printf("not find\n");
        }
    }
    //VarDec LB INT RB
    if(strcmp(VarDec->ch[0]->name,"VarDec")==0 &&
        strcmp(VarDec->ch[2]->name,"INT")==0)
    {
        return translate_VarDec(VarDec->ch[0],place);
    }
}
CodeList concatenate(CodeList list1,CodeList list2)
{
    if (list1 == NULL) {
        return list2;
    }
    if (list2 == NULL) {
        return list1;
    }

    CodeList ptr = list1;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = list2;
    if (list2 != NULL) {
        list2->prev = ptr;
    }
    return list1;
}

InterCode new_InterCode(int kind,...)
{
    InterCode p=(InterCode)malloc(sizeof(struct _InterCode));
    p->kind=kind;
    va_list valist;
    switch (kind) {
        case IC_LABEL:
        case IC_FUNC:
        case IC_RETURN:
        case IC_READ:
        case IC_PARAM:
        case IC_WRITE:
        case IC_GOTO:
        case IC_ARG:
            va_start(valist, 1);
            p->u.op = va_arg(valist, Operand);
            break;
        case IC_ASSIGN:
        case IC_GET_ADDR:
        case IC_READ_ADDR:
        case IC_WRITE_ADDR:
        case IC_CALL:
            va_start(valist, 2);
            p->u.assign.left = va_arg(valist, Operand);
            p->u.assign.right = va_arg(valist, Operand);
            break;
        case IC_IF_GOTO:
            va_start(valist, 4);
            p->u.if_goto.x = va_arg(valist, Operand);
            p->u.if_goto.relop = va_arg(valist, Operand);//remaining bug
            p->u.if_goto.y = va_arg(valist, Operand);
            p->u.if_goto.z = va_arg(valist, Operand);
            break;
        case IC_PLUS:
        case IC_SUB:
        case IC_MUL:
        case IC_DIV:
            va_start(valist, 3);
            p->u.binop.result = va_arg(valist, Operand);
            p->u.binop.op1 = va_arg(valist, Operand);
            p->u.binop.op2 = va_arg(valist, Operand);
            break;
        case IC_DEC:
            va_start(valist, 2);
            p->u.dec.op = va_arg(valist, Operand);
            p->u.dec.size = va_arg(valist, int);
            break;
        //... To be implement
    }
    return p;
}

CodeList new_CodeList(InterCode code)
{
    CodeList p = (CodeList)malloc(sizeof(struct _CodeList));
    p->code=code;
    p->prev=NULL;
    p->next=NULL;
}
Operand new_Operand(int kind,...)
{
    Operand p=(Operand)malloc(sizeof(struct _Operand));
    p->kind=kind;
    va_list valist;
    va_start(valist, 1);
    switch (kind) {
        case Em_CONSTANT:
            p->u.val = va_arg(valist, int);
            break;
        case Em_VARIABLE:
        case Em_ADDRESS:
        case Em_LABEL:
        case Em_RELOP:
        case Em_FUNC:
        case Em_ARR:
            p->u.name = va_arg(valist, char*);
            break;
    }
    return p;
}
char* newstring(const char* source) {
    // 计算源字符串的长度
    size_t length = strlen(source);

    // 为新字符串分配内存空间（包括结尾的空字符'\0'）
    char* newStr = (char*)malloc((length + 1) * sizeof(char));

    // 复制源字符串到新字符串
    strcpy(newStr, source);

    return newStr;
}
Operand new_temp()
{
    char tName[10] = {0};
    tName[0]='t';
    int digit=temp_num;
    int num=temp_num;
    int size=0;
    while (num>0)
    {
        num=num/10;
        size++;
    }
    num=temp_num;
    while(num!=0)
    {
        digit=num%10;
        tName[size]=digit+'0';
        size--;
        num=num/10;
    }
    temp_num++;
    Operand temp = new_Operand(Em_VARIABLE, newstring(tName));
    return temp;
}
Operand new_label()
{
    char tName[10] = "label";
    int digit=label_num;
    int num=label_num;
    int size=0;
    while (num>0)
    {
        num=num/10;
        size++;
    }
    num=label_num;
    while(num!=0)
    {
        digit=num%10;
        tName[size+4]=digit+'0';
        size--;
        num=num/10;
    }
    label_num++;
    Operand temp = new_Operand(Em_LABEL, newstring(tName));
    return temp;
}
CodeList genInterCode(int kind,...)
{
    va_list valist;
    Operand temp=NULL;
    Operand result=NULL,op1=NULL,op2=NULL;
    Operand relop=NULL;
    int size=0;
    CodeList newCodeList=NULL;
    switch (kind)
    {
        case IC_LABEL:
        case IC_FUNC:
        case IC_RETURN:
        case IC_ARG:
        case IC_GOTO:
        case IC_PARAM:
        case IC_READ:
        case IC_WRITE:
            va_start(valist,1);
            op1=va_arg(valist,Operand);
            if(kind!=IC_PARAM)
            {
                if(op1->kind==Em_ADDRESS)
                {
                    temp=new_temp();
                    newCodeList=genInterCode(IC_READ_ADDR,temp,op1);
                    op1=temp;
                }
                newCodeList=concatenate(newCodeList,new_CodeList(new_InterCode(kind,op1)));
            }
            else
            {
                if(op1->kind==Em_ADDRESS)
                {
                    temp=new_temp();
                    newCodeList=concatenate(new_CodeList(new_InterCode(kind,op1)),genInterCode(IC_READ_ADDR,temp,op1));
                }
                else
                {
                    newCodeList=new_CodeList(new_InterCode(kind,op1));
                }
            }
            return newCodeList;
        case IC_ASSIGN:
        case IC_CALL:
        case IC_GET_ADDR:
        case IC_READ_ADDR:
        case IC_WRITE_ADDR:
            va_start(valist,2);
            op1=va_arg(valist,Operand);
            op2=va_arg(valist,Operand);

            if(kind==IC_ASSIGN && (op1->kind==Em_ADDRESS || op2->kind==Em_ADDRESS))
            {
                CodeList c1=NULL;
                CodeList c2=NULL;
                if (op1->kind == Em_ADDRESS && op2->kind != Em_ADDRESS)
                {
                    c1=genInterCode(IC_WRITE_ADDR, op1, op2);
                }
                else if (op2->kind == Em_ADDRESS && op1->kind != Em_ADDRESS)
                {
                    c1=genInterCode(IC_READ_ADDR, op1, op2);
                }  
                else {
                    temp = new_temp();
                    c1=genInterCode(IC_READ_ADDR, temp, op2);
                    c2=genInterCode(IC_WRITE_ADDR, op1, temp);
                }
                return concatenate(newCodeList,concatenate(c1,c2));
            }
            else
            {
                
                newCodeList=new_CodeList(new_InterCode(kind,op1,op2));
                return newCodeList;
            }
            break;
        case IC_PLUS:
        case IC_SUB:
        case IC_MUL:
        case IC_DIV:
            va_start(valist,3);
            result = va_arg(valist,Operand);
            op1=va_arg(valist,Operand);
            op2=va_arg(valist,Operand);
            if (op1->kind == Em_ADDRESS) {
                
                temp = new_temp();
                newCodeList=concatenate(newCodeList,genInterCode(IC_READ_ADDR, temp, op1));
                op1 = temp;
            }
            if (op2->kind == Em_ADDRESS) {
                temp = new_temp();
                newCodeList=concatenate(newCodeList,genInterCode(IC_READ_ADDR, temp, op2));
                op2 = temp;
            }
            newCodeList=concatenate(newCodeList,new_CodeList(new_InterCode(kind,result,op1,op2)));
            return newCodeList;
            break;
        case IC_IF_GOTO:
            va_start(valist, 4);
            result = va_arg(valist, Operand);
            relop = va_arg(valist, Operand);
            op1 = va_arg(valist, Operand);
            op2 = va_arg(valist, Operand);
            newCodeList =new_CodeList(new_InterCode(kind, result, relop, op1, op2));
            return newCodeList;
        case IC_DEC:
            va_start(valist, 2);
            op1 = va_arg(valist, Operand);
            size = va_arg(valist, int);
            newCodeList = new_CodeList(new_InterCode(kind, op1, size));
            return newCodeList;
            break;
        default:
            printf("deal with the genintercode with no type%d\n",kind);
            break;
    }
    return NULL;
}


void print_cur_list()
{
    if(code_head==NULL)
        printf("cur list is null\n");
    else
    {
        for(CodeList p=code_head;p!=code_tail;p=p->next)
        {
            printf("%d\n",p->code->kind);
        }
    }
}


void print_IR(FILE* fp, CodeList l)
{
    if(fp==NULL)
    {
        printf("error file\n");
        return ;
    }
    for (CodeList p = l; p!=NULL;p=p->next)
    {
        switch (p->code->kind)
        {
            case IC_FUNC:
                fprintf(fp,"FUNCTION ");
                fprintf(fp,"%s",p->code->u.func);
                fprintf(fp," :");
                break;
            case IC_RETURN:
                fprintf(fp,"RETURN ");
                printOp(fp,p->code->u.op);
                break;
            case IC_ASSIGN:
                printOp(fp,p->code->u.assign.left);
                fprintf(fp," := ");
                printOp(fp,p->code->u.assign.right);
                break;
            case IC_READ:
                fprintf(fp,"READ ");
                printOp(fp,p->code->u.op);
                break;
            case IC_WRITE:
                fprintf(fp,"WRITE ");
                printOp(fp,p->code->u.op);
                break;
            case IC_ARG:
                fprintf(fp,"ARG ");
                printOp(fp,p->code->u.op);
                break;
            case IC_GOTO:
                fprintf(fp, "GOTO ");
                printOp(fp, p->code->u.op);
                break;
            case IC_IF_GOTO:
                fprintf(fp, "IF ");
                printOp(fp, p->code->u.if_goto.x);
                fprintf(fp, " ");
                printOp(fp, p->code->u.if_goto.relop);
                fprintf(fp, " ");
                printOp(fp, p->code->u.if_goto.y);
                fprintf(fp, " GOTO ");
                printOp(fp, p->code->u.if_goto.z);
                break;
            case IC_LABEL:
                fprintf(fp, "LABEL ");
                printOp(fp, p->code->u.op);
                fprintf(fp, " :");
                break;
            case IC_SUB:
                printOp(fp, p->code->u.binop.result);
                fprintf(fp, " := ");
                printOp(fp, p->code->u.binop.op1);
                fprintf(fp, " - ");
                printOp(fp, p->code->u.binop.op2);
                break;
            case IC_PLUS:
                printOp(fp, p->code->u.binop.result);
                fprintf(fp, " := ");
                printOp(fp, p->code->u.binop.op1);
                fprintf(fp, " + ");
                printOp(fp, p->code->u.binop.op2);
                break;
            case IC_MUL:
                printOp(fp, p->code->u.binop.result);
                fprintf(fp, " := ");
                printOp(fp, p->code->u.binop.op1);
                fprintf(fp, " * ");
                printOp(fp, p->code->u.binop.op2);
                break;
            case IC_DIV:
                printOp(fp, p->code->u.binop.result);
                fprintf(fp, " := ");
                printOp(fp, p->code->u.binop.op1);
                fprintf(fp, " / ");
                printOp(fp, p->code->u.binop.op2);
                break;
            case IC_PARAM:
                fprintf(fp, "PARAM ");
                printOp(fp, p->code->u.op);
                break;
            case IC_CALL:
                printOp(fp, p->code->u.assign.left);
                fprintf(fp," := CALL ");
                printOp(fp, p->code->u.assign.right);
                break;
            case IC_DEC:
                fprintf(fp, "DEC ");
                printOp(fp, p->code->u.dec.op);
                fprintf(fp, " ");
                fprintf(fp, "%d", p->code->u.dec.size);
                break;
            case IC_GET_ADDR:
                printOp(fp, p->code->u.assign.left);
                fprintf(fp, " := &");
                printOp(fp, p->code->u.assign.right);
                break;
            case IC_READ_ADDR:
                printOp(fp, p->code->u.assign.left);
                fprintf(fp, " := *");
                printOp(fp, p->code->u.assign.right);
                break;
            case IC_WRITE_ADDR:
                fprintf(fp,"*");
                printOp(fp, p->code->u.assign.left);
                fprintf(fp, " := ");
                printOp(fp, p->code->u.assign.right);
                break;

            default:
                break;
        }
        fprintf(fp,"\n");
    }
    
    
}
void printOp(FILE* fp, Operand p)
{
    switch (p->kind)
    {
        
        case Em_CONSTANT:
            fprintf(fp, "#%d", p->u.val);
            break;
        case Em_VARIABLE:
        case Em_ADDRESS:
        case Em_LABEL:
        case Em_ARR:
        case Em_RELOP:
        case Em_TEMP:
        case Em_STRUCT:
        case Em_FUNC:
            fprintf(fp, "%s", p->u.name);
            break;
        default:
            break;
    }
}

void flag(char*t)
{
    //printf("%s\n",t);
}

void set_newOPerand(Operand op,int kind,void*val)
{
    op->kind = kind;
    switch (kind) {
        case Em_CONSTANT:
            op->u.val = (int)val;
            break;
        case Em_VARIABLE:
        case Em_ADDRESS:
        case Em_LABEL:
        case Em_FUNC:
        case Em_RELOP:
        case Em_ARR:
            if (op->u.name) free(op->u.name);
            op->u.name = (char*)val;
            break;
    }
}
CodeList translate_Args(Node* Args, ArgList* arg_list) {
    if(Args==NULL)
        return NULL;
    flag("Args");
    Operand t1 = new_temp();
    ArgList newArgList = malloc(sizeof(struct _ArgList));
    
    CodeList c1 = translate_Exp(Args->ch[0], t1);
    Operand p2=new_temp();
    
    if(find(t1->u.name)!=NULL && find(t1->u.name)->type->kind==ARRAY)
    {
        if(find(t1->u.name)->type->u.array.elem->kind==ARRAY)
        {
            Operand k=new_temp();
            c1=concatenate(c1,genInterCode(IC_GET_ADDR,k,p2));
            c1=concatenate(c1,genInterCode(IC_PLUS,k,last_array_base,last_array_offset));
            t1=k;
            // printf("*****output2****\n");
            // FILE *ff = fopen("output2", "a");
            // print_IR(ff,genInterCode(IC_PLUS,p2,last_array_base,last_array_offset));
            // fprintf(ff,"\n");
            // fclose(ff);
            // printf("finised\n");
        }
        else
        {
            if(find(t1->u.name)->isaddr==0)
            {
                Operand k=new_temp();
                c1=concatenate(c1,genInterCode(IC_GET_ADDR,k,t1));
                t1=k;
            }
        }  
    }
    newArgList->args = t1;
    newArgList->next = *arg_list;
    *arg_list = newArgList;


    if (Args->cnt==1) {
        return c1; // 只有一个参数直接返回
    } 
    else {
        CodeList c2 = translate_Args(Args->ch[2], arg_list);
        return concatenate(c1,c2);
    }
}

CodeList translate_Cond(Node* Exp, Operand label_true, Operand label_false) {
    if(Exp==NULL)
        return;
    if(strcmp(Exp->ch[0]->name, "LP") == 0)
    {
        return translate_Cond(Exp->ch[1], label_true, label_false);
    }
    if (strcmp(Exp->ch[0]->name, "NOT") == 0) { // 处理NOT逻辑运算符
        return translate_Cond(Exp->ch[1], label_false, label_true);
    } 
    else if(strcmp(Exp->ch[1]->name, "RELOP") == 0||strcmp(Exp->ch[1]->name, "AND") == 0||strcmp(Exp->ch[1]->name, "OR") == 0)
    {
        if (strcmp(Exp->ch[1]->name, "RELOP") == 0) 
        { // 处理关系运算符
            Operand t1=new_temp();
            Operand t2=new_temp();
            CodeList c1=translate_Exp(Exp->ch[0],t1);
            CodeList c2=translate_Exp(Exp->ch[2],t2);
            Operand relop=new_Operand(Em_RELOP,newstring(Exp->ch[1]->strval));

            if (t1->kind == Em_ADDRESS) {
                Operand temp = new_temp();
                c2=concatenate(c2,genInterCode(IC_READ_ADDR, temp, t1));
                t1 = temp;
            }
            if (t2->kind == Em_ADDRESS) {
                Operand temp = new_temp();
                c2=concatenate(c2,genInterCode(IC_READ_ADDR, temp, t2));
                t2 = temp;
            }
            CodeList c3=genInterCode(IC_IF_GOTO, t1, relop, t2, label_true);
            CodeList c4=genInterCode(IC_GOTO,label_false);
            CodeList c5=concatenate(c1,c2);
            CodeList c6=concatenate(c3,c4);
            CodeList c7=concatenate(c5,c6);
            return c7;
        } 
        else if (strcmp(Exp->ch[1]->name, "AND") == 0) 
        { // 处理逻辑与
            Operand label1 = new_label(); // 短路运算
            CodeList c1 = translate_Cond(Exp->ch[0], label1, label_false);
            CodeList c2 = translate_Cond(Exp->ch[2], label_true, label_false);
            CodeList clabel1 = genInterCode(IC_LABEL,label1);
            return concatenate(concatenate(c1,clabel1),c2);
        } 
        else if (strcmp(Exp->ch[1]->name, "OR") == 0) { // 逻辑或
            Operand label1 = new_label(); // 短路运算
            CodeList c1 = translate_Cond(Exp->ch[0], label_true, label1);
            CodeList c2 = translate_Cond(Exp->ch[2], label_true, label_false);
            CodeList clabel1 = genInterCode(IC_LABEL,label1);
            return concatenate(concatenate(c1,clabel1),c2);
        }
        // ...
    }
    else 
    { // 处理表达式
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Exp, t1);
        if (t1->kind == Em_ADDRESS) {
            Operand temp = new_temp();
            c1=concatenate(c1,genInterCode(IC_READ_ADDR, temp, t1));
            t1 = temp;
        }
        // 处理true分支
        Operand t2 = new_Operand(Em_CONSTANT, 0);
        Operand relop = new_Operand(Em_RELOP, newstring("!="));
        c1=concatenate(c1,genInterCode(IC_IF_GOTO, t1, relop, t2, label_true));
        c1=concatenate(c1,genInterCode(IC_GOTO, label_false));
        return c1;
    } 
}

int getSize(Type type) {
    if (type == NULL)
        return 0;
    else if (type->kind == BASIC)
        return 4;
    else if (type->kind == ARRAY)
        return type->u.array.size * getSize(type->u.array.elem);
    else if (type->kind == STRUCTURE) {
        int size = 0;
        FieldList temp = type->u.structure->domain;
        while (temp) {
            size += getSize(temp->type);
            temp = temp->tail;
        }
        return size;
    }
    return 0;
}