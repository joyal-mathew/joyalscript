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

RESULT lexer_integer(Lexer *lexer) {
    u64 last_value;

    lexer->token_type = tt_Integer;
    lexer->integer = 0;

    while (isdigit(peek(lexer))) {
        HRESULT mult_result = ULongLongMult(lexer->integer, 10, &lexer->integer);
        HRESULT add_result = ULongLongAdd(lexer->integer, next(lexer) - '0', &lexer->integer);

        if (mult_result != S_OK || add_result != S_OK) {
            DISPATCH_ERROR_FMT(lexer->context, lexer->line, "Overflowing integer literal `%llu...`", last_value);
            return TRUE;
        }

        last_value = lexer->integer;
    }

    return FALSE;
}

RESULT lexer_operator(Lexer *lexer) {
    char op_str[MAX_OPERATOR_LEN + 1] = { 0 };
    u8 i = 0;

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
        DISPATCH_ERROR_FMT(lexer->context, lexer->line, "Undefined operator `%s`", op);
        return TRUE;
    }

    lexer->token_type = tt_Operator;
    lexer->operator_type = op;
    lexer->index = op_index;

    return FALSE;
}

RESULT lexer_init(Lexer *lexer) {
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

RESULT lexer_next(Lexer *lexer) {
    while (isspace(peek(lexer))) {
        if (next(lexer) == '\n') {
            lexer->line += 1;
        }
    }

    switch (peek(lexer)) {
        case 0:
            lexer->token_type = tt_Eof;
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

void token_to_str(Lexer *lexer) {
    switch (lexer->token_type) {
        case tt_Integer:
            sprintf(lexer->token_str, "%llu", lexer->integer);
            break;
        case tt_Operator:
            switch (lexer->operator_type) {
                case op_Addition:           strcpy(lexer->token_str, "+");  break;
                case op_Subtraction:        strcpy(lexer->token_str, "-");  break;
                case op_Multiplication:     strcpy(lexer->token_str, "*");  break;
                case op_Division:           strcpy(lexer->token_str, "/");  break;
                case op_OpenParenthesis:    strcpy(lexer->token_str, "(");  break;
                case op_CloseParenthesis:   strcpy(lexer->token_str, ")");  break;
                default: break;
            }

            break;
        case tt_Eof:
            strcpy(lexer->token_str, "EOF");
            break;
    }
}