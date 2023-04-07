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
                    *ptr = malloc(strlen(currentToken)+1);
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    push(tokens, (void *)ptr);
                    num_tokens++;
                    char * relation = "!";
                    if(conditionalString[i + 1] == '='){
                        relation = "!=";
                        i++;
                    }
                    else{
                        printf("Lexical Error\n");
                        return NULL;
                    }
                    ptr = malloc(sizeof(char*));
                    *ptr = malloc(sizeof(relation)+1);
                    strcpy(*ptr, relation);
                    push(tokens, (void *)ptr);
                }

                currentToken[0] = '\0';
                currentTokenIdx = 0;
                break;
            case ('\"'):
                if (parsingString) {
                    currentToken[currentTokenIdx + 1] = '\0';
                    currentToken[currentTokenIdx] = '\"';
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken) + 1);
                    (*ptr)[0] = '\0';
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
                    push(tokens, (void *)ptr);
                    //char *copyToken = malloc(strlen(currentToken) + 1);
                    //strcpy(copyToken, currentToken);
                    //push(tokens, (void *)&copyToken);
                    num_tokens++;
                    currentToken[0] = '\0';
                    currentTokenIdx = 0;

                    parsingString = false;
                }
                else if (currentTokenIdx == 0){
                    currentToken[1] = '\0';
                    currentToken[0] = '\"';
                    currentTokenIdx = 1;
                    parsingString = true;
                }
                else {
                    char ** ptr = malloc(sizeof(char *));
                    *ptr = malloc(strlen(currentToken) + 1);
                    (*ptr)[0] = '\0';
                    strcpy(*ptr, currentToken);
                    (*ptr)[strlen(currentToken)] = '\0';
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
    printf("Current Token: %s\n", next_token);
    ConditionalParseTree *tree = initConditionalParseTree();
    (tree->type)[4] = '\0';
    (tree->type)[0] = 't';
    (tree->type)[1] = 'e';
    (tree->type)[2] = 's';
    (tree->type)[3] = 't';;

    if(strcmp(next_token, "true") == 0 || strcmp(next_token, "false") == 0){
        (tree->val)[0] = '\0';
        strcpy(tree->val, next_token);
        (tree->val)[strlen(next_token)] = '\0';
        return tree;
    }

    char *relToken = *(char **)pop(tokens);

    if (strcmp(relToken, "=") != 0 && strcmp(relToken, "<") != 0 && strcmp(relToken, ">") != 0 &&
        strcmp(relToken, "<=") != 0 && strcmp(relToken, ">=") != 0 && strcmp(relToken, "!=") != 0) {
        printf("Syntax Error\n");
        return NULL;
    }
    (tree->val)[0] = '\0';
    strcpy(tree->val, relToken);
    (tree->val)[strlen(relToken)] = '\0';
    tree->right = initConditionalParseTree();
    if (next_token[0] == '\"') {
        // remove quotes
        next_token = next_token + 1;
        next_token[strlen(next_token) - 1] = '\0';

        ((tree->right)->type)[0] = '\0';
        strcpy((tree->right)->type, "const");
        ((tree->right)->type)[5] = '\0';
        ((tree->right)->val)[0] = '\0';
        strcpy((tree->right)->val, next_token);
        ((tree->right)->val)[strlen(next_token)] = '\0';
    } else if (isdigit(next_token[0])) {
        ((tree->right)->type)[0] = '\0';
        (tree->right)->type = "const";
        char * constant = "const";
        (tree->right)->type = constant;
        ((tree->right)->val)[0] = '\0';
        strcpy((tree->right)->val, next_token);
        ((tree->right)->val)[strlen(next_token)] = '\0';
    } else {
        ((tree->right)->type)[0] = '\0';
        (tree->right)->type = "attr";
        ((tree->right)->val)[0] = '\0';
        strcpy((tree->right)->val, next_token);
        ((tree->right)->val)[strlen(next_token)] = '\0';
    }

    next_token = *(char **) (pop(tokens));
    printf("Current Token: %s\n", next_token);
    tree->left = initConditionalParseTree();
    if (next_token[0] == '\"') {
        next_token = next_token + 1;
        next_token[strlen(next_token) - 1] = '\0';

        if (strcmp((tree->right)->type, "attr") == 0) {
            printf("Syntax Error\n");
            return NULL;
        }
        ((tree->left)->type)[0] = '\0';
        (tree->left)->type = "const";
        ((tree->left)->type)[5] = '\0';
        ((tree->left)->val)[0] = '\0';
        strcpy((tree->left)->val, next_token);
        ((tree->left)->val)[strlen(next_token)] = '\0';
    } else if (isdigit(next_token[0])) {
        if (strcmp(tree->right->type, "attr") == 0) {
            printf("Syntax Error\n");
            return NULL;
        }
        ((tree->left)->type)[0] = '\0';
        strcpy((tree->left)->type, "const");
        ((tree->left)->type)[5] = '\0';
        ((tree->left)->val)[0] = '\0';
        strcpy((tree->left)->val, next_token);
        ((tree->left)->val)[strlen(next_token)] = '\0';
    } else {
        ((tree->left)->type)[0] = '\0';
        strcpy((tree->left)->type, "attr");
        ((tree->left)->type)[4] = '\0';
        ((tree->left)->val)[0] = '\0';
        strcpy((tree->left)->val, next_token);
        ((tree->left)->val)[strlen(next_token)] = '\0';

    }

    //printf("Test Tree Type: %s\n", tree->type);
    //printf("Test Tree Val: %s\n", tree->val);
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
        (andTree->type)[3] = '\0';
        (andTree->type)[0] = 'a';
        (andTree->type)[1] = 'n';
        (andTree->type)[2] = 'd';

        (andTree->val)[3] = '\0';
        (andTree->val)[0] = 'a';
        (andTree->val)[1] = 'n';
        (andTree->val)[2] = 'd';

        andTree->right = testTree;
        andTree->left = parseAndConditional(tokens);
        return andTree;
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
        (andTree->type)[3] = '\0';
        (andTree->type)[0] = 'a';
        (andTree->type)[1] = 'n';
        (andTree->type)[2] = 'd';

        (andTree->val)[3] = '\0';
        (andTree->val)[0] = 'a';
        (andTree->val)[1] = 'n';
        (andTree->val)[2] = 'd';

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
    ConditionalParseTree * conditionalParseTree = parseOrConditional(tokens);
    return conditionalParseTree;
}














