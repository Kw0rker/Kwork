#include <kwork_params.h>
#include <assert.h>
#ifndef MAX_CODE_SIZE
#define MAX_CODE_SIZE 10000
#endif
#ifndef MAX_STATIC_SIZE
#define MAX_STATIC_SIZE 500 //max static space for each function, containging kworker assembly instructions and variables
#endif
#define input "READ \0"

int flags[MAX_CODE_SIZE];
char *KWAC[MAX_CODE_SIZE];

//pass pointer to struct
#define Table_EntryToString(entry)												\
			printf("Struct table_entry{\nint const_value %d\nint symbol %d;\nchar type %c;\nlong location %ld;\nchar fucn_name %s;\n}\n",entry->const_value, entry->symbol,entry->type,entry->location,entry->fucn_name)		\

/*
use in main function only ! as it depends on local variables
@comp is comparator == or <= etc
*/
#define getVars(comp) char *f;\
				char *s;\
				f = strtok(operand,#comp);\
				s = strtok(NULL,#comp);\
				if(isdigit((int)f[0])) v1 = find_entry('C',atoi(f),fucn_name,line_n,function_pointer);\
				else v1 = find_entry('V',f[0],fucn_name,line_n,function_pointer);\
				if(isdigit((int)s[0]))  v2 = find_entry('C',atoi(s),fucn_name,line_n,function_pointer);\
				else v2 = find_entry('V',s[0],fucn_name,line_n,function_pointer)\
			

#define UPDATE_IF_BLOCKS(offset)if(returns[total_comands-offset]!=NULL){\
								TABLE_ENTRY_PTR assm = symbolTable[total_comands-offset];\
				 				char new_addres[30];\
								sprintf(new_addres,"%ld",assm->location);\
								assert(assm!=NULL);\
									while(!isEmpty(returns[total_comands-offset])){\
										int adress = pop(&(returns[total_comands-offset]));\
										char *new_command = strcat(symbolTable[adress]->fucn_name,new_addres);\
										strcpy(symbolTable[adress]->fucn_name,new_command);\
									}\
								}\
/*
@const value used for constants only
@symbol is ansi representation of var name (var name is one char long as defined in standart)
@type {
		C -> constant
		L -> line_number
		V -> variable
	  }
@location adress in the memory	 
@fucn_name servers as frame for local vars of function, but if type os L serves as LINE OF CODE 
*/

struct table_entry
{
	int const_value;
	int symbol;
	char type;
	long location;
	char fucn_name[50];	
};
typedef struct table_entry TABLE_ENTRY;

typedef TABLE_ENTRY *TABLE_ENTRY_PTR;

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
void second_compile();
TABLE_ENTRY_PTR find_entry(char ,int ,char *,int ,int );
TABLE_ENTRY_PTR create_new(char ,int ,char *,long);

int main(int argc, char const *argv[])
{	
	if(argc<1){
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
	STACKPTR stack = new_stack();
	char *line = malloc(100);
	STACKPTR returns[MAX_CODE_SIZE];
	int function_pointer;
	int line_n=-1;
	int local_comands =-8 ;//number of commands in function and i have no fucking clue why it has to be  -8 lmao im fucking retard but it works this way
	int total_vars=0; //total ammount of vars declareted
	int total_comands = 0;
	char *operator = malloc(20);
	char *operand = malloc(20);
	char *fucn_name = malloc(50);
	fucn_name = "main"; //main by defual
	while(!feof(file))
	{
	fgets(line,100,file);
	//fscanf(file,"%100s\n",line);
	//printf("%s\n",line );
	operator = strtok(line," ");
	operand = strtok(NULL," ");
	//if(operand == NULL)continue;
	line_n++;
	if(operator[0]=='/' && operator[1] == '/'){
		line_n--;
	}
	else if(!strcmp(operator,"function")){
		//strcpy(fucn_name,operand);
		//fucn_name=operand;
		TABLE_ENTRY_PTR func = malloc(sizeof(TABLE_ENTRY));
		func->type='L';
		if(!strcmp(operand,"main\r\n")){
			function_pointer=0;
			strcpy(func->fucn_name,"&0\nMAIN");
		}
		else {
			function_pointer = function_pointer + MAX_STATIC_SIZE + 1;
			strcpy(func->fucn_name,"FUCN");
		}
		func->location=function_pointer;
		symbolTable[total_comands++] = func;
	}
	else if(!strcmp(operator,"end")){
		line_n=-1;
		while(!feof(file)){
			fscanf(file,"%100s\n",line);

			operator = strtok(line," ");
			operand = strtok(NULL," ");
			if(!strcmp(operator,"function")){
				fucn_name=operand;
				TABLE_ENTRY_PTR func = malloc(sizeof(TABLE_ENTRY));
				func->type='L';
				if(!strcmp(operand,"main\r\n")){
					function_pointer=0;
					strcpy(func->fucn_name,"MAIN");
				}
				else {
				function_pointer = function_pointer + MAX_STATIC_SIZE + 1;
				strcpy(func->fucn_name,"FUNC");
				}
			func->location=function_pointer;
			symbolTable[total_comands++] = func;
			break; //we found next function now repead
			}
		}
		//todo do while search for next function in file
	}
	else if(!strcmp(operator,"input")){
		TABLE_ENTRY_PTR table_entry =find_entry('V',operand[0],fucn_name,line_n,function_pointer);
		symbolTable[MAX_CODE_SIZE-(++total_vars)] = table_entry;
		char KWAC_COMMAND [50];
		sprintf(KWAC_COMMAND,"%s %ld",input,table_entry->location);
		// strncat(KWAC_COMMAND,input,sizeof(KWAC_COMMAND)-1);
		// char p[] = {table_entry->location + '0','\0'};
		// strncat(KWAC_COMMAND,p,sizeof(KWAC_COMMAND));
		TABLE_ENTRY_PTR new = malloc(sizeof(TABLE_ENTRY));
		new->type = 'L';
		new->location = function_pointer + local_comands; //add fucntion base pointer with ammount of kwac comands

		strcpy(new->fucn_name,KWAC_COMMAND); //copy complete input comand to the entry 
		symbolTable[total_comands++] = new; //and save it
		local_comands++;
		UPDATE_IF_BLOCKS(1)
		memset(KWAC_COMMAND,0,sizeof(KWAC_COMMAND));
		}
		else if(!strcmp(operator,"if")){
			TABLE_ENTRY_PTR v1;
			TABLE_ENTRY_PTR v2;
			char command[30];
			//todo change if blocs as they dont support const bigger than 9 
			char comparator[3];
			int x=0;
			int y =0;
			//while end of the string is not encountered
			//get comparator from the operand
			#ifdef _DEBUG
			printf ("operand ->%s\n",operand);
			#endif
			while(operand[x]!='\0' && y<3){
				if(operand[x]=='=' || operand[x]=='<' || operand[x]=='>')comparator[y++] = operand[x];
				x++;
			}
			comparator[y] = '\0';
			#ifdef _DEBUG
			printf("comparator -> %s\n",comparator);
			#endif
			if(!strcmp(comparator,"==")){
				getVars(==);
				sprintf(command,"LOAD %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCHZERO %d",function_pointer+local_comands+2); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+3] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				push(total_comands,&stack); 
				UPDATE_IF_BLOCKS(4) 


			}
			else if(!strcmp(comparator,"<=")){
				getVars(<=);
				//v1 -v2
				sprintf(command,"LOAD %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//branzero and branch neg jump over branch statement 
				sprintf(command,"BRANCHZERO %d",function_pointer+local_comands+3); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCHZNEG %d",function_pointer+local_comands+3); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+4] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				push(total_comands,&stack);
				UPDATE_IF_BLOCKS(5)    

			}
			else if(!strcmp(comparator,">=")){
				getVars(>=);
				//v2 -v1

				sprintf(command,"LOAD %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//branzero and branch neg jump over branch statement 
				sprintf(command,"BRANCHZERO %d",function_pointer+local_comands+3); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCHZNEG %d",function_pointer+local_comands+3); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+4] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				push(total_comands,&stack);
				UPDATE_IF_BLOCKS(5)  
			}

			else if(!strcmp(comparator,">")){
				getVars(>);
				sprintf(command,"LOAD %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//branzero and branch neg jump over branch statement 
				
				sprintf(command,"BRANCHZNEG %d",function_pointer+local_comands+2); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+local_comands+2] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				push(total_comands,&stack);   
				UPDATE_IF_BLOCKS(4)   

			}
			else if(!strcmp(comparator,"<")){
				getVars(<);
				sprintf(command,"LOAD %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//branzero and branch neg jump over branch statement 
				
				sprintf(command,"BRANCHZNEG %d",function_pointer+local_comands+2); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+3] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				push(total_comands,&stack);
				UPDATE_IF_BLOCKS(4)      

			}
			symbolTable[MAX_CODE_SIZE - (++total_vars)] = v1;
			symbolTable[MAX_CODE_SIZE - (++total_vars)] = v2;
		}
		else if(line[0]=='}'){
			printf("bracket found\n");
			//we found clossing bracket now pop 
			if(!isEmpty(stack)){
				int adress = pop(&stack)-1;
				//if no stack create one
				//used to store addresses of all jump statments that exit if or while statements
				//stores in adress of next command
				if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
				push(adress,&(returns[total_comands]));
			}
			else {
				perror("extra bracket found");
				//line_n--;
			}
		}




		else{
			//dont count number of commands if operation is not defined in compiler
			printf("command -> %s is not defined\n" ,line);
			//line_n--;
		}



	}
	fclose(file);
	// free(operator);
	// free(operand);
	// free(fucn_name);
	// free(line);
}

void compileFunction(FILE *file,char *fucn_name){

}
void second_compile(){

	// todo resolve udefined reference


	//print to file

	FILE *file = NULL;
	file = fopen(DEFNAME,"w");
	if(file==NULL){
		perror("could not create kworker assembly file\n");
	}
	int last_adr = 0;
	for(int a=0;a<MAX_CODE_SIZE;a++){
		if(symbolTable[a]!=NULL){
			#ifdef _DEBUG
			Table_EntryToString(symbolTable[a]);
			#endif
			switch (symbolTable[a]->type){
				case'C':
				//#location
				//number
				fprintf(file,"#%ld\n%d\n",symbolTable[a]->location,(symbolTable[a]->const_value) );
				break;
				//case 'V':
				//do nothing as we dont need to alloc memory in kernel
				//break;
				case 'L':
				//if adress is different with preivus command we place & and specify andress
				//& - sets kernel instruction input adress by value
				if( (symbolTable[a]->location - last_adr ) >1){
					fprintf(file,"&%ld\n%s\n",symbolTable[a]->location, symbolTable[a]->fucn_name);
					last_adr = symbolTable[a]->location;
				}
				else {
					fprintf(file,"%s\n",symbolTable[a]->fucn_name);
					last_adr++;
				}
				break;

			}
		}
	}
	fclose(file);

}

TABLE_ENTRY_PTR find_entry(char type,int data,char *fucn_name,int line_n,int function_pointer){
	switch(type){
		case'C':
		for(int x=1;x<=line_n;x++){ //todo search from the top of array as there vars are stored
			int a =MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->const_value==data &&(!strcmp(symbolTable[a]->fucn_name,fucn_name))){
				return symbolTable[a]; // found
				break;
			}
		}
		break;

		case'V':
		for(int x=1;x<=line_n;x++){ //todo search from the top of array as there vars are stored
			int a = MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->symbol==data &&(!strcmp(symbolTable[a]->fucn_name,fucn_name))){
				return symbolTable[a]; // found
			}
		}
		break;
	}
	//if not found create new
	TABLE_ENTRY_PTR new = malloc(sizeof (TABLE_ENTRY));
	new->location=function_pointer+MAX_STATIC_SIZE-line_n;
	new->type=type;
	new->const_value=data;
	new->symbol=data;
	strcpy(new->fucn_name,fucn_name);
	return new;

}
TABLE_ENTRY_PTR create_new(char type,int data,char *fucn_name,long location){
	TABLE_ENTRY_PTR new = malloc(sizeof (TABLE_ENTRY));
	new->location=location;
	new->type=type;
	new->const_value=data;
	new->symbol=data;
	strcpy(new->fucn_name,fucn_name);
	return new;
}