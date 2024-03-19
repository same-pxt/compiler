#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashNode gTable[1024];
HashNode sTable[1024];
int hashFunc(char *key){
    unsigned int hash = 5381;
    while(*key){
        hash += (hash << 5 ) + (*key++);
    }
    return (hash & 0x7FFFFFFF) % 1024;
}


HashNode insert(char *name,Type type){

    int hash=hashFunc(name);
    HashNode newNode = (HashNode)malloc(sizeof(struct HashNode_));
    newNode->name= name;
    newNode->type = type;
    newNode->next = NULL;
    
    HashNode node = gTable[hash];
    
    if(node==NULL){
        gTable[hash] = newNode;
        return gTable[hash];
    }else{
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = newNode;
        return node->next;
    }
    return NULL;
}

int check(char *name){
    if(name==NULL)
        return 0;
    int hash=hashFunc(name);
    HashNode node = gTable[hash];
    while (node != NULL) {
        if (strcmp(node->name, name) == 0) {
            return 1; // 找到了该名字
        }
        node = node->next;
    }
    return 0; // 没找到该名字
}

HashNode find(char *name)
{
    int hash=hashFunc(name);
    HashNode node = gTable[hash];
    while (node != NULL) {
        if (strcmp(node->name, name) == 0) {
            return node; // 找到了该名字
        }
        node = node->next;
    }
    return NULL; // 没找到该名字
}