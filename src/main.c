#include <stdio.h>
#include "../include/lexer.h"
#include "../include/token.h"

int main(){
    Lexer l;
    lexer_init(&l, "1 + 2 * (3 - 4)");

    Token t;
    while((t = lexer_next(&l)).type != TOKEN_EOF){
        if(t.type == TOKEN_NUMBER)
            printf("NUMBER(%.0f)\n", t.value);
        else
            printf("%s\n", token_type_to_str(t.type));
    }
    return 0;
}