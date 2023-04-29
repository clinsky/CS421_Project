//
// Created by Stewart Lantner on 4/22/23.
//

#include "bplus_tree.h"
#include <math.h>



BPlusTree * init_BPlusTree(int N, bool is_root, bool is_leaf) {
    BPlusTree * bPlusTree = (BPlusTree *)malloc(sizeof(BPlusTree));
    bPlusTree->is_root = is_root;
    bPlusTree->is_leaf = is_leaf;
    /*
     * A Node that is not a root or a leaf has between ceil(N/2) and N children
     * A leaf node has between ceil((N-1)/2) and N-1 values
     * If the root is not a leaf, it has at least 2 children.
     * If the root is a leaf, it can have between 0 and (N-1) values
     */
    bPlusTree->N = N;
    if(bPlusTree->is_root == false && bPlusTree->is_leaf == false){
        bPlusTree->min_num_key_values = ceil(N/2.0);
        bPlusTree->max_num_key_values = N;
    }
    else if(bPlusTree->is_root && bPlusTree->is_leaf == false){
        bPlusTree->min_num_key_values = 2;
        bPlusTree->max_num_key_values = N;
    }
    else if(bPlusTree->is_root && bPlusTree->is_leaf){
        bPlusTree->min_num_key_values = 0;
        bPlusTree->max_num_key_values = N - 1;
    }
    else if(bPlusTree->is_root == false && bPlusTree->is_leaf){
        bPlusTree->min_num_key_values = ceil((N-1)/2.0);
        bPlusTree->max_num_key_values = N - 1;
    }
    bPlusTree->search_key_values = malloc(sizeof(int) * (N - 1));
    bPlusTree->ptrs = (void **)malloc(sizeof(void *) * N);
    bPlusTree->num_search_keys = 0;
    bPlusTree->num_ptrs = 0;
    bPlusTree->parent = NULL;
    for(int i = 0; i < N - 1; i++){
        bPlusTree->search_key_values[i] = 0;
    }
    for(int i = 0; i < N; i++){
        bPlusTree->ptrs[i] = NULL;
    }
    return bPlusTree;
}

void insert_in_parent(BPlusTree * left, int K, BPlusTree * right) {
    if (left->is_root) {
        // create a new node R containing (N, K', N')
        BPlusTree *parent = init_BPlusTree(left->N, true, false);
        parent->search_key_values[0] = K;
        parent->ptrs[0] = left;
        parent->ptrs[1] = right;
        parent->num_search_keys = 1;
        parent->num_ptrs = 2;
        left->is_root = false;
        right->is_root = false;
        left->parent = parent;
        right->parent = parent;
    } else {
        BPlusTree *parent = left->parent;
        if (parent->num_ptrs < parent->N) {
            // Insert (K'.N') in P just after N
            int i = 0;
            while (i < parent->num_search_keys && K >= parent->search_key_values[i]) {
                i++;
            }
            int val_to_be_inserted = K;
            void *ptr_to_be_inserted = right;
            for (int j = i; j <= parent->num_search_keys; j++) {
                int temp_val = parent->search_key_values[j];
                void *temp_ptr = parent->ptrs[j+1];
                parent->search_key_values[j] = val_to_be_inserted;
                parent->ptrs[j+1] = ptr_to_be_inserted;
                val_to_be_inserted = temp_val;
                ptr_to_be_inserted = temp_ptr;
            }
            parent->num_search_keys++;
            parent->num_ptrs++;
            right->parent = parent;
        } else {
            // Split the parent

            // Copy P to a block of memory T that can hold P and (K', N')
            int *T_vals = malloc(sizeof(int) * (parent->N));
            void **T_ptrs = malloc(sizeof(void *) * (parent->N + 1));

            for (int i = 0; i < parent->N - 1; i++) {
                T_vals[i] = parent->search_key_values[i];
            }
            for(int i = 0; i < parent->N; i++){
                T_ptrs[i] = parent->ptrs[i];
            }

            // Insert (K', N') into T just after N
            //int value = left->search_key_values[0];
            int i = 0;
            while (i < parent->num_search_keys && K >= parent->search_key_values[i]) {
                i++;
            }
            int val_to_be_inserted = K;
            void *ptr_to_be_inserted = right;
            for (int j = i; j < parent->N; j++) {
                int temp_val = T_vals[j];
                T_vals[j] = val_to_be_inserted;
                val_to_be_inserted = temp_val;
            }

            for(int j = i + 1; j < parent->N + 1; j++){
                void * temp_ptr = T_ptrs[j];
                T_ptrs[j] = ptr_to_be_inserted;
                ptr_to_be_inserted = temp_ptr;
            }

            // Erase all entries from P; Create node P'
            parent->num_search_keys = 0;
            parent->num_ptrs = 0;
            // Create node P'
            BPlusTree *parent_sibling = init_BPlusTree(parent->N, false, parent->is_leaf);
            // copy first ceil((n+1)/2) vals and ptrs into P
            for (int k = 0; k < ceil((parent->N + 1) / 2.0) - 1; k++) {
                parent->search_key_values[k] = T_vals[k];
                parent->num_search_keys++;
            }

            for (int k = 0; k < ceil((parent->N + 1) / 2.0); k++) {
                parent->ptrs[k] = T_ptrs[k];
                parent->num_ptrs++;
            }

            // let K'' = T.K[ceil((n+1)/2)].
            int K2 = T_vals[(int) ceil((parent->N + 1) / 2.0) - 1];

            for (int k = (int) ceil((parent->N + 1) / 2.0); k < parent->N; k++) {
                parent_sibling->search_key_values[k - (int) (ceil((parent->N + 1) / 2.0))] = T_vals[k];
                parent_sibling->num_search_keys++;

            }

            for(int k = ceil((parent->N + 1) / 2.0); k < parent->N + 1; k++){
                parent_sibling->ptrs[k - (int) ceil((parent->N + 1) / 2.0)] = T_ptrs[k];
                parent_sibling->num_ptrs++;

            }

            right->parent = parent;
            // insert_in_parent(P, K'', P')
            insert_in_parent(parent, K2, parent_sibling);
        }
    }
}

