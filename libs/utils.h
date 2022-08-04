#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifndef DATA_STR_H
#include <data_str.h>
#define DATA_STR_H
#endif
#define KWORK_PARAMS_H
#ifndef MAX_STR_SIZE
#define MAX_STR_SIZE 100
#endif
char *convertToPostfix(char*);
int isOperand(char);
int isOperator(char);
int Prec(char);
char *getToken(char **);
char *convertToPostfix(char *exp){
	STACKPTR stack = new_stack();
	 int i, k;
 
    // Create a stack of capacity
    // equal to expression size
	 char *result = malloc(MAX_STR_SIZE);
    if(!stack) // See if stack was created successfully
        return NULL ;
 
    for (i = 0, k = -1; exp[i]; ++i)
    {
        if(exp[i]=='{'){
            int bracket = 0; 
            //while bracket is not 0
            do{
                if(exp[i]=='{')bracket++;
                else if (exp[i]=='}')bracket--;
                result[++k] = exp[i++];
            }while(bracket);
            //result[++k] = exp[i++];
        }
        if (exp[i]=='@'||exp[i]=='#'||exp[i]=='!'){
                push((int)' ',&stack);
        }
        if(exp[i]=='['){
            while(exp[i]!=']'){
                result[++k] = exp[i++];
            }
            result[++k] = exp[i++];
            // while(exp[i]==' ' && exp[i]!='\0')i++;
        }
        if(exp[i]=='\''){

            do{
                result[++k] = exp[i++];
            } while(exp[i]!='\'');
            result[++k] = exp[i++];
            // while(exp[i]==' ' && exp[i]!='\0')i++;
        }
         // If the scanned character is
        // an operand, add it to output.
        if (isOperand(exp[i]) || (exp[i]=='-'&&k==-1) )
            result[++k] = exp[i];
         
        // If the scanned character is an
        // ‘(‘, push it to the stack.
        else if (exp[i] == '(')
            push((int)exp[i],&stack);
         
        // If the scanned character is an ‘)’,
        // pop and output from the stack
        // until an ‘(‘ is encountered.
        else if (exp[i] == ')')
        {
            while (!isEmpty(stack) && peek(&stack) != '(')
                result[++k] = pop(&stack);
            if (!isEmpty(stack) && peek(&stack) != '(')
                return NULL; // invalid expression            
            else
                pop(&stack);
        }
        else // an operator is encountered
        {
        	result[++k] = ' ';
            while (!isEmpty(stack) &&
                 Prec(exp[i]) <= Prec(peek(&stack)))
                result[++k] = pop(&stack);
            int t=0;
            if(exp[i+1]=='='||exp[i+1]=='!'||exp[i+1]=='<'||exp[i+1]=='>'){
                push((int)exp[i+1],&stack);
                t=1;
            }
            push((int)exp[i],&stack);
            i+=t;
        }
 
    }
 
    // pop all the operators from the stack
    while (!isEmpty(stack)){
        result[++k]=' ';
        result[++k] = pop(&stack );
    }
 
    result[++k] = '\0';
	free(stack);
	return result;
}

int isOperand(char ch){
	 return (ch >= 'a' && ch <= 'z') ||
           	(ch >= 'A' && ch <= 'Z') ||
           	isdigit((int)ch) || ch=='\'';
}
int isOperator(char ch){
	return ch=='+' ||
		   ch=='-' ||
		   ch=='/' ||
		   ch=='*' ||
		   ch=='%' ||
		   ch=='^' ||
           ch=='@' ||
           ch=='<' ||
           ch=='>' ||
           ch=='!' ||
           ch=='=' ||
           ch=='#';
}
int Prec(char c){
	 switch (c)
    {
    case '+':
    case '-':
        return 1;
 
    case '*':
    case '/':
        return 2;
 
    case '^':
    case '%':
        return 3;
    case '@':
    case '#':
    case '!':
        return 4;    
    }
    return -1;
}
char *getToken(char **str){
	char *temp = (*str);
	int toWrtie = 0;
	while(temp[0]!='\0' && !isOperand(temp[0]) && temp[0]!=')' && temp[0]!=')'){
		temp++;
		toWrtie++;
	}
	char *result  = malloc(toWrtie+1);
	snprintf(result,toWrtie+1,"%s",(*str));
	(*str)+=toWrtie;
	return result;

}
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ( (*s++ = *d++) );
}
char array_is_empty(int a[],int size){
    int isEmpty=1;
    for (int i = 0; i < size; ++i)
    {
        if(a[i]!=0){
            isEmpty=0;
            break;
        }
    }
    return isEmpty;
}
