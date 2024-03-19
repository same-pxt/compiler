#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stddef.h>

typedef struct HashNode_* HashNode;
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Structure_* Structure;
typedef struct Function_* Function;
// 变量、参数、结构体的域
struct FieldList_{
    char *name;
    Type type;
    FieldList tail;
};
// 结构体类型，包含名称、域
struct Structure_{
    char *name;
    FieldList domain;
};
// 函数类型，包括名称、行号、返回值类型、参数
struct Function_{
    char *name;
    int line;
    Type type;
    FieldList param;
};
struct Type_{
    enum {
        BASIC, // variable
        ARRAY, // array
        STRUCTURE, // structure
        FUNCTION // function
    } kind;
    union{
        int basic; // int(1), float(2)
        struct {
            Type elem; 
            int size;
        } array; 
        Structure structure;
        Function function;
    } u;
    enum {
        RIGHT, // right value
        BOTH, // left | right value
        LEFT // left value

    } assign;
    int is_dec_func;
};   
typedef enum SymbolKind 
{ 
    VAR, 
    FIELD, 
    STRUCT, 
    FUNC 
} SymbolKind;

struct HashNode_{
    char *name;
    Type type;
    FieldList param;
    struct HashNode_ *next;
    int isaddr;
};

int hashFunc(char *key);
HashNode insert(char *name,Type type);
int check(char *name);
HashNode find(char *name);


#endif