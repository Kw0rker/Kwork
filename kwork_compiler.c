#include <stdio.h>
#include <stdlib.h>
#ifndef MAX_CODE_SIZE
#define MAX_CODE_SIZE 10000
#endif
#ifndef MAX_STATIC_SIZE
#define MAX_STATIC_SIZE 500 //max static space for each function, containging kworker assembly instructions and variables
#ifndef MAX_DYNAMIC_SPACE
#define MAX_DYNAMIC_SPACE value
#endif
#endif
#define input "READ "
int flags[MAX_CODE_SIZE];
char *KWAC[MAX_CODE_SIZE];
/*
@symbol is ansi representation of var name (var name is one char long as defined in standart)
@type {
		C -> constant
		L -> line_number
		V -> variable
	  }
@location adress in the memory	  
*/

struct table_entry
{
	int symbol;
	char type;
	long location;	
}
typedef struct table_entry TABLE_ENTRY

typedef TABLE_ENTRY *TABLE_ENTRY_PTR

TABLE_ENTRY_PTR symbolTable[MAX_CODE_SIZE];
/*
precompiles code
with defines and includes
*/
void precompile(FILE *);

/*
compiles code to KWAC (kworker assembly code)
*/
void compile(FILE *);
void first_compile(FILE *);
void second_compile()

void int main(int argc, char const *argv[])
{	
	if(argc<=1){
	  perror("No source file provided\n");	
	  return -1;
	}
	FILE *file = NULL;
	file = fopen(argv[1],"r");
	if(file==NULL){
		perror("No file found\n");	
	    return -1;
	}
	compile(file);
	memset(flags,-1,sizeof(flags));
	return 0;
}
void compile(FILE *file){
	first_compile(file);
	second_compile();
}
void first_compile(FILE *file){
	char line[100];
	int line_n=-1;
	char operator[20];
	char operand[20];
	while(!feof(file))
	{

	fscanf("%100s\n",line);
	operator = strtok(line," ");
	operand = line;
	line_n++;
	if(operator[0]=='/' && operator[1] == '/'){
		//thats a comment ignore
		line_n--;
	}
	else if(!strcmp(operator,"input")){
		char found = 0;
		for(int a=0;a<line_n;a++){
			//search for var in symbol table, if present make reference to its location
			//else create a new reference
			if(symbolTable[a]!=NULL && symbolTable[a]->symbol==operand[0]){
				char *KWAC_COMMAND[40];
				strncpy(KWAC_COMMAND,input,sizeof(KWAC_COMMAND));
				strncpy(KWAC_COMMAND,symbolTable[a]->location,sizeof(KWAC_COMMAND));
				KWAC[line_n] = KWAC_COMMAND;
				found=1;
			}
		}
		//if no reference yet, create a new one
		if(!found){

		}
	}



	}
}