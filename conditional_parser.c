//
// Created by Stewart Lantner on 3/31/23.
//
#include "conditional_parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

Stack * initStack(){
    Stack * stack = malloc(sizeof(Stack));
    stack -> arr = (void **)malloc(10 * sizeof(void *));
    stack -> size = 0;
    stack -> capacity = 10;
    return stack;
}

void resizeStack(Stack * stack){
    void ** new_arr = (void **) malloc(2 * stack->capacity * sizeof(void *));
    for(int i = 0; i < stack->size; i++){
        new_arr[i] = stack->arr[i];
    }
    stack->arr = new_arr;
    stack->capacity = 2 * stack->capacity;
}


void push(Stack * stack, void * val){
    if(stack -> size == stack -> capacity){
        resizeStack(stack);
    }

    stack->arr[stack->size] = val;

    stack->size++;
}

void * pop(Stack * stack){
    void * val = stack->arr[stack->size - 1];
    (stack->size)--;
    return val;
}

bool stackIsEmpty(Stack * stack){
    return stack->size == 0;
}

ConditionalParseTree * initConditionalParseTree(){
    ConditionalParseTree * new_tree = malloc(sizeof(ConditionalParseTree));
    new_tree->val = malloc(256);
    new_tree->type = malloc(256);
    new_tree -> left = NULL;
    new_tree -> right = NULL;
    return new_tree;
}

Stack * getTokensFromConditionalString(char * conditionalString) {
    Stack * tokens = initStack();
    char * currentToken = malloc(256);
    currentToken[0] = '\0';
    int num_tokens = 0;
    bool parsingString = false;
    int currentTokenIdx = 0;
    int i = 0;
    while(i < strlen(conditionalString)){
        switch (conditionalString[i]) {
            case (' '):
                if (parsingString) {
                    currentToken[currentTokenIdx] = ' ';
                    currentTokenIdx++;
                } else if (currentToken[0] != '\0') {
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken) + 1);
                    (*ptr)[0] = '\0';
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    push(tokens, (void *)ptr);
                    num_tokens++;
                    currentToken[0] = '\0';
                    currentTokenIdx = 0;
                }
                break;
            case ('='):
                if(true){
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken) + 1);
                    (*ptr)[0] = '\0';
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    push(tokens, (void *)ptr);
                    num_tokens++;

                    char ** equals = malloc(sizeof(char *));
                    *equals = malloc(2);
                    (*equals)[1] = '\0';
                    (*equals)[0] = '=';

                    push(tokens, (void *)equals);
                    currentToken[0] = '\0';
                    currentTokenIdx = 0;
                    num_tokens++;
                }
                break;

            case ('<'):
                if (true) {
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken)+1);
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    push(tokens, (void *)ptr);
                    num_tokens++;
                    char *relation = "<";
                    if (conditionalString[i + 1] == '=') {
                        relation = "<=";
                        i += 1;
                    }
                    ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(relation) + 1);
                    strcpy(*ptr, relation);
                    (*ptr)[strlen(relation)] = '\0';
                    push(tokens, (void *) ptr);
                }
                currentToken[0] = '\0';
                currentTokenIdx = 0;
                num_tokens++;
                break;
            case ('>'):
                if (true) {
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken)+1);
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    push(tokens, (void *)ptr);
                    num_tokens++;
                    char * relation = ">";
                    if(conditionalString[i + 1] == '='){
                        relation = ">=";
                        i++;
                    }
                    ptr = malloc(sizeof(char*));
                    *ptr = malloc(sizeof(relation)+1);
                    strcpy(*ptr, relation);
                    push(tokens, (void *)ptr);
                }

                currentToken[0] = '\0';
                currentTokenIdx = 0;
                break;
            case ('!'):
                if (true) {
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken) + 1);
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    char *copyToken = malloc(strlen(currentToken) + 1);
                    strcpy(copyToken, currentToken);
                    push(tokens, (void *)&copyToken);
                    num_tokens++;
                    char * relation = "!=";
                    ptr = malloc(sizeof(char *));
                    *ptr = malloc(3);
                    strcpy(*ptr, relation);
                    (*ptr)[3] = '\0';
                    push(tokens, (void *)ptr);
                    i++;
                    currentToken[0] = '\0';
                    currentTokenIdx = 0;
                    num_tokens++;
                }
                break;
            case ('\"'):
                if (parsingString) {
                    currentToken[currentTokenIdx] = '\"';
                    currentToken[currentTokenIdx + 1] = '\0';
                    char *copyToken = malloc(strlen(currentToken) + 1);
                    strcpy(copyToken, currentToken);
                    push(tokens, (void *)&copyToken);
                    num_tokens++;
                    currentTokenIdx = 0;
                    parsingString = false;
                } else {
                    currentToken[currentTokenIdx] = '\0';
                    char *copyToken = malloc(strlen(currentToken) + 1);
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken) + 1);
                    strcpy(*ptr, currentToken);

                    push(tokens, (void *)ptr);
                    num_tokens++;
                    currentToken[1] = '\0';
                    currentToken[0] = '\"';
                    currentTokenIdx = 1;
                    parsingString = true;
                }
                break;
            default:
                currentToken[currentTokenIdx + 1] = '\0';
                currentToken[currentTokenIdx] = conditionalString[i];
                currentTokenIdx++;
        }
        i++;
    }
        if(currentToken[0] != '\0'){
            //char * copyToken = malloc(strlen(currentToken) + 1);
            char ** ptr = malloc(sizeof(char *));
            *ptr = malloc(sizeof(currentToken)+1);
            (*ptr)[strlen(currentToken)] = '\0';
            strcpy(*ptr, currentToken);
            push(tokens, (void *)ptr);
            num_tokens++;
        }
        return tokens;
}