void traverse_tree(BPlusTree * bPlusTree){
    if(bPlusTree->is_leaf){
        for(int i = 0; i < bPlusTree->num_search_keys; i++){
            printf("%d\n", bPlusTree->search_key_values[i]);
        }
    }
    else{
        for(int i = 0; i < bPlusTree->num_ptrs; i++){
            traverse_tree((BPlusTree *)bPlusTree->ptrs[i]);
        }
        for(int i = 0; i < bPlusTree->num_search_keys; i++){
            printf("%d\n", bPlusTree->search_key_values[i]);
        }
    }
}

void split_leaf_node(BPlusTree * bPlusTree, int value, Record * record) {
    // Place the first ceil(n/2) (value, ptr) pairs in the original node
    // Put the rest in a new node
    BPlusTree *right = init_BPlusTree(bPlusTree->N,  false, bPlusTree->is_leaf);
    // Copy pointers and key values to a block of memory T that can hold n (pointer, key-value) pairs
    int * T_values = malloc(sizeof(int) * (right->N));
    void ** T_ptrs = (void **)malloc(sizeof(void *) * (right->N + 1));
    for (int i = 0; i < bPlusTree->N - 1; i++) {
        T_values[i] = bPlusTree->search_key_values[i];
    }

    for (int i = 0; i < bPlusTree->N; i++) {
        T_ptrs[i] = bPlusTree->ptrs[i];
    }

    //insert_in_leaf(T, K, P)
    int i = 0;
    while (i < bPlusTree->num_search_keys && value >= T_values[i]) {
        i++;
    }

    int val_to_be_inserted = value;
    for (int j = i; j < bPlusTree->N; j++) {
        int temp = T_values[j];
        T_values[j] = val_to_be_inserted;
        val_to_be_inserted = temp;
    }

    void *ptr_to_be_inserted = record;
    for (int j = i; j < bPlusTree->N+1; j++) {
        void *temp = T_ptrs[j];
        T_ptrs[j] = ptr_to_be_inserted;
        ptr_to_be_inserted = temp;
    }

    // Set L' P_n = L P_n ; Set L Pn = L'
    right->ptrs[right->N - 1] = bPlusTree->ptrs[bPlusTree->N - 1];
    bPlusTree->ptrs[bPlusTree->N - 1] = right;

    // Delete all pointers and keys from bPlusTree
    bPlusTree->num_search_keys = 0;
    bPlusTree->num_ptrs = 0;
    // Copy TP1 through TK_(ceil(N/2)) from T into L starting at LP1
    for (int k = 0; k < (int) ceil(bPlusTree->N / 2.0); k++) {
        bPlusTree->search_key_values[k] = T_values[k];
        bPlusTree->ptrs[k] = T_ptrs[k];
        bPlusTree->num_search_keys++;
        bPlusTree->num_ptrs++;
    }

    for (int k = ceil(bPlusTree->N / 2.0); k < bPlusTree->N; k++) {
        right->search_key_values[k - (int)(ceil(bPlusTree->N / 2.0))] = T_values[k];
        right->ptrs[k] = T_ptrs[k - (int)(ceil(bPlusTree->N / 2.0))] = T_ptrs[k];
        right->num_search_keys++;
        right->num_ptrs++;
    }

    // Let K' be the smallest key-value in L'
    int K = right->search_key_values[0];
    insert_in_parent(bPlusTree, K, right);
}

