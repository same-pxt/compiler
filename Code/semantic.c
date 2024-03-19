#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
Type t;
int undeclared =0;
int is_in_func=0;
int Type_check(Type t1,Type t2){
    // printatype(t1);
    // printatype(t2);
    if (t1 == NULL && t2 == NULL)
        return 1; // 都为空一致
    if (t1 == NULL || t2 == NULL)
        return 0;
    if (t1->kind == BASIC)
    {
        if (t1->u.basic == t2->u.basic)
        {
            return 1; // 基本类型相同
        }
        return 0; // 基本类型不同
    }
    if (t1->kind == ARRAY) // 逐个比对数组元素类型
        return Type_check(t1->u.array.elem, t2->u.array.elem);
    if (t1->kind == STRUCTURE)
    {
        if (t2->kind != STRUCTURE)
        {
            return 0;
        }
            
        return t1->u.structure->name == t2->u.structure->name;
    }  
}

//check func's param
int Param_check(FieldList p1,FieldList p2){
    while (p1 != NULL && p2 != NULL) {
        if (Type_check(p1->type, p2->type) == 0)
            return 0; // 参数类型不一致
        p1 = p1->tail;
        p2 = p2->tail; // 获取下一个参数
    }
    if (p1 == NULL && p2 == NULL)
        return 1; // 检查结束，参数类型一致
    return 0; // 默认类型不一致
}

void ExtDefList(Node *node){
    if(node==NULL)
        return;
    //Extdef ExtdefList
    ExtDef(node->ch[0]);
    //for(int i=1;i<node->cnt;i++)
    if(node->cnt>0)
        ExtDefList(node->ch[1]);
}

