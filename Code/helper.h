#ifndef HELPER_H
#define HELPER_H

#include <stdbool.h>
#include <stdio.h>

#define STRING_OF(__STRING_ARG__) #__STRING_ARG__

#define DEBUG

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__);
#else
#define LOG(...) ;
#endif

#define lex_error(__msg__, __text__)                                       \
    fprintf(stderr, "Error type A at Line %d: %s%s.\n", yylineno, __msg__, \
            __text__);

#define syn_error(__msg__)                                                 \
    fprintf(stderr, "Error type B at Line %d: %s%s.\n", yylineno, __msg__, \
            __text__);

#endif