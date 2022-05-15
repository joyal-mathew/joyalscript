#include <intsafe.h>
#include <ctype.h>
#include <string.h>
#include "lexing.h"
#include "context.h"

#define MAX_OPERATOR_LEN 8

bool is_operator(char c) {
    return  c == '+' ||
            c == '-' ||
            c == '*' ||
            c == '/' ||
            c == '(' ||
            c == ')' ;
}

char peek(Lexer *lexer) {
    return lexer->context->program[lexer->index];
}

char next(Lexer *lexer) {
    return lexer->context->program[lexer->index++];
}

WARN_UNUSED
bool lexer_integer(Lexer *lexer) {
    lexer->type = tt_Integer;
    lexer->integer = 0;

    while (isdigit(peek(lexer))) {
        HRESULT mult_result = ULongLongMult(lexer->integer, 10, &lexer->integer);
        HRESULT add_result = ULongLongAdd(lexer->integer, next(lexer) - '0', &lexer->integer);

        if (mult_result != S_OK || add_result != S_OK) {
            return dispatch_error(lexer->context, "Overflowing integer literal", lexer->line);
        }
    }

    return FALSE;
}

WARN_UNUSED
bool lexer_operator(Lexer *lexer) {
    char op_str[MAX_OPERATOR_LEN + 1] = { 0 };
    u64 i = 0;

    bool op_found = FALSE;
    u64 op_index;
    u64 op;

    while (is_operator(peek(lexer))) {
        if (i >= MAX_OPERATOR_LEN + 1) {
            break;
        }

        op_str[i++] = next(lexer);

        if (!hashmap_get(&lexer->operator_map, op_str, &op)) {
            op_found = TRUE;
            op_index = lexer->index;
        }
    }

    if (!op_found) {
        return dispatch_error(lexer->context, "Undefined operator", lexer->line);
    }

    lexer->type = tt_Operator;
    lexer->operator_type = op;
    lexer->index = op_index;

    return FALSE;
}

bool lexer_init(Lexer *lexer) {
    lexer->index = 0;
    lexer->line = 1;

    hashmap_init(&lexer->operator_map);

    hashmap_put(&lexer->operator_map, "+", op_Addition);
    hashmap_put(&lexer->operator_map, "-", op_Subtraction);
    hashmap_put(&lexer->operator_map, "*", op_Multiplication);
    hashmap_put(&lexer->operator_map, "/", op_Division);
    hashmap_put(&lexer->operator_map, "(", op_OpenParenthesis);
    hashmap_put(&lexer->operator_map, ")", op_CloseParenthesis);

    return lexer_next(lexer);
}

void lexer_deinit(Lexer *lexer) {
    hashmap_deinit(&lexer->operator_map);
}

bool lexer_next(Lexer *lexer) {
    while (isspace(peek(lexer))) {
        if (next(lexer) == '\n') {
            lexer->line += 1;
        }
    }

    switch (peek(lexer)) {
        case 0:
            lexer->type = tt_Eof;
            break;
        default:
            if (isdigit(peek(lexer))) {
                CHECK(lexer_integer(lexer));
            }
            else if (is_operator(peek(lexer))) {
                CHECK(lexer_operator(lexer));
            }
    }

    return FALSE;
}

void print_token(Lexer *lexer) {
    switch (lexer->type) {
        case tt_Integer:
            printf("[INT %llu]", lexer->integer);
            break;
        case tt_Operator:
            switch (lexer->operator_type) {
                case op_Addition:           printf("[OP add]"); break;
                case op_Subtraction:        printf("[OP sub]"); break;
                case op_Multiplication:     printf("[OP mul]"); break;
                case op_Division:           printf("[OP div]"); break;
                case op_OpenParenthesis:    printf("[OP (]");   break;
                case op_CloseParenthesis:   printf("[OP )]");   break;
            }

            break;
        case tt_Eof:
            printf("[EOF]");
            break;
    }
}
