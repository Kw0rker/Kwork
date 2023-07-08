#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#ifndef DATA_STR_H
#include <data_str.h>
#define DATA_STR_H
//used to show the begining of the negative number
#define NEGNUMBER ';'
#endif
#define KWORK_PARAMS_H
#ifndef MAX_STR_SIZE
#define MAX_STR_SIZE 512
#endif
#define DOUBLE_STACK_CHAR -1337
typedef union{
    double a;
    struct 
    {
        int sign_byte:1;
        int exponent:11;
        long mantisa:52;
    };
}IEEE;
char *convertToPostfix(char*);
int isOperand(char);
int isOperator(char);
int Prec(int);
char *getToken(char **);
char *convertToPostfix(char *exp){
	STACKPTR stack = new_stack();
	 int i, k;
 
    // Create a stack of capacity
    // equal to expression size
	 char *result = malloc(MAX_STR_SIZE);
    if(!stack) // See if stack was created successfully
        return NULL ;
    
    char bracket = 0; //used to see if we are inside function arguemnt 
    for (i = 0, k = -1; exp[i]; ++i)
    {
        if(exp[i]=='.'){
            result[++k]='.';
            continue;
        }
        if (!strncmp(&exp[i],"CALL ",sizeof("CALL ")-1)){
            result[++k] = 'C';
            result[++k] = 'A';
            result[++k] = 'L';
            result[++k] = 'L';
            result[++k] = ' ';
            i+=sizeof("CALL ")-1;
            //next char must be begining of the function name
            while(exp[i]!='{'){
                //copy function name 
                result[++k]=exp[i++];
            }
            result[++k]=exp[i++];//copy bracket
            bracket++; //set bracket to 1
            while(bracket){ //loop while bracket is not 0
                if(exp[i]=='{')bracket++;
                else if(exp[i]=='}')bracket--;
                result[++k]=exp[i];
                i++;
            }
        }
        if(!strncmp(&exp[i],"{f}",sizeof("{f}")-1)){
            result[++k]='{';
            result[++k]='f';
            result[++k]='}';
            exp+=sizeof("{f}")-1;
        }
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


        if(exp[i]=='-'){
            int x=i;
            //go back for any whitespaces before the minus sign
            while(--x && exp[x]==' ');
            if(isOperator(exp[x])){
                //means that we have smth like 5*-1
                if(isdigit((int)exp[++i])){
                    result[++k]=NEGNUMBER;
                    while(isdigit((int)exp[i]) || exp[i]=='.'){
                        result[++k]=exp[i++];
                    }
                    i--;
                }
                else{
                    //some real shit happened
                    printf("incorrect expression %s\n",exp );
                    exit(EXIT_FAILURE);
                }

            //if true
            continue;
            }
        }        
         // If the scanned character is
        // an operand, add it to output.
        if (isOperand(exp[i]) || (exp[i]=='-'&&k==-1) ){
            while(isOperand(exp[i]) || exp[i]=='.'){
                result[++k] = exp[i++];
            }
            i--;
        }
         
        // If the scanned character is an
        // ‘(‘, push it to the stack.
        else if (exp[i] == '(')
        {
            //check if someone surrond negative number in paretecies 
            int x=i+1;
            while(exp[x]&&exp[x]==' ')x++;
            if(exp[x]=='-'){
                i=x;
                if(isdigit((int)exp[++i])){
                    result[++k]=NEGNUMBER;
                    while(isdigit((int)exp[i]) || exp[i]=='.'){
                        result[++k]=exp[i++];
                    }
                    if(exp[i]==')')i++;
                }
                else{
                    //some real shit happened
                    printf("incorrect expression %s\n",exp );
                    exit(EXIT_FAILURE);
                }
            }

            else push((int)exp[i],&stack);
        }
         
        // If the scanned character is an ‘)’,
        // pop and output from the stack
        // until an ‘(‘ is encountered.
        else if (exp[i] == ')')
        {
            while (!isEmpty(stack) && peek(&stack) != '('){

                int stack_val=pop(&stack);
                if(stack_val==DOUBLE_STACK_CHAR){
                   result[++k] = pop(&stack);
                   result[++k] = pop(&stack); 
                }
                else{result[++k] = stack_val;}

            }
            if (!isEmpty(stack) && peek(&stack) != '(')
                return NULL; // invalid expression            
            else
            {
                if(isEmpty(stack)){
                    fprintf(stderr, "WTF happened with expression debug!!!");
                    continue;
                }
                int stack_val=pop(&stack);
                if(stack_val==DOUBLE_STACK_CHAR)
                {
                pop(&stack);
                pop(&stack); 
                }
            }
        }
        else // an operator is encountered
        {
        	result[++k] = ' ';
            char comp = exp[i];
            char flag=0;
            //check for double operator such as <= , == , <= , >= , && , ||, ++, --
            if(exp[i+1]=='='||exp[i+1]=='!'||exp[i+1]=='<'||exp[i+1]=='>'||exp[i+1]=='&'||exp[i+1]=='|'||exp[i+1]=='+'||exp[i+1]=='-'){
                comp+=exp[i+1];
                flag=1;
                //if not ++ or -- reverse all steps made
                if((exp[i+1]=='+'&&exp[i]!='+')||(exp[i+1]=='-'&&exp[i]!='-')){
                    comp=exp[i];
                    flag=0;
                }
            }

            //todo when pop chech for DOUBLE_STACK_CHAR
            int stack_val;
            char val1=-1;
            char val2=-1;
            char flag1=1;
            while (!isEmpty(stack)&&flag1){

                    stack_val=pop(&stack);
                    if(stack_val==DOUBLE_STACK_CHAR)
                    {
                        val1=pop(&stack);
                        val2=pop(&stack);
                        stack_val=val1+val2;
                        if(Prec(comp)<=Prec(val1+val2)){
                            result[++k] = val1;
                            result[++k] = val2;
                        }
                        else
                        {
                            //if prec comp > prec of stack push values back
                            flag1=0;
                            push((int)val2,&stack);
                            push((int)val1,&stack);
                            push(stack_val,&stack);
                        }
                    }
                    else
                    {
                        if (Prec(comp)<=Prec(stack_val)){
                            result[++k] = stack_val;
                        }
                        else{
                            flag1=0;
                            push(stack_val,&stack);
                        }
                    }
                    
            }
                
           
            if(flag)push((int)exp[i+1],&stack);
            push((int)exp[i],&stack);
            if(flag){
            push(DOUBLE_STACK_CHAR,&stack); //indicate that next two stack entries to be treated as one
            i++;
            }
        }
 
    }
 
    while (!isEmpty(stack)){
        result[++k]=' ';
         int stack_val=pop(&stack);
         if(stack_val==DOUBLE_STACK_CHAR){
            result[++k] = pop(&stack);
            result[++k] = pop(&stack); 
        }
        else{
            result[++k] = stack_val;
        }
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
           ch=='&' ||
           ch=='|' ||
           ch=='#';
}
int Prec(int c){
	 switch (c)
    {
    case '+'+'+':
    case '-'+'-':
        return 0;    
        
    case '+':
    case '-':
        return 2;
 
    case '*':
    case '/':
    case '%':
    case '!':    
        return 1;
 
    // case '^': colides with != should i fix it ???
    //     return 6;
    case '@':
    case '#':
        return 3;
    case '<':
    case '>':
    case '<'+'=':
    case '>'+'=':    
        return 4;
    case '='+'=':
    case '!'+'=':    
        return 5;
    case '&'+'&':
    case '|'+'|':
    case '<'+'<':
    case '>'+'>':     
        return 6;                 
    }
    return -1;
}
int isDoubleOperator(char c){
    //only returns if c is potential part of the double operator like ++ or <= or || etc
    return c=='='||c=='!'||c=='<'||c=='>'||c=='&'||c=='|'||c=='+'||c=='-';
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

//removes whitespaces suraunding the operands
char remove_ws_fr_exp(char *orig,char *new){
    //todo add chech for strings. 
    char *copy = orig;
    char flag_inside_char_or_string = 0;
    while(orig[0]!='\0'){
        if(!strncmp(orig,"CALL ",sizeof("CALL ")-1)){
            new++[0]='C';
            new++[0]='A';
            new++[0]='L';
            new++[0]='L';
            new++[0]=' '; 

            orig+=sizeof("CALL ")-1;
        }
        else if(orig[0]=='\'' || orig[0]=='"'){
            (new++)[0]=orig[0];
            orig++;
            if (!flag_inside_char_or_string)flag_inside_char_or_string=1;
            else{
                flag_inside_char_or_string=0;
            }
        }
        else if(flag_inside_char_or_string || isOperand(orig[0])||isOperator(orig[0]) || orig[0]=='{'|| orig[0]=='}'
                || orig[0]=='('|| orig[0]==')'  || orig[0]=='['|| orig[0]==']'|| orig[0]==','|| orig[0]=='.'
            ){
            (new++)[0]=orig[0];
            orig++;
        }
        else if(!isspace(orig[0])){
            fprintf(stderr, "WTF is (%c) in Expression %s\n",orig[0],copy);
            orig++;
        }
        while(orig[0]!='\0' && isspace((int)orig[0])){
            orig++;
        }

    }
}
//checks if data was sent to stdin
int inputAvailable()  
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

#ifndef D_MAX
#define D_MAX 1000
#endif
#define D_MAP_INIT double dStack[D_MAX];
#define store_double(adress,value)(dStack[(adress)%D_MAX]=(value),dStack[(adress)%D_MAX]);
#define load_double(adress)(dStack[(adress)%D_MAX]);

#define perform_float_operation(var,oper,var2)((*(double *)&var)oper(*(double *)&var2) );
