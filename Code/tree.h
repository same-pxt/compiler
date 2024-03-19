// tree.h
#ifndef _TREE_H_
#define _TREE_H_
#include <string.h>
typedef struct NODE {
    char name[32]; // 语法单元标号
    int lineno; // 语法单元所在行号
    struct NODE *ch[16]; // 子节点
    int cnt; // 子节点数量
    /* 以下为词法单元值 */
    int intval;
    float floatval;
    char* strval;
} Node;
Node* createNode(char*name, int lineno);
Node* createLeaf(char*name,int lineno, char *text); // 终结符节点记录值属性
void printTree(Node *nd, int level); // 输出语法分析树
#endif