bool too_few_values(BPlusTree * bPlusTree){
     if(bPlusTree->is_root == false && bPlusTree->is_leaf == false && bPlusTree->num_search_keys < ceil(bPlusTree->N/2.0)){
         return true;
    }
    else if(bPlusTree->is_root && bPlusTree->is_leaf == false && bPlusTree->num_ptrs < 2){
        return true;
    }
    else if(bPlusTree->is_root == false && bPlusTree->is_leaf && bPlusTree->num_search_keys < ceil((bPlusTree->N-1)/2.0)){
        return true;
    }
    return false;
}


void delete_entry(BPlusTree * bPlusTree, int value, void * ptr) {
    // delete (K, P) from N

    int i = 0;
    while (bPlusTree->search_key_values[i] < value) {
        i++;
    }
    //printf("Deleting %d from tree\n", value);
    //printf("i: %d\n", i);
    for (int j = i; j < bPlusTree->N - 1; j++) {
        bPlusTree->search_key_values[j] = bPlusTree->search_key_values[j + 1];
    }
    for(int j = i; j < bPlusTree->N; j++){
        bPlusTree->ptrs[j] = bPlusTree->ptrs[j + 1];
    }

    bPlusTree->num_search_keys--;
    bPlusTree->num_ptrs--;
    //printf("%d Deleted\n", value);
    //printf("Num Search Keys: %d\n", bPlusTree->num_search_keys);
    //printf("Num Ptrs: %d\n", bPlusTree->num_ptrs);

    // if N is the root and N has only one remaining child
    if (bPlusTree->is_root && bPlusTree->num_ptrs == 1) {
        // Make the child of N the new root of the tree and delete N
        ((BPlusTree *) (bPlusTree->ptrs[0]))->is_root = true;
        *bPlusTree = *(BPlusTree *)bPlusTree->ptrs[0];
    }
    else if (too_few_values(bPlusTree)) {
        // Let N' be the previous or next child of parent(N)
        // Let K' be the value between pointers N and N' in parent(N)

        i = 0;
        while(bPlusTree->parent->search_key_values[i] <= value){
            i++;
        }

        BPlusTree * right_sibling = bPlusTree->parent->ptrs[i+1];
        BPlusTree * left_sibling = bPlusTree->parent->ptrs[i-1];
        //BPlusTree * N2 = right_sibling;
        /*
        int K2 = bPlusTree->parent->search_key_values[i];
        if(N2 == NULL){
            N2 = left_sibling;
            K2 = bPlusTree->parent->search_key_values[i - 1];
        }
         */

        // If entries in N and N' can fit in the
        if (bPlusTree->num_search_keys + right_sibling ->num_search_keys <= bPlusTree -> N - 1) {
            BPlusTree * N2 = right_sibling;
            int K2 = bPlusTree->parent->search_key_values[i];
            // If N is a predecessor of N', then swap_variables(N, N')
            if (K2 > value) {
                // swap_variables(bPlusTree, sibling);
                BPlusTree temp_tree = *bPlusTree;
                *bPlusTree = *N2;
                *N2 = temp_tree;
            }
            if (bPlusTree->is_leaf == false) {
                // append K' and all pointers  and values in N to N'
                N2->search_key_values[0] = K2;
                for (int k = 1; k < bPlusTree->num_search_keys; k++) {
                    N2->search_key_values[k + N2->num_search_keys] = bPlusTree->search_key_values[k];
                    N2->ptrs[k + N2->num_ptrs] = bPlusTree->ptrs[k];
                }
                N2->num_search_keys += bPlusTree->num_search_keys + 1;
                N2->num_ptrs += bPlusTree->num_ptrs;
            } else {
                // 1. append all (Ki, Pi) pairs in N to N'
                for (int k = 0; k < bPlusTree->num_search_keys; k++) {
                    N2->search_key_values[k + N2->num_search_keys] = bPlusTree->search_key_values[k];
                }
                for (int k = 0; k < bPlusTree->num_ptrs; k++){
                    N2->ptrs[k + N2->num_ptrs] = bPlusTree->ptrs[k];
                }
                N2->num_search_keys += bPlusTree->num_search_keys;
                N2->num_ptrs += bPlusTree->num_ptrs;
                // 2. Set N'.Pn = N.Pn
                N2->ptrs[N2->N - 1] = bPlusTree->ptrs[bPlusTree->N - 1];
            }
            // delete_entry(Parent(N), K', N)
            delete_entry(bPlusTree->parent, K2, bPlusTree);
        }
        else if (bPlusTree->num_search_keys + left_sibling ->num_search_keys <= bPlusTree -> N - 1) {
            BPlusTree * N2 = right_sibling;
            int K2 = bPlusTree->parent->search_key_values[i];
            // If N is a predecessor of N', then swap_variables(N, N')
            if (K2 > value) {
                // swap_variables(bPlusTree, sibling);
                BPlusTree temp_tree = *bPlusTree;
                *bPlusTree = *N2;
                *N2 = temp_tree;
            }
            if (bPlusTree->is_leaf == false) {
                // append K' and all pointers  and values in N to N'
                N2->search_key_values[0] = K2;
                for (int k = 1; k < bPlusTree->num_search_keys; k++) {
                    N2->search_key_values[k + N2->num_search_keys] = bPlusTree->search_key_values[k];
                    N2->ptrs[k + N2->num_ptrs] = bPlusTree->ptrs[k];
                }
                N2->num_search_keys += bPlusTree->num_search_keys + 1;
                N2->num_ptrs += bPlusTree->num_ptrs;
            } else {
                // 1. append all (Ki, Pi) pairs in N to N'
                for (int k = 0; k < bPlusTree->num_search_keys; k++) {
                    N2->search_key_values[k + N2->num_search_keys] = bPlusTree->search_key_values[k];
                }
                for (int k = 0; k < bPlusTree->num_ptrs; k++){
                    N2->ptrs[k + N2->num_ptrs] = bPlusTree->ptrs[k];
                }
                N2->num_search_keys += bPlusTree->num_search_keys;
                N2->num_ptrs += bPlusTree->num_ptrs;
                // 2. Set N'.Pn = N.Pn
                N2->ptrs[N2->N - 1] = bPlusTree->ptrs[bPlusTree->N - 1];
            }
            // delete_entry(Parent(N), K', N)
            delete_entry(bPlusTree->parent, K2, bPlusTree);
        }
        else {
            BPlusTree * N2 = right_sibling;
            int K2 = bPlusTree->parent->search_key_values[i];
            // Redistribution of pointers
            if (N2 > bPlusTree){
                if (bPlusTree->is_leaf == false) {
                    // let m be such that N'.Pm is the last pointer in N'
                    int m = N2->num_ptrs - 1;
                    // remove (N'.K_m-1, N'.Pm) from N'
                    for (int k = m - 1; k < N2->num_search_keys; k++) {
                        N2->search_key_values[k] = N2->search_key_values[k + 1];
                    }
                    N2->num_search_keys--;
                    for (int k = m; k < N2->num_ptrs; k++) {
                        N2->ptrs[k] = N2->ptrs[k + 1];
                    }
                    N2->num_ptrs--;
                    // insert (N'.Pm, K') as the first pointer and value in N
                    int val_to_be_inserted = K2;
                    void *ptr_to_be_inserted = N2->ptrs[m - 1];

                    for (int k = 0; k < bPlusTree->num_search_keys; k++) {
                        int temp_val = bPlusTree->search_key_values[k];
                        void *temp_ptr = bPlusTree->ptrs[k];
                        bPlusTree->search_key_values[k] = val_to_be_inserted;
                        bPlusTree->ptrs[k] = ptr_to_be_inserted;
                        val_to_be_inserted = temp_val;
                        ptr_to_be_inserted = temp_ptr;
                    }
                    // replace K' in parent(N) by N'.Km-1
                    BPlusTree * parent = bPlusTree->parent;
                    i = 0;
                    while (parent->search_key_values[i] <= K2) {
                        i++;
                    }
                    parent->search_key_values[i] = N2->search_key_values[m - 1];
                } else {
                    // let m be such that (N'.Pm, N'.Km) is the last pointer/value pair in N'
                    int m = 0;
                    int Km = N2->search_key_values[m];
                    void * Nm = N2->ptrs[m];
                    // remove (N' Pm, N' Km) from N'
                    for(int q = 0; q < N2->N - 1; q++){
                        N2->search_key_values[q] = N2->search_key_values[q+1];
                        N2->ptrs[q] = N2->ptrs[q+1];
                    }
                    N2->num_search_keys--;
                    N2->num_ptrs--;


                    // Insert (N' Pm, N' Km) as the last pointer and value in N
                    bPlusTree->search_key_values[bPlusTree->num_search_keys] = Km;
                    bPlusTree->ptrs[bPlusTree->num_ptrs] = Nm;
                    bPlusTree->num_search_keys++;
                    bPlusTree->num_ptrs++;

                    // replace K' in parent(N) by N'.Km
                    int K3 = N2->search_key_values[0];
                    i = 0;
                    while (bPlusTree->parent->search_key_values[i] != K2) {
                        i++;
                    }
                    bPlusTree->parent->search_key_values[i] = K3;
                }
            } else {
                if (bPlusTree->is_leaf == false) {
                    // let m be such that N'.Pm is the last pointer in N'
                    int m = N2->num_search_keys - 1;
                    // remove (N'.K_m-1, N'.Pm) from N'
                    for (int k = m - 1; k < N2->num_search_keys; k++) {
                        N2->search_key_values[k] = N2->search_key_values[k + 1];
                    }
                    N2->num_search_keys--;
                    for (int k = m; k < N2->num_ptrs; k++) {
                        N2->ptrs[k] = N2->ptrs[k + 1];
                    }
                    N2->num_ptrs--;
                    N2->num_search_keys--;
                    // insert (N'.Pm, K') as the first pointer and value in N
                    int val_to_be_inserted = K2;
                    void *ptr_to_be_inserted = N2->ptrs[m - 1];

                    for (int k = 0; k < bPlusTree->num_search_keys; k++) {
                        int temp_val = bPlusTree->search_key_values[k];
                        void *temp_ptr = bPlusTree->ptrs[k];
                        bPlusTree->search_key_values[k] = val_to_be_inserted;
                        bPlusTree->ptrs[k] = ptr_to_be_inserted;
                        val_to_be_inserted = temp_val;
                        ptr_to_be_inserted = temp_ptr;
                    }
                    // replace K' in parent(N) by N'.Km-1
                    i = 0;
                    while (N2->search_key_values[i] != K2) {
                        i++;
                    }
                    BPlusTree * parent = bPlusTree->parent;
                    parent->search_key_values[i] = N2->search_key_values[m - 1];
                } else {
                    // let m be such that (N'.Pm, N'.Km) is the last pointer/value pair in N'
                    int m = N2->num_search_keys - 1;
                    int val_to_be_inserted = N2->search_key_values[m];
                    void *ptr_to_be_inserted = N2->ptrs[m];
                    int Km = val_to_be_inserted;
                    void *Pm = ptr_to_be_inserted;
                    // remove (sibling->ptrs[m], sibling->values[m]) from sibling
                    N2->num_search_keys--;
                    N2->num_ptrs--;

                    // Insert (sibling.P[m], sibling.K[m]) as the first ptr and val in N
                    for (int k = 0; k < bPlusTree->num_search_keys; k++) {
                        int temp_val = bPlusTree->search_key_values[k];
                        void *temp_ptr = bPlusTree->ptrs[k];
                        bPlusTree->search_key_values[k] = val_to_be_inserted;
                        bPlusTree->ptrs[k] = ptr_to_be_inserted;
                        val_to_be_inserted = temp_val;
                        ptr_to_be_inserted = temp_ptr;
                    }
                    // replace K' in parent(N) by N'.Km
                    i = 0;
                    while (bPlusTree->parent->search_key_values[i] != K2) {
                        i++;
                    }
                    bPlusTree->parent->search_key_values[i] = Km;
                }

            }
        }
    }
}



