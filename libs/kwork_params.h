#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <syscalls.h>
#include <fast_math.h>
#include <utils.h>
#include<fcntl.h> 
#ifndef DATA_STR_H
#include <data_str.h>
#define DATA_STR_H
#endif
#ifndef DEFNAME
#define DEFNAME "out.kw"
#endif
#ifndef ASEM_NAME
#define ASEM_NAME "input.kwac"
#endif
#define MEM_SIZE 10000
#ifndef MAX_THREAD_POOL
#define MAX_THREAD_POOL 1000
#endif
#ifndef MIN_THREAD_TIME
#define MIN_THREAD_TIME 150 //in nanosecs
#endif
#ifndef MAX_THREAD_TIME
#define MAX_THREAD_TIME 450 //in nanosecs
#endif
#define CONVERT_THREAD_POINTER_TO_ID(x) x % MAX_THREAD_POOL
#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE 1000
#endif
#ifndef MAX_CONST_POOL
#define MAX_CONST_POOL 254
#endif
#define ESP_ADR (MEM_SIZE - MAX_CONST_POOL)
#ifndef MAX_NESTED_LOOPS_VALUE
#define MAX_NESTED_LOOPS_VALUE 40
#endif
#ifndef MAX_ARGS
#define MAX_ARGS 35
#endif
#ifndef MAX_IF_EXPP_LEN
#define MAX_IF_EXPP_LEN 228
#endif
#ifndef MAX_LIBS
#define MAX_LIBS 212
#endif
#ifndef MAX_LIB_FUNCTIONS
#define MAX_LIB_FUNCTIONS 424
#endif
#if defined (REL_KWH_PH) && !defined (ABS_KWH_PH)
#define LIB_PATH REL_KWH_PH
#elif defined(ABS_KWH_PH)
#define LIB_PATH ABS_KWH_PH
#else
#define REL_KWH_PH "./kwh_libs"
#define LIB_PATH REL_KWH_PH
#endif
#ifndef FLOAT_SUPPORT
//#define FLOAT_SUPPORT 1
#endif
#define RANDOM_V 1941287553
#ifndef MAX_F_ARGUMENTS
#define MAX_F_ARGUMENTS 123
#endif
#define AB_FUNC_MAX MAX_LIB_FUNCTIONS*2

// macros bellow
///////////////////////


#define CREATE_INSTRUCTION(command,adress)\
char com[100];\
sprintf(com,"%s %d",#command,adress);\
symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));\

#define FIND_OR_CREATE(X,type)\
int ad = find_location (type,X,fucn_name,total_vars);\
if(ad<0){\
	TABLE_ENTRY_PTR CONST = create_new(type,X,fucn_name,MAX_CODE_SIZE - total_const++);\
	ad = MAX_CODE_SIZE-(++total_vars);\
	symbolTable[ad]=CONST;}\


