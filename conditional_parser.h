//
// Created by Stewart Lantner on 3/31/23.
//

#include <stdbool.h>


#ifndef CS421_PROJECT_CONDITIONALPARSER_H
#define CS421_PROJECT_CONDITIONALPARSER_H

struct stack {
    void ** arr;
    int size;
    int capacity;
};



typedef struct stack Stack;

Stack * initStack();
void resizeStack(Stack * stack);
void push(Stack * stack, void * val);
void * pop (Stack * stack);
bool stackIsEmpty(Stack * stack);

struct conditionalParseTree{
    char * val;
    char * type;
    struct conditionalParseTree * left;
    struct conditionalParseTree * right;
};

typedef struct conditionalParseTree ConditionalParseTree;

Stack * getTokensFromConditionalString(char * conditionalString);
ConditionalParseTree * initConditionalParseTree();
ConditionalParseTree * parseConditional(char * conditionalString);

#endif //CS421_PROJECT_CONDITIONALPARSER_H
