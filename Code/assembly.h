#ifndef _ASSEMBLY_H_
#define _ASSEMBLY_H_

#include "ir.h"
#define REG_NUM 32

struct _VarStructure{
    char*name;//变量名;
    int regNo;//变量存放的寄存器;
    int offset;//变量在内存中的存储位置;
    Operand op;
    struct _VarStructure* next;//变量链表;
};
typedef struct _VarStructure* VarStructure;

struct _Register{
    char*name;
    VarStructure var;//关联的变量
    int isFree;
};
typedef struct _Register* Register;

typedef struct _varList {
    VarStructure head;
    VarStructure cur;
};
typedef struct _varList* VarList;

typedef struct _varTable {
    VarList varListReg;  // 寄存器中的变量表
    VarList varListMem;  // 内存中的变量表
    int inFunc;
    char* curFuncName;
};
typedef struct _varTable* varTable;

typedef enum _regNo {
    ZERO,
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    K0,
    K1,
    GP,
    SP,
    FP,
    RA,
} RegNo;

void generate_asm(CodeList curcode,FILE* file);
void generate_IR_asm(CodeList intercode,FILE* file);
int getReg(FILE *file,Operand op);
int allocReg(Operand op,FILE*file);
VarList newVarlist();
varTable newVarTable();
void addVarible(VarList varList, int regNo, Operand op) ;
void delVarible(VarList varList, VarStructure var);

void pushstack(FILE*fp);
void popstack(FILE*fp);
#endif