ConditionalParseTree * parseTestConditional(Stack * tokens) {
    /*
     * true | false | attr rel attr | attr rel const | const rel const
     */
    if(stackIsEmpty(tokens)) {
        printf("Syntax Error\n");
        return NULL;
    }
    char *next_token = *(char **)pop(tokens);
    ConditionalParseTree *tree = initConditionalParseTree();
    (tree->type)[0] = '\0';
    strcmp(tree->type, "test");
    (tree->type)[4] = '\0';

    if(strcmp(next_token, "true") == 0 || strcmp(next_token, "false") == 0){
        (tree->val)[0] = '\0';
        strcmp(tree->val, next_token);
        (tree->val)[strlen(next_token)] = '\0';
        return tree;
    }

    char *relToken = *(char **)pop(tokens);

    strcpy(tree->type, "test");
    if (strcmp(relToken, "=") != 0 && strcmp(relToken, "<") != 0 && strcmp(relToken, ">") != 0 &&
        strcmp(relToken, "<=") != 0 & strcmp(relToken, ">=") != 0 && strcmp(relToken, "!=") != 0) {
        printf("Syntax Error\n");
        return NULL;
    }
    strcpy(tree->val, relToken);
    tree->right = initConditionalParseTree();
    if (next_token[0] == '\"') {
        (tree->right)->type = "const";
        strcpy((tree->right)->val, next_token);
    } else if (isdigit(next_token[0])) {
        (tree->right)->type = "const";
        strcpy((tree->right)->val, next_token);
    } else {
        (tree->right)->type = "attr";
        strcpy((tree->right)->val, next_token);
    }

    next_token = *(char **) (pop(tokens));
    tree->left = initConditionalParseTree();
    if (next_token[0] == '\"') {
        if (strcmp(tree->right->type, "attr") == 0) {
            printf("Syntax Error\n");
            return NULL;
        }
        ((tree->left)->type)[0] = '\0';
        (tree->left)->type = "const";
        strcpy((tree->left)->val, next_token);
    } else if (isdigit(next_token[0])) {
        if (strcmp(tree->right->type, "attr") == 0) {
            printf("Syntax Error\n");
            return NULL;
        }
        ((tree->left)->type)[0] = '\0';
        strcpy((tree->left)->type, "const");
        ((tree->left)->type)[5] = '\0';
        strcpy((tree->left)->val, next_token);
    } else {
        ((tree->left)->type)[0] = '\0';
        strcpy((tree->left)->type, "attr");
        ((tree->left)->type)[4] = '\0';
        ((tree->left)->val)[0] = '\0';
        strcpy((tree->left)->val, next_token);
        ((tree->left)->val)[strlen(next_token)] = '\0';

    }
    return tree;
}


