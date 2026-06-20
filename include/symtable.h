#ifndef SYMTABLE_H
#define SYMTABLE_H

#define MAX_SYMBOLS 256
#include <stdbool.h>
typedef struct{
    char    name[64];
    double  value;
}Symbol;

typedef struct{
    Symbol  symbols[MAX_SYMBOLS];
    int     count;
}SymTable;

void    symtable_init(SymTable *st);
void    symtable_set(SymTable *st, const char *name, double value);
double  symtable_get(SymTable *st, const char *name);
bool     symtable_exists(SymTable *st, const char *name);

#endif