void ExtDef(Node* node){
    if(node==NULL)
        return;
    //printf("ExtDef\n");
    // 获取node的specifier类型t;
    Type type=Specifier(node->ch[0]);
    if(type==NULL)
        return;
    // 获取node的其他节点信息sibc;
    //SEMI
    if(strcmp(node->ch[1],"SEMI")==0)
    {
        //for structure
        return;
    }
    //ExtDecList SEMI
    else if(strcmp(node->ch[1],"ExtDecList")==0)
    {
        ExtDecList(node->ch[1],type);
    }
    else if(strcmp(node->ch[1],"FunDec")==0)
    {
        //FunDec SEMI
        if(strcmp(node->ch[2],"SEMI")==0)
        {
            type->is_dec_func=1;
            is_in_func=1;
            FunDec(node->ch[1],type);
            is_in_func=0;
        }
        //FunDec CompSt
        else if(strcmp(node->ch[2],"CompSt")==0)
        {
            FunDec(node->ch[1],type);
            CompSt(node->ch[2],type);
        }
    }
}
void ExtDecList(Node *cur, Type type){
    if(cur == NULL) return;
    
    if(strcmp(cur->ch[0],"VarDec")==0)
    {
        //VarDec
        if(cur->cnt==1)
        {
            VarDec(cur->ch[0],type,0);
        }
        //VarDec COMMA ExtDecList
        else if(strcmp(cur->ch[1],"COMMA")==0)
        {
            VarDec(cur->ch[0],type,0);
            ExtDecList(cur->ch[2],type);
        }
    }
}
Function FunDec(Node *cur, Type type){
    if(cur==NULL) return;
    //printf("FunDec\n");
    if(strcmp(cur->ch[0]->name,"ID")==0)
    {   
        
        if(type->is_dec_func==1)
        {
            if(strcmp(cur->ch[2]->name,"RP")==0)
            {
                //ID LP RP
                int contain=0;
                if (check(cur->ch[0]->strval)==1) {
                    assert(0);
                    contain=1;
                }
                if(contain==0)
                {   
                    Type temp=(Type)malloc(sizeof(struct Type_));
                    temp->kind=FUNCTION;
                    temp->u.function=(Function)malloc(sizeof(struct Function_));
                    temp->u.function->type=type;
                    temp->u.function->param=NULL;
                    temp->u.function->name=cur->ch[0]->strval;
                    undeclared++;
                    insert(cur->ch[0]->strval,temp);
                    //printf("%s has been insert successfully,return value is%d,%d\n",cur->ch[0]->strval,type->kind,type->u.basic);
                    return temp->u.function;
                }
            }
            else if(strcmp(cur->ch[2],"VarList")==0)
            {
                //ID LP VarList RP
                Type temp3=(Type)malloc(sizeof(struct Type_));
                temp3->kind=FUNCTION;
                temp3->u.function=(Function)malloc(sizeof(struct Function_));
                temp3->u.function->line=0;
                temp3->u.function->param=VarList(cur->ch[2]);
                temp3->u.function->type=type;
                temp3->u.function->name=cur->ch[0]->strval;
                int contain=0;
                if (check(cur->ch[0]->strval)==1) {
                    HashNode item=find(cur->ch[0]->strval);
                    FieldList t=item->type->u.function->param;
                    if(Param_check(t,temp3->u.function->param)==0)
                    {
                        printSemanticError(19, cur->ch[0]->lineno, "Inconsistent declaration of function");
                        return NULL;
                    }
                    
                    contain=1;
                }
                if(contain==0)
                {
                    
                    insert(cur->ch[0]->strval,temp3);
                    undeclared++;
                    //printf("%s has been insert successfully,return value is%d\n",cur->ch[0]->strval,type->kind);
                }
                return temp3->u.function;
            }
        }
        else
        {
            if(strcmp(cur->ch[2]->name,"RP")==0)
            {
                //ID LP RP
                int contain=0;
                if (check(cur->ch[0]->strval)==1) {
                    HashNode temp=find(cur->ch[0]->strval);
                    if(temp->type->u.function->type->is_dec_func==1)
                    {
                        temp->type->u.function->type->is_dec_func==0;
                    }
                    else
                        printSemanticError(4, cur->ch[0]->lineno, "Redefined function");
                    contain=1;
                }
                if(contain==0)
                {   
                    Type temp=(Type)malloc(sizeof(struct Type_));
                    temp->kind=FUNCTION;
                    temp->u.function=(Function)malloc(sizeof(struct Function_));
                    temp->u.function->type=type;
                    temp->u.function->param=NULL;
                    insert(cur->ch[0]->strval,temp);
                    //printf("%s has been insert successfully,return value is%d,%d\n",cur->ch[0]->strval,type->kind,type->u.basic);
                    return temp->u.function;
                }
            }
            else if(strcmp(cur->ch[2],"VarList")==0)
            {
                //ID LP VarList RP
                Type temp3=(Type)malloc(sizeof(struct Type_));
                temp3->kind=FUNCTION;
                temp3->u.function=(Function)malloc(sizeof(struct Function_));
                temp3->u.function->line=0;
                temp3->u.function->param=VarList(cur->ch[2]);
                temp3->u.function->type=type;
                temp3->u.function->name=cur->ch[0]->strval;
                int contain=0;
                if (check(cur->ch[0]->strval)==1) {
                    HashNode temp=find(cur->ch[0]->strval);
                    if(temp->type->u.function->type->is_dec_func==1)
                    {
                        temp->type->u.function->type->is_dec_func==0;
                    }
                    else
                        printSemanticError(4, cur->ch[0]->lineno, "Redefined function");
                    contain=1;
                }
                if(contain==0)
                {
                    insert(cur->ch[0]->strval,temp3);
                    //printf("%s has been insert successfully,return value is%d\n",cur->ch[0]->strval,type->kind);
                }
                return temp3->u.function;
            }
        }
        
    }   
}
void CompSt(Node *cur, Type type){
    if(cur==NULL)
        return ;
    //printf("Compst\n");
    if(strcmp(cur->ch[0],"LC")==0)
    {
        //LC DefList StmtList RC
        DefList(cur->ch[1],0);
        StmtList(cur->ch[2],type);
    }
}
FieldList DefList(Node*Deflist,int is_structure){
    //Def DefList
    if(Deflist==NULL)
        return;
    //printf("DefList\n");
    if(is_structure==0)
    {
        Def(Deflist->ch[0],is_structure);
        DefList(Deflist->ch[1],is_structure);
        return NULL;
    }
    else if(is_structure==1)
    {
        FieldList field=Def(Deflist->ch[0],is_structure);
        if(field==NULL)
        {
            return DefList(Deflist->ch[1],is_structure);
        }
        else 
        {
            FieldList find_tail=field;
            for(;find_tail->tail!=NULL;find_tail=find_tail->tail);
            find_tail->tail=DefList(Deflist->ch[1],is_structure);
            return field;
        }
    }   
}
FieldList Def(Node*def,int is_structure)
{
    if(def==NULL)
    {
        return NULL;
    }
    //printf("Def\n");
    //Specifier DecList  SEMI
    if(strcmp(def->ch[0]->name,"Specifier")==0)
    {
        Type type2=Specifier(def->ch[0]);
        if(type2==NULL)
            return NULL;
        FieldList declist=DecList(def->ch[1],type2,is_structure);
        if(is_structure==0)
        {
            return NULL;
        }
        else
        {
            return declist;
        }
    }
}

