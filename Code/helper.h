#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>

#define STRING_OF(__STRING_ARG__) #__STRING_ARG__

#define DEBUG

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__);
#else
#define LOG(...) ;
#endif

#endif