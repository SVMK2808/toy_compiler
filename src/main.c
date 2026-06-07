#include <stdio.h>
#include "lexer.h"
#include "token.h"

int main(){
    Lexer l;
    lexer_init(&l, "1 + 2 * (3 - 4)");

    Token t;
    while((t = lexer_next(&l)).type != TOKEN_EOF){
        if(t.type == TOKEN_NUMBER)
            printf("NUMBER: %.0f\n", t.value);
        else
            printf("TOKEN: %d \n", t.type);
    }
    return 0;
}