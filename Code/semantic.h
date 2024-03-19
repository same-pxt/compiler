#ifndef _SEMANTIC_H
#define _SEMANTIC_H
#include "hashtable.h"
#include "tree.h"
int Type_check(struct Type_ * t1,struct Type_ * t2);
int Param_check(struct FieldList_ * p1,struct FieldList_ * p2);
void ExtDefList(Node *node);
void ExtDef(Node* node);
void ExtDecList(Node *cur, Type type);
Type Specifier(Node* specifier);
Type TYPE(Node*type);
Type StructSpecifier(Node*node);
Function FunDec(Node *cur, Type type);
void CompSt(Node *cur, Type type);
FieldList VarDec(Node *cur, Type type,int is_structure);
FieldList VarList(Node* cur);
FieldList DefList(Node*DefList,int is_structure);
FieldList DecList(Node*declist,Type type,int is_structure);
void StmtList(Node*DefList,Type type);
void Stmt(Node *cur, Type type);
Type Exp(Node *cur);
FieldList Def(Node*def,int is_structure);
FieldList Dec(Node*dec,Type type,int is_structure);
FieldList Args(Node *cur);
FieldList ParamDec(Node *cur);



void printSemanticError(int errorType, int lineNum, char* msg);
#endif // _SEMANTIC_H