void remove_from_BPlusTree(BPlusTree ** bPlusTree, int value, void * ptr) {
    // 1. Let V be the search key value of the record, and Pr be ethe pointer to the record
    int V = value;
    BPlusTree *Pr = NULL;
    BPlusTree *C = *bPlusTree;
    while (C->is_leaf == false) {
        int i = 0;
        while (i < C->num_search_keys && value > C->search_key_values[i]) {
            i++;
        }
        if (i >= C->num_search_keys) {
            C = C->ptrs[C->num_ptrs];
        } else if (value == C->search_key_values[i]) {
            C = C->ptrs[i + 1];
        } else {
            C = C->ptrs[i];
        }
    }
    delete_entry(C, value, ptr);
}


Record * find_in_BPlusTree(BPlusTree ** bPlusTree, int value){
    BPlusTree * C = bPlusTree;
    while(C->is_leaf == false){
        int i = 0;
        while(i < C->num_search_keys && value > C->search_key_values[i]){
            i++;
        }
        if(i >= C->num_search_keys){
            C = C->ptrs[C->num_ptrs];
        }
        else if (value == C->search_key_values[i]){
            C = C->ptrs[i + 1];
        }
        else{
            C = C->ptrs[i];
        }
    }
    for(int i = 0; i < C->num_search_keys; i++){
        if(C->search_key_values[i] == value){
            return (Record *)C->ptrs[i];
        }
    }
    return NULL;
}