ConditionalParseTree * parseAndConditional(Stack * tokens){
    /*
     * test | andCond and andCond | andCond and test
     */
    ConditionalParseTree * testTree = parseTestConditional(tokens);

    if(stackIsEmpty(tokens)){
        return testTree;
    }

    char * currentToken = *(char **)pop(tokens);
    if(strcmp(currentToken, "and") == 0){
        ConditionalParseTree * andTree = initConditionalParseTree();
        strcpy(andTree->type, "and");
        strcpy(andTree->val, "and");
        andTree->right = testTree;
        andTree->left = parseAndConditional(tokens);
    }
    else{
        return testTree;
    }
}

ConditionalParseTree * parseOrConditional(Stack * tokens){
    /*
     * test | orCond or orCond | andCond or test | andCond
     */

    ConditionalParseTree * testTree = parseTestConditional(tokens);

    if(stackIsEmpty(tokens)){
        return testTree;
    }

    char * nextToken = *(char **)pop(tokens);
    if(strcmp(nextToken, "or") == 0){
        ConditionalParseTree * orTree = initConditionalParseTree();
        strcpy(orTree->type, "or");
        strcpy(orTree->val, "or");
        orTree->right = testTree;
        orTree->left = parseOrConditional(tokens);
        return orTree;
    }

    else if(strcmp(nextToken, "and") == 0){
        ConditionalParseTree * andTree =  initConditionalParseTree();
        strcpy(andTree->type, "and");
        strcpy(andTree->val, "and");
        andTree->right = testTree;
        andTree->left = parseAndConditional(tokens);
        if(stackIsEmpty(tokens)){
            return andTree;
        }
        else{
            nextToken = *(char **)pop(tokens);
            if(strcmp(nextToken, "or") == 0){
                ConditionalParseTree * orTree = initConditionalParseTree();
                strcpy(orTree->type, "or");
                strcpy(orTree->val, "or");
                orTree -> right = andTree;
                orTree -> left = parseOrConditional(tokens);
                return orTree;
            }
            else{
                printf("syntax Error\n");
                return NULL;

            }
        }
    }
    else {
        printf("Syntax Error\n");
        return NULL;

    }


}

void printConditionalParseTree(ConditionalParseTree * tree){
    if(strcmp(tree->type, "test") == 0){
        if(strcmp(tree->val, "true") == 0 || strcmp(tree->val, "false") == 0){
            printf("%s\n", tree->val);
        }
        else{
            printf("%s\n", tree->left->val);
            printf("%s\n", tree->val);
            printf("%s\n", tree->right->val);
        }
    }
    else{
        printConditionalParseTree(tree->left);
        printf("%s\n", tree->val);
        printConditionalParseTree(tree->right);
    }
}

ConditionalParseTree * parseConditional(char * conditionalString){
    /*
     * true | orCond
     */
    Stack * tokens = getTokensFromConditionalString(conditionalString);

    /*
    while(stackIsEmpty(tokens) == false){
        printf("Token: %s\n", *(char **)pop(tokens));
    }
     */

    if(stackIsEmpty(tokens)){
        ConditionalParseTree * conditionalParseTree = initConditionalParseTree();
        strcpy(conditionalParseTree->val, "true");
        strcpy(conditionalParseTree->type, "test");
        printConditionalParseTree(conditionalParseTree);
        return conditionalParseTree;
    }

    ConditionalParseTree * conditionalParseTree = parseOrConditional(tokens);
    return conditionalParseTree;
}














