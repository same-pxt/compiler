// tree.c
#include "tree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
Node* createLeaf(char*name, int lineno, char *text) {
    Node* nd = malloc(sizeof(Node)); 
    strcpy(nd->name, name);
    nd->lineno = lineno;
    if(strcmp("TYPE",name)==0)
    {
        nd->strval = malloc(sizeof(char) * strlen(text));
        strcpy(nd->strval, text);
    }
    else if(strcmp(nd->name,"ID")==0)
    {
        nd->strval = malloc(sizeof(char) * strlen(text));
        strcpy(nd->strval, text);
    }
    else if(strcmp(nd->name,"INT")==0)
    {
        nd->intval = atoi(text); 
    }
    else if(strcmp(nd->name,"FLOAT")==0)
    {
        nd->floatval = atof(text);
    }
    else if(strcmp(nd->name,"RELOP")==0)
    {
        nd->strval = malloc(sizeof(char) * strlen(text));
        strcpy(nd->strval, text);
    }
    return nd;
}

Node* createNode(char*name, int lineno) {
    //printf("creatNode\n");
    Node *nd = malloc(sizeof(Node));
    strcpy(nd->name, name);
    nd->lineno = lineno;
    return nd;
    //printf("T am here*********************************************\n");
}

void insertChild(Node *pa, Node *ch) {
    //printf("insert\n");
    pa->ch[pa->cnt++] = ch;
    //printTree(pa,1);
}

void printTree(Node *nd, int level) {
    int i;
    char indent[128] = {0}; 
    for(i = 0; i < level; i++) indent[i] = ' '; indent[i] = '\0';
    printf("%s%s%s", indent,indent, nd->name);
    if(strcmp(nd->name,"TYPE")==0)
    {
        printf(": %s",nd->strval);
    }
    else if(strcmp(nd->name,"ID")==0)
    {
        printf(": %s",nd->strval);
    }
    else if(strcmp(nd->name,"INT")==0)
    {
        printf(": %d",nd->intval);
    }
    else if(strcmp(nd->name,"FLOAT")==0)
    {
        printf(": %lf",nd->floatval);
    }
    else if(strcmp(nd->name,"LP")==0 ||
            strcmp(nd->name,"RP")==0 ||
            strcmp(nd->name,"LC")==0||
            strcmp(nd->name,"RC")==0 ||
            strcmp(nd->name,"LP")==0 ||
            strcmp(nd->name,"LB")==0 ||
            strcmp(nd->name,"RB")==0 ||
            strcmp(nd->name,"SEMI")==0 ||
            strcmp(nd->name,"ASSIGNOP")==0 ||
            strcmp(nd->name,"PLUS")==0 ||
            strcmp(nd->name,"MINUS")==0 ||
            strcmp(nd->name,"STAR")==0 ||
            strcmp(nd->name,"DIV")==0 ||
            strcmp(nd->name,"STRUCT")==0 ||
            strcmp(nd->name,"COMMA")==0 ||
            strcmp(nd->name,"DOT")==0 ||
            strcmp(nd->name,"RELOP")==0 ||
            strcmp(nd->name,"AND")==0 ||
            strcmp(nd->name,"OR")==0 ||
            strcmp(nd->name,"NOT")==0 ||
            strcmp(nd->name,"RETURN")==0 ||
            strcmp(nd->name,"IF")==0 ||
            strcmp(nd->name,"ELSE")==0 ||
            strcmp(nd->name,"WHILE")==0
            )
    {
        
    }
    else
    {
        printf(" (%d)",nd->lineno);
    }
    printf("\n");
    for(i = 0; i < nd->cnt; i++) {
        if(nd->ch[i] == NULL) continue;
        printTree(nd->ch[i], level + 1);
    }
}