void insert_in_leaf(BPlusTree * bPlusTree, int K, Record * P){
        // Let Ki be the highest value in L that is less than or equal to K
        int i = 0;
        while(i < bPlusTree->num_search_keys && bPlusTree->search_key_values[i] <= K) {
            i++;
        }
        int val_to_be_inserted = K;
        Record * ptr_to_be_inserted = (Record *)P;
        for(int j = i; j <= bPlusTree->num_search_keys; j++) {
            int temp_val = bPlusTree->search_key_values[j];
            Record * temp_ptr = (Record *)bPlusTree->ptrs[j];
            //printf("val: %d\n", val_to_be_inserted);
            bPlusTree->search_key_values[j] = val_to_be_inserted;
            bPlusTree->ptrs[j] = (void *)ptr_to_be_inserted;
            val_to_be_inserted = temp_val;
            ptr_to_be_inserted = temp_ptr;
        }
        bPlusTree->num_search_keys++;
        bPlusTree->num_ptrs++;
}

void insert_into_BPlusTree(BPlusTree ** bplusTree, Record * record, int value) {
    /*
     * Postcondition; No node is overfull
     */

    // If tree is empty, create an empty leaf node L, which is also the root
    BPlusTree * C = *bplusTree;
    if(C->num_search_keys == 0){
        C->is_root = true;
    }
    else {
        // Set C to the leaf node in which the search-key value would appear
        while (C->is_leaf == false) {
            int i = 0;
            while (i < C->num_search_keys && value > C->search_key_values[i]) {
                i++;
            }
            if (i >= C->num_search_keys) {
                C = C->ptrs[C->num_search_keys];
            } else if (value == C->search_key_values[i]) {
                C = C->ptrs[i + 1];
            } else {
                C = C->ptrs[i];
            }
        }
    }

    if(C->num_search_keys < C->N - 1) {
        insert_in_leaf(C, value, record);
    }
    else{
        // L has N - 1 key values, split it
        split_leaf_node(C, value, record);
        while((*bplusTree)->is_root == false){
            *bplusTree = (*bplusTree)->parent;
        }
    }
}