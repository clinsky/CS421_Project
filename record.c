//
// Created by Jared Lantner on 2/11/23.
//
#include "record.h"
#include "conditional_parser.h"
#include "catalog.h"
#include <string.h>

bool compareIntegers(int lvalue, int rvalue, char * op){
    if(strcmp(op, "=") == 0){
        return lvalue == rvalue;
    }
    else if(strcmp(op, "<") == 0){
        return lvalue < rvalue;
    }
    else if(strcmp(op, ">") == 0){
        return lvalue > rvalue;
    }
    else if(strcmp(op, "<=") == 0){
        return lvalue <= rvalue;
    }
    else if(strcmp(op, ">=") == 0){
        return lvalue >= rvalue;
    }
    else if(strcmp(op, "!=") == 0){
        return lvalue != rvalue;
    }
    else{
        printf("Error\n");
        return false;
    }
}

bool compareBools(bool lvalue, bool rvalue, char * op){
    if(strcmp(op, "=") == 0){
        return lvalue == rvalue;
    }

    else if(strcmp(op, "!=") == 0){
        return lvalue != rvalue;
    }

    else{
        printf("Error\n");
        return false;
    }
}

bool compareDoubles(double lvalue, double rvalue, char * op){
    if(strcmp(op, "=") == 0){
        return lvalue == rvalue;
    }
    else if(strcmp(op, "<") == 0){
        return lvalue < rvalue;
    }
    else if(strcmp(op, ">") == 0){
        return lvalue > rvalue;
    }
    else if(strcmp(op, "<=") == 0){
        return lvalue <= rvalue;
    }
    else if(strcmp(op, ">=") == 0){
        return lvalue >= rvalue;
    }
    else if(strcmp(op, "!=") == 0){
        return lvalue != rvalue;
    }
    else{
        printf("Error\n");
        return false;
    }
}

bool compareStrings(char * lvalue, char * rvalue, char * op){
    if(strcmp(op, "=") == 0){
        return strcmp(lvalue, rvalue) == 0;
    }
    else if(strcmp(op, "<") == 0){
        return strcmp(lvalue, rvalue) < 0;
    }
    else if(strcmp(op, ">") == 0){
        return strcmp(lvalue, rvalue) > 0;
    }
    else if(strcmp(op, "<=") == 0){
        return strcmp(lvalue, rvalue) <= 0;
    }
    else if(strcmp(op, ">=") == 0){
        return strcmp(lvalue, rvalue) >= 0;
    }
    else if(strcmp(op, "!=") == 0){
        return strcmp(lvalue, rvalue) != 0;
    }
    else{
        printf("Error\n");
        return false;
    }
}

bool evaluateTest(Record * record, ConditionalParseTree * conditionalParseTree, Table * table) {
    if (strcmp(conditionalParseTree->val, "true") == 0) {
        return true;
    } else if (strcmp(conditionalParseTree->val, "false") == 0) {
        return false;
    } else {
        if (strcmp(conditionalParseTree->left->type, "attr") == 0 &&
            strcmp(conditionalParseTree->right->type, "attr") == 0) {
            int attr_idx = 0;
            Attribute_Values left_value;
            Attribute_Values right_value;
            for (int i = 0; i < table->num_attributes; i++) {
                if (strcmp(table->attributes[i].name, conditionalParseTree->left->val) == 0) {
                    left_value = record->attr_vals[i];
                }
            }

            for (int i = 0; i < table->num_attributes; i++) {
                if (strcmp(table->attributes[i].name, conditionalParseTree->right->val) == 0) {
                    right_value = record->attr_vals[i];
                }
            }

            // enum ATTRIBUTE_TYPE { INTEGER, BOOL, DOUBLE, CHAR, VARCHAR, INVALID_ATTR };
            switch (left_value.type) {
                case (INTEGER):
                    return compareIntegers(left_value.int_val, right_value.int_val, conditionalParseTree->val);
                    break;
                case (BOOL):
                    return compareBools(left_value.bool_val, right_value.bool_val, conditionalParseTree->val);
                    break;
                case (DOUBLE):
                    return compareDoubles(left_value.double_val, right_value.double_val, conditionalParseTree->val);
                    break;
                case (CHAR):
                    return compareStrings(left_value.chars_val, right_value.chars_val, conditionalParseTree->val);
                    break;
                case (VARCHAR):
                    return compareStrings(left_value.chars_val, right_value.chars_val, conditionalParseTree->val);
                    break;
                default:
                    printf("Error\n");
                    return false;
            }
        }
        if (strcmp(conditionalParseTree->left->type, "attr") == 0 &&
            strcmp(conditionalParseTree->right->type, "const") == 0) {
            int attr_idx = 0;
            Attribute_Values left_value;
            for (int i = 0; i < table->num_attributes; i++) {
                if (strcmp(table->attributes[i].name, conditionalParseTree->left->val) == 0) {
                    left_value = record->attr_vals[i];
                }
            }

            // enum ATTRIBUTE_TYPE { INTEGER, BOOL, DOUBLE, CHAR, VARCHAR, INVALID_ATTR };
            switch (left_value.type) {
                case (INTEGER):
                    return compareIntegers(left_value.int_val, atoi(conditionalParseTree->right->val),
                                           conditionalParseTree->val);
                    break;
                case (BOOL):
                    if (strcmp(conditionalParseTree->right->val, "true") == 0) {
                        return compareBools(left_value.bool_val, true, conditionalParseTree->val);
                    } else if (strcmp(conditionalParseTree->right->val, "false") == 0) {
                        return compareBools(left_value.bool_val, false, conditionalParseTree->val);
                    }
                    break;
                case (DOUBLE):
                    return compareDoubles(left_value.double_val, strtod(conditionalParseTree->right->val, NULL),
                                          conditionalParseTree->val);
                    break;
                case (CHAR):
                    return compareStrings(left_value.chars_val, conditionalParseTree->right->val,
                                          conditionalParseTree->val);
                    break;
                case (VARCHAR):
                    return compareStrings(left_value.chars_val, conditionalParseTree->right->val,
                                          conditionalParseTree->val);
                    break;
                default:
                    printf("Error\n");
                    return false;
            }
        } else if (strcmp(conditionalParseTree->left->type, "const") == 0 &&
                   strcmp(conditionalParseTree->right->type, "const") == 0) {
            return compareStrings(conditionalParseTree->left->val, conditionalParseTree->right->val,
                                  conditionalParseTree->val);
        } else {
            printf("Error\n");
            return false;
        }



        /*
         * struct attribute_values {
            ATTRIBUTE_TYPE type; // type of attribute
            int int_val;
            char *chars_val; // chars and varchars vals;
            double double_val;
            int bool_val;
            bool is_null;
        };
         */
    }
}

bool evaluateCondition(Record * record, ConditionalParseTree * conditionalParseTree, Table * table){

    if(strcmp(conditionalParseTree->type, "test") == 0){
        // printf("going evaluate test\n");
        return evaluateTest(record, conditionalParseTree, table);
    }
    else if(strcmp(conditionalParseTree->type, "or") == 0) {
        // printf("going evaluate or\n");
        return evaluateCondition(record, conditionalParseTree->left, table) ||
               evaluateCondition(record, conditionalParseTree->right, table);
    }
    else if(strcmp(conditionalParseTree->type, "and") == 0) {
        // printf("going evaluate and\n");
        return evaluateCondition(record, conditionalParseTree->left, table) &&
               evaluateCondition(record, conditionalParseTree->right, table);
    }
    else{
        printf("Error\n");
        return false;
    }

}
