#include "../include/symtable.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

int linear_search(SymTable *st, const char *name){
    for(int i = 0; i < st -> count; i++){
        if(strcmp(st->symbols[i].name, name) == 0){
            return i;
        }
    }
    return -1;
}

void symtable_init(SymTable *st){
    st -> count = 0;
}

void symtable_set(SymTable *st, const char *name, double value){
    
    int index = linear_search(st, name);
    if(index >= 0){ // element already exists
        st -> symbols[index].value = value;
    }else{ // new entry 
        st -> symbols[st -> count].value = value;
        strcpy(st -> symbols[st -> count].name, name);
        st -> count++;
    }
}

double symtable_get(SymTable *st, const char *name){
    int index = linear_search(st, name);
    if(index >= 0){
        return st -> symbols[index].value;
    }else return 0.0;
}

bool symtable_exists(SymTable *st, const char *name){
    int index = linear_search(st, name);
    return (index >= 0) ? true : false;
}