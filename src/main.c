#include <stdio.h>
#include "../include/lexer.h"
#include "../include/token.h"
#include <stdlib.h>

int main(void){
    Lexer l;
    lexer_init(&l, "let x = 5 + 3");

    Token t;
    while ((t = lexer_next(&l)).type != TOKEN_EOF) {
        if (t.type == TOKEN_NUMBER)
            printf("NUMBER(%.0f)\n", t.value);
        else if (t.type == TOKEN_IDENTIFIER)
            printf("IDENT(%s)\n", t.name);
        else
            printf("%s\n", token_type_to_str(t.type));
    }
    return 0;
}