FieldList DecList(Node*declist,Type type,int is_structure)
{
    if(declist==NULL)
        return;
    //printf("DecList\n");
    if(strcmp(declist->ch[0]->name,"Dec")==0)
    {
        if(declist->cnt==1)
        {
            return Dec(declist->ch[0],type,is_structure);
        }
        else if(declist->cnt==3)
        {
            FieldList f=Dec(declist->ch[0],type,is_structure);
            if(f==NULL)
            {
                return DecList(declist->ch[2],type,is_structure);
            }
            else
            {
                f->tail=DecList(declist->ch[2],type,is_structure);
                return f;
            }
        }
    }
}
FieldList Dec(Node*dec,Type type,int is_structure)
{
    if(dec==NULL)
        return;
    //printf("Dec\n");
    if(strcmp(dec->ch[0]->name,"VarDec")==0)
    {
        //VarDec
        if(dec->cnt==1)
        {
            return VarDec(dec->ch[0],type,is_structure);
        }
        //VarDec ASSIGNOP Exp
        else if(dec->cnt==3)
        {
            FieldList f=VarDec(dec->ch[0],type,is_structure);
            if(is_structure==0)
            {
                Type temp=Exp(dec->ch[2]);
                if (Type_check(type,temp)!=1) {
                    printSemanticError(5, dec->ch[2]->lineno, "Type mismatched for assignment");
                }
                return f;
            }
            else
            {
                printSemanticError(15, dec->ch[1]->lineno, "Initialize field at definition time");
                return f;
            }
            
        }
    }
}
void StmtList(Node*DefList,Type type){
    //Stmt StmtList
    if(DefList==NULL)
        return;
    //printf("StmtList\n");
    Stmt(DefList->ch[0],type);
    StmtList(DefList->ch[1],type);
}
void Stmt(Node *cur, Type type){
    if(cur==NULL)
        return;
    //printf("Stmt\n");
    //Exp SEMI
    if(strcmp(cur->ch[0]->name,"Exp")==0)
    {
        Exp(cur->ch[0]);
    }
    //CompSt
    else if(strcmp(cur->ch[0]->name,"CompSt")==0)
    {
        CompSt(cur->ch[0],type);
    }
    //RETURN Exp SEMI
    else if(strcmp(cur->ch[0]->name,"RETURN")==0)
    {
        Type temp=Exp(cur->ch[1]);
        if(Type_check(type,temp)!=1)
        {
             printSemanticError(8, cur->lineno, "Type mismatched for return");
        }
    }
    else if (strcmp(cur->ch[0]->name,"IF")==0)
    {
        //IF LP Exp RP Stmt
        if(cur->cnt==5)
        {
            Type temp=Exp(cur->ch[2]);
            if(temp==NULL) 
                return NULL;
            if(!(temp->kind==BASIC && temp->u.basic==1))
            {
                printSemanticError(8, cur->lineno, "Type mismatched for return");
            }
            Stmt(cur->ch[4],type);
        }
        //IF LP Exp RP Stmt ELSE Stmt
        else if(cur->cnt==7)
        {
            Type temp=Exp(cur->ch[2]);
            if(temp==NULL) return NULL;
            if(!(temp->kind==BASIC && temp->u.basic==1))
            {
                printSemanticError(8, cur->lineno, "Type mismatched for return");
            }
            Stmt(cur->ch[4],type);
            Stmt(cur->ch[6],type);
        }
    }
    else if(strcmp(cur->ch[0]->name,"WHILE")==0){
        //WHILE LP Exp RP Stmt
        Type temp=Exp(cur->ch[2]);
        if(temp==NULL)
            return NULL;
        if(!(temp->kind==BASIC && temp->u.basic==1))
        {
            printSemanticError(7, cur->lineno, "Type mismatched for \"while\"(int only)");
        }
        Stmt(cur->ch[4],type);
    }
}
Type Exp(Node *cur){
    if(cur==NULL)
        return NULL;
    if(strcmp(cur->ch[0]->name,"INT")==0)
    {
        //printf("Exp INT\n");
        Type temp=(Type)malloc(sizeof(struct Type_));
        temp->assign=RIGHT;
        temp->kind=BASIC;
        temp->u.basic=1;
        return temp;
    }
    else if(strcmp(cur->ch[0]->name,"FLOAT")==0)
    {
        //printf("Exp FLOAT\n");
        Type temp=(Type)malloc(sizeof(struct Type_));
        temp->assign=RIGHT;
        temp->kind=BASIC;
        temp->u.basic=2;
        return temp;
    }
    else if(strcmp(cur->ch[0]->name,"ID")==0)
    {
        if(cur->cnt==1)
        {
            //printf("EXP ID\n");
            if(check(cur->ch[0]->strval))
            {
                HashNode temp=find(cur->ch[0]->strval);
                temp->type->assign=LEFT;
                return temp->type;
            }
            else
            {
                printSemanticError(1, cur->ch[0]->lineno, "Undefined variable");
                return NULL;
            }
        }
        else if(cur->cnt==3)
        {
            //printf("exp ID LP RP\n");
            if(check(cur->ch[0]->strval))
            {
                HashNode temp=find(cur->ch[0]->strval);
                //printf("%s\n",cur->ch[0]->strval);
                if(temp->type->kind==FUNCTION)
                {
                    FieldList temp2=NULL;
                    if(Param_check(temp2,temp->type->u.function->param))
                    {
                        //printf("EXP ID LP RP pass\n");
                        return temp->type->u.function->type;
                    }
                    else 
                    {
                        //printf("EXP ID LP RP pass\n");
                        printSemanticError(9, cur->ch[0]->lineno, "Function is not applicable for arguments");
                        return NULL;
                    }
                }
                else if(temp->type->kind==BASIC || temp->type->kind==STRUCT)
                {
                    printSemanticError(11, cur->ch[0]->lineno, "This variable is not a function");
                    return NULL;
                }
            }
            else
            {
                printSemanticError(2, cur->ch[0]->lineno, "Undefined function");
                return NULL;
            }
        }
        else if(cur->cnt==4 && strcmp(cur->ch[2]->name,"Args")==0)
        {
            //printf("exp ID LP ARGS RP\n");
            if(check(cur->ch[0]->strval))
            {
                HashNode temp=find(cur->ch[0]->strval);
                //printf("%s\n",cur->ch[0]->strval);
                if(temp->type->kind==FUNCTION)
                {
                    FieldList temp2=Args(cur->ch[2]);
                    if(Param_check(temp2,temp->type->u.function->param))
                    {
                        return temp->type->u.function->type;
                    }
                    else 
                    {
                        printSemanticError(9, cur->ch[0]->lineno, "Function is not applicable for arguments");
                        return temp->type->u.function->type;;
                    }
                }
                else if(temp->type->kind==BASIC || temp->type->kind==STRUCT)
                {
                    printSemanticError(11, cur->ch[0]->lineno, "This variable is not a function");
                    return NULL;
                }
            }
            else
            {
                printSemanticError(2, cur->ch[0]->lineno, "Undefined function");
                return NULL;
            }
        }
    }
    else if(strcmp(cur->ch[0]->name,"Exp")==0&&strcmp(cur->ch[2]->name,"Exp")==0)
    {
        if(strcmp(cur->ch[1]->name,"ASSIGNOP")==0)
        {
            //printf("Exp ASSIGNOP Exp\n");
            Type left=Exp(cur->ch[0]);
            Type right=Exp(cur->ch[2]);
            if (left != NULL && left->assign==RIGHT) 
            {
                printSemanticError(6, cur->ch[1]->lineno, "The left-hand side of an assignment must be a variable");
                if(Type_check(left,right)==0)
                {
                    printSemanticError(5, cur->ch[1]->lineno, "Type mismatched for assignment");
                }
            }
            if(Type_check(left,right)==0)
            {
                printSemanticError(5, cur->ch[1]->lineno, "Type mismatched for assignment");
            }
            if (left != NULL)
                return left;
            else
                return right;
        }
        else if(strcmp(cur->ch[1]->name,"AND")==0 || strcmp(cur->ch[1]->name,"OR")==0 || strcmp(cur->ch[1]->name,"RELOP")==0)
        {
            //printf("Exp AND Exp\n");
            Type left=Exp(cur->ch[0]);
            Type right=Exp(cur->ch[2]);
            if (left==NULL)
            {
                return NULL;
            }
            if(Type_check(left,right)!=1)
            {
                printSemanticError(7, cur->ch[1]->lineno, "Type mismatched for oprand");
            }
            if(left->kind==BASIC && left->u.basic==1)
                return left;
            else 
                return NULL;
        }
        else if(strcmp(cur->ch[1]->name,"PLUS")==0 || strcmp(cur->ch[1]->name,"MINUS")==0 || strcmp(cur->ch[1]->name,"STAR")==0 || strcmp(cur->ch[1]->name,"DIV")==0)
        {
            //printf("Exp ADD Exp\n");
            Type left=Exp(cur->ch[0]);
            Type right=Exp(cur->ch[2]);
            if (left==NULL)
            {
                return NULL;
            }
            if(Type_check(right,left)==0)
            {
                printSemanticError(7, cur->ch[1]->lineno, "Type mismatched for oprand");
            }
            if (left != NULL)
                return left;
            else
                return right;
        }
        else if(strcmp(cur->ch[1]->name,"LB")==0)
        {
            //printf("EXP EXP LB EXP RB\n");
            Type vartype=Exp(cur->ch[0]);
            if(vartype==NULL)
                return;
            Type indextype=Exp(cur->ch[2]);
            if (vartype->kind != ARRAY) {
                printSemanticError(10, cur->ch[0]->lineno, "This variable is not an array");
                if (!(indextype->kind==BASIC && indextype->u.basic==1)) {
                    printSemanticError(12, cur->ch[2]->lineno, "Expression between \"[]\" is not an integer");
                }
                return NULL;
            }
            if (!(indextype->kind==BASIC && indextype->u.basic==1)) {
                printSemanticError(12, cur->ch[2]->lineno, "Expression between \"[]\" is not an integer");
            }
            vartype->u.array.elem->assign=LEFT;
            return vartype->u.array.elem;
        }
    }
    else if(strcmp(cur->ch[0]->name,"Exp")==0)
    {
        if(strcmp(cur->ch[1]->name,"DOT")==0)
        {
            //EXP DOT ID
            //printf("EXP DOT ID\n");
            Type varType = Exp(cur->ch[0]);
            if (varType == NULL) return NULL;
            if (varType->kind != STRUCTURE) {
                printSemanticError(13, cur->ch[0]->lineno, "Illegal use of \".\"");
                return NULL;
            }
            //printf("----------%s\n",varType->u.structure->name);
            if(check(varType->u.structure->name)==1)
            {
                HashNode t=find(varType->u.structure->name);
                for(FieldList i=t->type->u.structure->domain;i!=NULL;i=i->tail)
                {
                    //printf("here   %s    %s\n",i->name,cur->ch[2]->strval);
                    if(strcmp(i->name,cur->ch[2]->strval)==0)
                    {
                        i->type->assign=LEFT;
                        return i->type;
                    }
                }
                printSemanticError(14, cur->ch[0]->lineno, "Non-existent field");
                return NULL;
            }
        }
    }
    else if(strcmp(cur->ch[1]->name,"Exp")==0)
    {
        if(strcmp(cur->ch[0]->name,"NOT")==0)
        {
            Type left=Exp(cur->ch[1]);
            
            if (left==NULL)
            {
                return NULL;
            }
            if(!(left->kind==BASIC&&left->u.basic==1))
            {
                printSemanticError(7, cur->ch[1]->lineno, "Type mismatched for oprand");
            }
            return left;
        }
        else if(strcmp(cur->ch[0]->name,"MINUS")==0)
        {
            //printf("MINUS EXP\n");
            Type left=Exp(cur->ch[1]);
            if (left==NULL)
            {
                return NULL;
            }
            if(!(left->kind==BASIC))
            {
                printSemanticError(7, cur->ch[1]->lineno, "Type mismatched for oprand");
            }
            if (left != NULL)
                return left;
            return NULL;
        }
        else if(strcmp(cur->ch[0]->name,"LP")==0)
        {
            Type temp=Exp(cur->ch[1]);
            return temp;
        }
    }
    return NULL;
    
}
FieldList Args(Node *cur){
    if(cur==NULL)
    return;
    //printf("args ");
    if(strcmp(cur->ch[0]->name,"Exp")==0)
    { 
        if(cur->cnt==1)
        {
            //printf("EXP\n");
            //Exp
            Type tps=Exp(cur->ch[0]);
            FieldList temp=(FieldList)malloc(sizeof(struct FieldList_));
            temp->tail=NULL;
            temp->type=tps;
            //printf("%d,\n",tps->kind);
            return temp;
        }
        else
        {
            //printf("comma Args\n");
            //Exp COMMA Args
            Type tps=Exp(cur->ch[0]);
            FieldList temp=(FieldList)malloc(sizeof(struct FieldList_));
            temp->tail=Args(cur->ch[2]);
            temp->type=tps;
            return temp;
        }
    }
}
Type Specifier(Node*specifier){
    if(specifier==NULL)
        return NULL;
    //printf("Specifier\n");
    if(strcmp(specifier->ch[0]->name,"TYPE")==0)
    {   //TYPE
        return TYPE(specifier->ch[0]);
    }
    else if(strcmp(specifier->ch[0]->name,"StructSpecifier")==0)
    {   
        //StructSpecifier
        return StructSpecifier(specifier->ch[0]);
    }
}
Type TYPE(Node*type){
    if(type==NULL)
        return NULL;
    //printf("TYPE**\n");
    ////printf("%s\n",type->strval);
    if(strcmp(type->strval,"int")==0)
    {   
        Type newType = (Type)malloc(sizeof(struct Type_));
        newType->kind = BASIC;
        newType->u.basic = 1;
        newType->assign = RIGHT;
        return newType;
    }
    else if (strcmp(type->strval,"float")==0)
    {
        Type newType = (Type)malloc(sizeof(struct Type_));
        newType->kind = BASIC;
        newType->u.basic = 2;
        newType->assign = RIGHT;
        return newType;
    }
}
Type StructSpecifier(Node*node){
    if(node ==NULL)
        return NULL;
    //printf("StructSpecifier\n");
    if(strcmp(node->ch[0]->name,"STRUCT")==0)
    {
        if(node->cnt==2)
        {
            //STRUCT Tag
            char*tag=node->ch[1]->ch[0]->strval;
            if(tag==0)
            {
                //printf("STRUCT TAG is null\n");
                return NULL;
            }
            int contain=0;
            if(check(tag)==0)
            {
                printSemanticError(17, node->ch[1]->lineno, "nodefined structure");
                return NULL;
            }
            HashNode h=find(tag);
            return h->type;
        }
        else if(node->cnt==5)
        {
            //STRUCT OptTag LC DefList RC
            if(node->ch[1]==NULL)
            {
                //printf("OptTag is NULL\n");
                Type temp=(Type)malloc(sizeof(struct Type_));
                temp->kind=STRUCTURE;
                temp->u.structure=(Structure)malloc(sizeof(struct Structure_));
                temp->u.structure->name="noname";
                temp->u.structure->domain=DefList(node->ch[3],1);
                insert("noname",temp);
                //printf("%s has insert success,type is %d\n","noname",temp->kind);
                return temp;
            }
            
            if(strcmp(node->ch[1]->name,"OptTag")==0)
            {
                char * tagname=node->ch[1]->ch[0]->strval;
                //printf("**********%s\n",tagname);
                int contain=0;
                if(check(tagname)==1)
                {
                    printSemanticError(16, node->ch[1]->lineno, "Duplicated name");
                }
                Type temp=(Type)malloc(sizeof(struct Type_));
                temp->kind=STRUCTURE;
                temp->u.structure=(Structure)malloc(sizeof(struct Structure_));
                temp->u.structure->name=tagname==NULL ? "nonename" : tagname;
                temp->u.structure->domain=DefList(node->ch[3],1);
                if(contain==0)
                {
                    insert(tagname,temp);
                    //printf("%s has insert success,type is %d\n",tagname,temp->kind);
                }
                return temp;
            }
            //printf("here\n");
            // for(FieldList T=temp->u.structure->domain;T!=NULL;T=T->tail)
            // {
            //     //printf("%s   ",T->name);
            // }
            // //printf("\n***********\n");
            
        }
        else if(node->cnt==4)
        {
            
            //STRUCT LC DefList RC
            Type temp=(Type)malloc(sizeof(struct Type_));
            temp->kind=STRUCTURE;
            temp->u.structure=(Structure)malloc(sizeof(struct Structure_));
            temp->u.structure->name="noname";
            temp->u.structure->domain=DefList(node->ch[3],1);
            // for(FieldList T=temp->u.structure->domain;T!=NULL;T=T->tail)
            // {
            //     //printf("%s   ",T->name);
            // }
            // //printf("\n***********\n");
            insert("noname",temp);
            //printf("%s has insert success,type is %d\n","noname",temp->kind);
            return temp;
        }
        else 
        {
            //printf("%d\n",node->cnt);
        }
    }
    return  NULL;
}
FieldList VarList(Node* cur)
{
    if(cur ==NULL)
        return NULL;
    //printf("VarList\n");
    if(strcmp(cur->ch[0]->name,"ParamDec")==0)
    {
        FieldList para=ParamDec(cur->ch[0]);
        if(cur->cnt==1)
        {
            //ParamDec
            return para;
        }
        else if(cur->cnt==3)
        {
            //ParamDec COMMA VarList
            FieldList t=VarList(cur->ch[2]);
            if(para!=NULL)
                para->tail=t;
            else
                para=t;
            return para;
        }
    }

}
FieldList ParamDec(Node *cur){
    if(cur ==NULL)
        return NULL;
    //printf("ParamDec\n");
    //Specifier VarDec
    if(strcmp(cur->ch[0]->name,"Specifier")==0)
    {
        Type type=Specifier(cur->ch[0]);
        return VarDec(cur->ch[1],type,0);
    }
}
FieldList VarDec(Node *cur, Type type,int is_structure){
    if(cur ==NULL)
        return NULL;
    //printf("VarDec\n");
    if(strcmp(cur->ch[0],"ID")==0)
    {
        //ID
        int contain=0;
        if (check(cur->ch[0]->strval)==1) {
            HashNode temp=find(cur->ch[0]->strval);
            if(is_structure==0)
            {
                if(temp->type->is_dec_func==0)
                {
                    printSemanticError(3, cur->ch[0]->lineno, "Redefined variable");
                    return NULL;
                }   
            }
            else if(is_structure==1)
            {
                printSemanticError(15, cur->ch[0]->lineno, "Redefined field");
                return NULL;
            }
            contain=1;
        }
        if(contain==0)
        {   
            FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
            f->name=cur->ch[0]->strval;
            f->tail=NULL;
            f->type=type;
            if(is_in_func==1)
                type->is_dec_func=1;
            insert(cur->ch[0]->strval,type);
            //printf("%s has been insert successfully,type is %d\n",cur->ch[0]->strval,type->kind);
            return f;
        }
    }
    else if(strcmp(cur->ch[0],"VarDec")==0)
    {
        //VarDec LB INT RB
        Type array = (Type)malloc(sizeof(struct Type_));
        array->kind = ARRAY;
        array->u.array.size = cur->ch[2]->intval;
        array->u.array.elem = type;
        FieldList t1=VarDec(cur->ch[0],array,is_structure);
        return t1;
    }
}
void printSemanticError(int errorType, int lineNum, char* msg) {
    //printf("Error type %d at Line %d: %s.\n", errorType, lineNum, msg);
}

void printatype(Type type)
{
    if(type==NULL)
        //printf("NULL\n");
    if(type->kind==BASIC)
    {
        //printf("BASIC %d\n",type->u.basic);
    }
    if(type->kind==ARRAY)
    {
        //printf("array %d\n",type->u.array.size);
    }
    if(type->kind==STRUCTURE)
    {
        //printf("structure %s\n",type->u.structure->name);
    }
    if(type->kind==FUNCTION)
    {
        //printf("func %s\n",type->u.function->name);
    }
}