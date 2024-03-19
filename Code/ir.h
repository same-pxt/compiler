#if !defined(_IR_H_)
#define _IR_H_
#include "hashtable.h"
#include "tree.h"
#include <stdio.h>
typedef struct _Operand* Operand;
typedef struct _InterCode* InterCode;
typedef struct _CodeList* CodeList;
typedef struct _ArgList* ArgList;
typedef struct _Variable* Variable;

struct _Operand{
    enum{
        Em_VARIABLE, // 变量（var）
        Em_CONSTANT, // 常量（#1）
        Em_ADDRESS, // 地址（&var）
        Em_LABEL, // 标签(LABEL label1:)
        Em_ARR, // 数组（arr[2]）
        Em_STRUCT, // 结构体（struct Point p）
        Em_TEMP, // 临时变量（t1）
        Em_RELOP,
        Em_FUNC,
    } kind;
    union{ 
        char*name;
        int varno; // 变量定义的序号
        int labelno; // 标签序号
        int val; // 操作数的值
        int tempno; // 临时变量序号（唯一性）
    } u;
    Type type; // 计算数组、结构体占用size
    int para; // 标识函数参数
};

struct _InterCode {
    enum {
        IC_ASSIGN,
        IC_LABEL,
        IC_PLUS,
        IC_SUB,
        IC_MUL,
        IC_DIV,
        IC_DEC,
        IC_FUNC,
        IC_CALL,
        IC_PARAM,
        IC_READ,
        IC_WRITE,
        IC_RETURN,
        IC_ARG,
        IC_GOTO,
        IC_IF_GOTO,
        IC_GET_ADDR,
        IC_READ_ADDR,
        IC_WRITE_ADDR,
        // ...
    } kind;

    union {
        Operand op;
        char* func;
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop; // 三地址代码
        struct { Operand x, y, z,relop} if_goto;
        struct { Operand result; char* func; } call;
        struct { Operand op; int size;} dec;
    } u;
};

struct _CodeList {
    InterCode code; // 中间代码列表实现方式
    CodeList prev, next;
};

struct _ArgList { // 参数列表实现方式
    Operand args;
    ArgList next;
};

struct _Variable { // 变量的实现方式
    char* name;
    Operand op;
    Variable next;
};
CodeList concatenate(CodeList c1,CodeList c2);
CodeList Intercode(Node* Root);

InterCode new_InterCode(int kind,...);
CodeList genInterCode(int kind,...);
CodeList new_CodeList(InterCode code);
Operand new_Operand(int kind,...);
void set_newOPerand(Operand op,int kind,void*val);
Operand new_temp();
Operand new_label();
char* newstring(const char* source);

CodeList translate_ExtDef(Node* ExtDef);
CodeList translate_ExtDefList(Node* ExtDefList);
CodeList translate_FunDec(Node* FunDec);
CodeList translate_CompSt(Node* CompSt);
CodeList translate_Stmt(Node* Stmt);
CodeList translate_StmtList(Node* StmtList);
CodeList translate_DefList(Node* DefList);
CodeList translate_Def(Node* Def);
CodeList translate_DecList(Node* DecList);
CodeList translate_Dec(Node* Dec);
CodeList translate_VarDec(Node* VarDec,Operand place);
CodeList translate_Exp(Node* Exp, Operand place);
CodeList translate_Args(Node* Args, ArgList* arg_list);
CodeList translate_Cond(Node *Exp, Operand label_true, Operand label_false);
void print_IR(FILE* fp, CodeList l);
void printOp(FILE* fp, Operand p);
int getSize(Type type);
#endif // _IR_H_
