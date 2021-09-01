#include <data_str.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef MAX_STR_SIZE
#define MAX_STR_SIZE 100
#endif
char *convertToPostfix(char*);
int isOperator(char);
int Prec(char);
char *convertToPostfix(char *string){
	if (sizeof string > MAX_STR_SIZE)
	{
		perror("Infix string is too big\n\0");
		return NULL;
	}
	STACKPTR stack = new_stack();
	char *postfix = malloc(MAX_STR_SIZE);
	//postfix[sizeof(string)-1]='\0';
	push((int)'(',&stack);
	//postfix[sizeof(string)-2]=')';
	while(!string){
		char ch = string++;
		if (isdigit(ch)){
			++postfix=ch;
		}
		else if(ch=='('){
			push((int)ch,&stack);
		}
		else if(isOperator(ch)){
			while(!isEmpty(stack)&&Prec(ch)<=Prec((char)peek(&stack))){
				++postfix=(char)pop(&stack);
			}
			push((int)ch,&stack);
		}
		else if(ch==')'){
			while(!isEmpty(stack)&&peek(&stack)!='('){
				++postfix=(char)pop(&stack);
			}
		}
	}
	++postfix=')';
	++postfix='\0';
}
int isOperator(char ch){
	return 		ch=='+'||
				ch=='-'||
				ch=='*'||	
				ch=='/'||
				ch=='^'||
				ch=='%';
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
    }
    return -1;
}