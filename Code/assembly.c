#include "assembly.h"

const char* REG_NAME[REG_NUM] = {
    "$0",  "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
    "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5",
    "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

Register r[REG_NUM];
varTable table;
int lastChangedReg=0;
int curMemOffset=-28;
void generate_asm(CodeList curcode,FILE* file)
{
    table=newVarTable();
    // 打印目标代码数据段
    fprintf(file,".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\n");
    //deal with DEC
    CodeList temp = curcode;
    while (temp) {
        if (temp->code->kind == IC_DEC)
            fprintf(file, "%s: .word %d\n", temp->code->u.dec.op->u.name,temp->code->u.dec.size);
        temp = temp->next;
    }
    // 打印read函数定义
    fprintf(file,"\nread:\n");
    fprintf(file,"\tli $v0, 4\n");
    fprintf(file,"\tla $a0, _prompt\n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tli $v0, 5\n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tjr $ra\n");

    // 打印write函数定义
    fprintf(file,"\nwrite:\n");
    fprintf(file,"\tli $v0, 1\n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tli $v0, 4\n");
    fprintf(file,"\tla $a0, _ret\n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tmove $v0, $0\n");
    fprintf(file,"\tjr $ra\n\n");

    //initialize register
    for(int i = 0; i<32; i++)
    {
        Register x = (Register)malloc(sizeof(struct _Register));
        x->name=REG_NAME[i];
        x->isFree=1;
        r[i] = x; 
    }
    r[0]->isFree=0;

    while(curcode != NULL){
        generate_IR_asm(curcode,file); // 遍历每一条IR，生成目标代码
        //获取下一条IR;
        curcode=curcode->next;
    }
}
varTable newVarTable() {
    varTable p = (varTable)malloc(sizeof(struct _varTable));
    p->varListReg = newVarlist();
    p->varListMem = newVarlist();
    p->inFunc = 0;
    p->curFuncName = NULL;
    return p;
}
void generate_IR_asm(CodeList codelist,FILE* file){
    InterCode intercode=codelist->code;
    switch (intercode->kind)
    {
        case IC_FUNC:
        {
            fprintf(file, "\n%s:\n", intercode->u.func);
            for (int i = 0; i < REG_NUM; i++)
            {
                r[i]->isFree=1;
            }
            clearVarList(table->varListReg);
            clearVarList(table->varListMem);
            if (!strcmp(intercode->u.func, "main")) {
                table->inFunc = 0;
                table->curFuncName = NULL;
            } 
            else
            {
                table->inFunc = 1;
                table->curFuncName = intercode->u.func;

                HashNode item=find(intercode->u.func);
                int argc=0;
                FieldList temp = item->type->u.function->param;
                FieldList temp2 = item->type->u.function->param;
                int num=0;
                while (temp2)
                {
                    num++;
                    temp2=temp->tail;
                }
                
                while (temp)
                {
                    printf("%s +++\n",temp->name);
                    if(argc<4)
                    {
                        addVarible(table->varListReg, A0 + argc,new_Operand(Em_VARIABLE,temp->name));
                    }
                    else
                    {
                        int regNo = getReg(file,new_Operand(Em_VARIABLE,temp->name));
                        int num=0;
                        fprintf(file, "\tlw %s, %d($fp)\n",r[regNo]->name,( num - 1 - argc) * 4);
                    }
                    argc++;
                    temp=temp->tail;
                }
                
            }
            break;
        }
            
        case IC_ASSIGN:
        {
            Operand right=intercode->u.assign.right;
            Operand left=intercode->u.assign.left;
            if(right->kind == Em_CONSTANT){ // x := #k
                int x = getReg(file,left); // 获取寄存器并保存常量
                fprintf(file, "\tli %s, %d\n", r[x]->name, right->u.val);
            }
            else
            {
                int x1=getReg(file,left);
                int x2=getReg(file,right);
                fprintf(file, "\tmove %s, %s\n", r[x1]->name,r[x2]->name);
            }
            break;
        }
        case IC_READ:
        {
            fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $ra, 0($sp)\n");
            fprintf(file, "\tjal read\n");
            fprintf(file, "\tlw $ra, 0($sp)\n");
            fprintf(file, "\taddi $sp, $sp, 4\n");
            int RegNo =getReg(file,intercode->u.op);
            fprintf(file, "\tmove %s, $v0\n", r[RegNo]->name);
            break;
        }
        case IC_IF_GOTO:
        {
            char* relopName = intercode->u.if_goto.relop->u.name;
            int x=getReg(file,intercode->u.if_goto.x);
            int y=getReg(file,intercode->u.if_goto.y);
            if(strcmp(relopName,"==")==0)
            {
                fprintf(file, "\tbeq %s, %s, %s\n", r[x]->name,r[y]->name,intercode->u.if_goto.z->u.name);
            }
            else if(strcmp(relopName,"!=")==0)
            {
                fprintf(file, "\tbne %s, %s, %s\n", r[x]->name,r[y]->name,intercode->u.if_goto.z->u.name);
            }
            else if(strcmp(relopName,">")==0)
            {
                fprintf(file, "\tbgt %s, %s, %s\n", r[x]->name,r[y]->name,intercode->u.if_goto.z->u.name);
            }
            else if(strcmp(relopName,"<")==0)
            {
                fprintf(file, "\tblt %s, %s, %s\n", r[x]->name,r[y]->name,intercode->u.if_goto.z->u.name);
            }
            else if(strcmp(relopName,">=")==0)
            {
                fprintf(file, "\tbge %s, %s, %s\n", r[x]->name,r[y]->name,intercode->u.if_goto.z->u.name);
            }
            else if(strcmp(relopName,"<=")==0)
            {
                fprintf(file, "\tble %s, %s, %s\n", r[x]->name,r[y]->name,intercode->u.if_goto.z->u.name);
            }
            break;
        }
        case IC_GOTO:
        {
            fprintf(file, "\tj %s\n", intercode->u.op->u.name);
            break;
        }
        case IC_RETURN:
        {
            int RegNo =getReg(file,intercode->u.op);
            fprintf(file, "\tmove $v0, %s\n", r[RegNo]->name);
            fprintf(file, "\tjr $ra\n");
            break;
        }
        case IC_LABEL:
            fprintf(file, "%s:\n", intercode->u.op->u.name);
            break;
        case IC_WRITE:
        {
            int RegNo = getReg(file, intercode->u.op);
            if (table->inFunc == 0) {
                fprintf(file, "\tmove $a0, %s\n", r[RegNo]->name);
                fprintf(file, "\taddi $sp, $sp, -4\n");
                fprintf(file, "\tsw $ra, 0($sp)\n");
                fprintf(file, "\tjal write\n");
                fprintf(file, "\tlw $ra, 0($sp)\n");
                fprintf(file, "\taddi $sp, $sp, 4\n");
            } else {
                // 函数嵌套调用，先将a0压栈 调用结束以后需要恢复a0
                fprintf(file, "\taddi $sp, $sp, -8\n");
                fprintf(file, "\tsw $a0, 0($sp)\n");
                fprintf(file, "\tsw $ra, 4($sp)\n");
                fprintf(file, "\tmove $a0, %s\n", r[RegNo]->name);
                fprintf(file, "\tjal write\n");
                fprintf(file, "\tlw $a0, 0($sp)\n");
                fprintf(file, "\tlw $ra, 4($sp)\n");
                fprintf(file, "\taddi $sp, $sp, 8\n");
            }
            break;
        }
        case IC_PLUS:
            {
                int resultReg=getReg(file,intercode->u.binop.result);
                if (intercode->u.binop.op1->kind == Em_CONSTANT &&
                    intercode->u.binop.op2->kind == Em_CONSTANT) 
                {
                    fprintf(file, "\tli %s, %d\n", r[resultReg]->name,
                            intercode->u.binop.op1->u.val +
                                intercode->u.binop.op1->u.val);
                }
                else if(intercode->u.binop.op1->kind != Em_CONSTANT &&
                    intercode->u.binop.op2->kind == Em_CONSTANT) 
                {
                    int op1Reg=getReg(file,intercode->u.binop.op1);
                    fprintf(file, "\taddi %s, %s, %d\n",r[resultReg]->name,r[op1Reg]->name,intercode->u.binop.op2->u.val);
                }
                else
                {
                    int op1Reg=getReg(file,intercode->u.binop.op1);
                    int op2Reg=getReg(file,intercode->u.binop.op2);
                    fprintf(file, "\tadd %s, %s, %s\n",r[resultReg]->name,r[op1Reg]->name,r[op2Reg]->name);
                }
                break;
            }
        case IC_SUB:
            {
                int resultReg=getReg(file,intercode->u.binop.result);
                if (intercode->u.binop.op1->kind == Em_CONSTANT &&
                    intercode->u.binop.op2->kind == Em_CONSTANT) 
                {
                    fprintf(file, "\tli %s, %d\n", r[resultReg]->name,
                            intercode->u.binop.op1->u.val -
                                intercode->u.binop.op1->u.val);
                }
                else if(intercode->u.binop.op1->kind != Em_CONSTANT &&
                    intercode->u.binop.op2->kind == Em_CONSTANT) 
                {
                    int op1Reg=getReg(file,intercode->u.binop.op1);
                    fprintf(file, "\taddi %s, %s, %d\n",r[resultReg]->name,r[op1Reg]->name,-intercode->u.binop.op2->u.val);
                }
                else
                {
                    int op1Reg=getReg(file,intercode->u.binop.op1);
                    int op2Reg=getReg(file,intercode->u.binop.op2);
                    fprintf(file, "\tsub %s, %s, %s\n",r[resultReg]->name,r[op1Reg]->name,r[op2Reg]->name);
                }
                break;
            }
        case IC_MUL:
        {
            int resultReg=getReg(file,intercode->u.binop.result);
            int op1Reg=getReg(file,intercode->u.binop.op1);
            int op2Reg=getReg(file,intercode->u.binop.op2);
            fprintf(file, "\tmul %s, %s, %s\n",r[resultReg]->name,r[op1Reg]->name,r[op2Reg]->name);
            break;
        }
        case IC_DIV:
        {
            int resultReg=getReg(file,intercode->u.binop.result);
            int op1Reg=getReg(file,intercode->u.binop.op1);
            int op2Reg=getReg(file,intercode->u.binop.op2);
            fprintf(file, "  div %s, %s\n", r[op1Reg]->name,r[op2Reg]->name);
            fprintf(file, "  mflo %s\n", r[resultReg]->name);
            break;
        }
        case IC_ARG:
        {
            break;
        }
        case IC_PARAM:
        {
            break;
        }
        case IC_CALL:
        {
            HashNode Item=find(intercode->u.assign.right->u.name);
            int leftNo=getReg(file,intercode->u.assign.left);
            
            // 函数调用前的准备
            fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $ra, 0($sp)\n");
            pushstack(file);
            FieldList temp=Item->type->u.function->param;
            int num=0;
            while (temp)
            {
                temp=temp->tail;
                num++;
            }
            
            // 如果是函数嵌套调用，把前形参存到内存，腾出a0-a3寄存器给新调用使用
            if (table->inFunc) {
                fprintf(file, "\taddi $sp, $sp, -%d\n",num * 4);
                HashNode curFunc = find(table->curFuncName);
                temp=curFunc->type->u.function->param;
                int num2=0;
                while (temp)
                {
                    temp=temp->tail;
                    num2++;
                }
                for (int i = 0; i < num2; i++) {
                    if (i > num) 
                        break;
                    if (i < 4) {
                        fprintf(file, "\tsw %s, %d($sp)\n",r[A0 + i]->name, i * 4);
                        VarStructure var = table->varListReg->head;
                        while (var && var->regNo != A0 + i) {
                            var = var->next;
                        }
                        delVarible(table->varListReg, var);
                        addVarible(table->varListMem, -1, var->op);
                        int regNo = getReg(file,var->op);
                        fprintf(file, "\tmove %s, %s\n",r[regNo]->name,r[A0 + i]->name);
                    }
                }
            }
            // 处理实参 IR_ARG
            CodeList arg =codelist->prev;
            int argc = 0;
            while (arg && argc < num) {
                if (arg->code->kind == IC_ARG) {
                    int argRegNo = getReg(file,arg->code->u.op);
                    // 前4个参数直接用寄存器存
                    if (argc < 4) {
                        fprintf(file, "\tmove %s, %s\n",r[A0 + argc]->name,r[argRegNo]->name);
                        argc++;
                    }
                    // 4个以后的参数压栈
                    else {
                        fprintf(file, "\taddi $sp, $sp, -4\n");
                        fprintf(file, "\tsw %s, 0($sp)\n",r[argRegNo]->name);
                        fprintf(file, "\tmove $fp, $sp\n");
                        argc++;
                    }
                }
                arg = arg->prev;
            }

            fprintf(file, "\tjal %s\n", intercode->u.assign.right->u.name);

            // 调用完后恢复栈指针、形参，然后恢复之前保存入栈的寄存器信息
            if (argc > 4) fprintf(file, "\taddi $sp, $sp, %d\n", 4 * argc);
            if (table->inFunc) {
                HashNode curFunc = find(table->curFuncName);
                temp=curFunc->type->u.function->param;
                int num2=0;
                while (temp)
                {
                    temp=temp->tail;
                    num2++;
                }
                for (int i = 0; i < num2; i++) {
                    if (i > num) break;
                    if (i < 4) {
                        fprintf(file, "\tlw %s, %d($sp)\n",r[A0 + i]->name, i * 4);
                        VarStructure var = table->varListReg->head;
                        while (var) {
                            if (var->op->kind != Em_CONSTANT &&!strcmp(table->varListMem->head->op->u.name,var->op->u.name))
                                break;
                            var = var->next;
                        }
                        if (var) {
                            r[var->regNo]->isFree = 1;
                            var->regNo = A0 + i;
                        } else {
                            addVarible(table->varListReg, A0 + i,table->varListMem->head->op);
                        }
                        delVarible(table->varListMem,table->varListMem->head);
                    }
                }
                fprintf(file, "\taddi $sp, $sp, %d\n",num * 4);
            }
            popstack(file);
            fprintf(file, "\tlw $ra, 0($sp)\n");
            fprintf(file, "\taddi $sp, $sp, 4\n");
            fprintf(file, "\tmove %s, $v0\n", r[leftNo]->name);
            break;
        }
        case IC_GET_ADDR:
        {
            int leftRegNo =getReg(file,intercode->u.assign.left);
            fprintf(file, "  la %s, %s\n", r[leftRegNo]->name,intercode->u.assign.right->u.name);
            break;
        }
        case IC_READ_ADDR:
        {
            int leftRegNo =getReg(file,intercode->u.assign.left);
            int rightRegNo =getReg(file,intercode->u.assign.right);
            fprintf(file, "  lw %s, 0(%s)\n", r[leftRegNo]->name,r[rightRegNo]->name);
            break;
        }
        case IC_WRITE_ADDR:
        {
            int leftRegNo =getReg(file,intercode->u.assign.left);
            int rightRegNo =getReg(file,intercode->u.assign.right);
            fprintf(file, "  sw %s, 0(%s)\n", r[leftRegNo]->name,r[rightRegNo]->name);
            break;
        }
        default:
            break;
    }
}
int getReg(FILE *file,Operand op)
{
    if(op->kind==Em_CONSTANT)
    {
        if(op->u.val==0)
            return ZERO;
        int regNo=allocReg(op,file);
        fprintf(file, "\tli %s, %d\n", r[regNo]->name,op->u.val);
        return regNo;
    }
    else
    {
        VarStructure temp=table->varListReg->head;
        printf("\ncheck Variable\n");
        while (temp)
        {
            printf("%s ",temp->op->u.name);
            if(temp->op->kind!=Em_CONSTANT && strcmp(temp->op->u.name, op->u.name)==0)
            {
                if(temp->offset==0)
                    return temp->regNo;
            }
            temp=temp->next;
        }
        fprintf("%s is not in the res",op->u.name);
        int regNo=allocReg(op,file);
        // VarStructure temp2=table->varListMem->head;
        // while (temp2)
        // {
        //     if(temp2->op->kind!=Em_CONSTANT && strcmp(temp2->op->u.name, op->u.name)==0)
        //     {
        //         fprintf(file, "  lw %s, %d($sp)\n", r[regNo]->name, temp2->offset);
        //         printf("*************%s was load to reg %d",op->u.name,regNo);
        //         return regNo;
        //     }
        //     temp2=temp2->next;
        // }
        return regNo;
    }
}


int allocReg(Operand op,FILE*file)
{
    for (int i = T0; i <= T9; i++)
    {
        if(r[i]->isFree==1)
        {
            r[i]->isFree=0;
            addVarible(table->varListReg, i, op);
            return i;
        }
    }
    printf("all the register is full\n");
    
    VarStructure temp=table->varListReg->head;
    while (temp)
    {
         if (temp->op->kind == Em_CONSTANT && temp->regNo != lastChangedReg) {
            int regNo = temp->regNo;
            lastChangedReg= regNo;
            delVarible(table->varListReg, temp);
            addVarible(table->varListReg, regNo, op);
            addVaribleMem(table->varListMem,curMemOffset,temp->op);
            fprintf(file, "  sw %s, %d($sp)\n", r[regNo]->name, curMemOffset);
            curMemOffset-=4;
            return regNo;
        }
        temp = temp->next;
    }
    temp=table->varListReg->head;
    while (temp)
    {
         if (temp->op->kind != Em_CONSTANT) {
            if (temp->op->u.name[0] == 't' &&
                temp->regNo != lastChangedReg) {
                int regNo = temp->regNo;
                lastChangedReg= regNo;
                delVarible(table->varListReg, temp);
                addVarible(table->varListReg, regNo, op);
                addVaribleMem(table->varListMem,curMemOffset,temp->op);
                fprintf(file, "  sw %s, %d($sp)\n", r[regNo]->name, curMemOffset);
                curMemOffset-=4;
                return regNo;
            }
        }
        temp = temp->next;
    }
}
void addVarible(VarList varList, int regNo, Operand op) {
    VarStructure p = (VarStructure)malloc(sizeof(struct _VarStructure));
    p->regNo = regNo;
    p->op = op;
    p->next = NULL;
    if (varList->head == NULL) {
        varList->head = p;
        varList->cur = p;
    } else {
        varList->cur->next = p;
        varList->cur = p;
    }
}
void addVaribleMem(VarList varList, int offset, Operand op) {
    VarStructure p = (VarStructure)malloc(sizeof(struct _VarStructure));
    p->offset = offset;
    p->op = op;
    p->next = NULL;
    if (varList->head == NULL) {
        varList->head = p;
        varList->cur = p;
    } else {
        varList->cur->next = p;
        varList->cur = p;
    }
}
void delVarible(VarList varList, VarStructure var) {
    if (var == varList->head) {
        varList->head = varList->head->next;
    } else {
        VarStructure temp = varList->head;
        while (temp) {
            if (temp->next == var) break;
            temp = temp->next;
        }
        if (varList->cur == var) varList->cur = temp;
        temp->next = var->next;
        var->next = NULL;
        free(var);
    }
}

VarList newVarlist()
{
    VarList p = (VarList)malloc(sizeof(struct _varList));
    p->head = NULL;
    p->cur = NULL;
    return p;
}
void clearVarList(VarList varList) {
    VarStructure temp = varList->head;
    while (temp) {
        VarStructure p = temp;
        temp = temp->next;
        free(p);
    }
    varList->head = NULL;
    varList->cur = NULL;
}
void f()
{
    printf("here\n");
}
void pushstack(FILE *fp)
{
    fprintf(fp, "\taddi $sp, $sp, -72\n");
    for (int i = T0; i <= T9; i++) {
        fprintf(fp, "\tsw %s, %d($sp)\n", r[i]->name,(i - T0) * 4);
    }
}

void popstack(FILE *fp)
{
    for (int i = T0; i <= T9; i++) {
        fprintf(fp, "\tlw %s, %d($sp)\n", r[i]->name,(i - T0) * 4);
    }
    fprintf(fp, "\taddi $sp, $sp, 72\n");
}