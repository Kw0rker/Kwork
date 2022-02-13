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