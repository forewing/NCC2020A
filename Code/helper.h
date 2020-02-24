#ifndef HELPER_H
#define HELPER_H

#include <stdbool.h>
#include <stdio.h>

#define STRING_OF(__STRING_ARG__) #__STRING_ARG__

#define DEBUG

#define DEBUG_LEX
#define DEBUG_SYN

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__);
#else
#define LOG(...) ;
#endif

#ifdef DEBUG_LEX
#define LOG_LEX(...) LOG(__VA_ARGS__)
#else
#define LOG_LEX(...) ;
#endif

#ifdef DEBUG_SYN
#define LOG_SYN(...) LOG(__VA_ARGS__)
#else
#define LOG_SYN(...) ;
#endif

#endif