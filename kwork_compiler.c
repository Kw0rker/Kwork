#include <kwork_params.h>
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
			printf("Struct table_entry{\nint symbol %d;\nchar type %c;\nlong location %ld;\nchar fucn_name %s;\n}\n",entry->symbol,entry->type,entry->location,entry->fucn_name)		\

/*
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
	char *line = malloc(100);
	int function_pointer;
	int line_n=-1;
	int total_vars=0; //total ammount of vars declareted
	int total_comands = 0;
	int var_number = 0; //amount of var declareted
	char *operator = malloc(20);
	char *operand = malloc(20);
	char *fucn_name = malloc(50);
	fucn_name = "main"; //main by defual
	while(!feof(file))
	{
	fgets(line,sizeof (line),file);
	//fscanf(file,"%100s\n",line);
	printf("%s\n",line );
	operator = strtok(line," ");
	operand = strtok(NULL," ");
	if(operand == NULL)continue;
	line_n++;
	if(operator[0]=='/' && operator[1] == '/'){
		line_n--;
	}
	else if(!strcmp(operator,"function")){
		fucn_name=operand;
		TABLE_ENTRY_PTR func = malloc(sizeof(TABLE_ENTRY));
		func->type='L';
		if(!strcmp(operand,"main")){
			function_pointer=0;
			strcpy(func->fucn_name,"MAIN");
		}
		else {
			function_pointer = function_pointer + MAX_STATIC_SIZE + 1;
			strcpy(func->fucn_name,"FUCN");
		}
		func->location=function_pointer;
		symbolTable[total_comands++] = func;
	}
	else if(!strcmp(operator,"end")){
		var_number=0; //reset number of variable as were out of scope
		line_n=-1;
		while(!feof(file)){
			fscanf(file,"%100s\n",line);

			operator = strtok(line," ");
			operand = strtok(NULL," ");
			if(!strcmp(operator,"function")){
				fucn_name=operand;
				TABLE_ENTRY_PTR func = malloc(sizeof(TABLE_ENTRY));
				func->type='L';
				if(!strcmp(operand,"main")){
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
		TABLE_ENTRY_PTR table_entry = NULL;
		for(int a=MAX_CODE_SIZE -1 ;a>=line_n;a--){ //todo search from the top of array as there vars are stored
			//search for var in symbol table, if present make reference to its location
			//else create a new reference
			//printf("%d\n",a );
			if(symbolTable[a]!=NULL && symbolTable[a]->symbol==operand[0] &&(!strcmp(symbolTable[a]->fucn_name,fucn_name))){
				table_entry = symbolTable[a]; // found
				break;
			}
		}
		//if no reference yet, create a new one
		if(table_entry==NULL){
			table_entry = malloc(sizeof(TABLE_ENTRY));
			table_entry->symbol = (int)operand[0]; //set ANSI code of var name
			table_entry->type='V'; //set V for variable
			strcpy((table_entry->fucn_name),fucn_name); //copy name of curent function slope 
			table_entry->location = function_pointer + MAX_STATIC_SIZE - (++var_number); //we define local vars from the botom of function mem scope
			symbolTable[MAX_CODE_SIZE - (++total_vars)] = table_entry; //save table entry for the var we just created at the top

			}
		char KWAC_COMMAND [50];
		sprintf(KWAC_COMMAND,"%s %ld",input,table_entry->location);
		// strncat(KWAC_COMMAND,input,sizeof(KWAC_COMMAND)-1);
		// char p[] = {table_entry->location + '0','\0'};
		// strncat(KWAC_COMMAND,p,sizeof(KWAC_COMMAND));
		TABLE_ENTRY_PTR new = malloc(sizeof(TABLE_ENTRY));
		new->type = 'L';
		new->location = function_pointer + line_n; //add fucntion base pointer with ammount of kwac comands

		strcpy(new->fucn_name,KWAC_COMMAND); //copy complete input comand to the entry 
		symbolTable[total_comands++] = new; //and save it
		memset(KWAC_COMMAND,0,sizeof(KWAC_COMMAND));
		}



	}
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
				fprintf(file,"#%ld\n%d\n",symbolTable[a]->location,(symbolTable[a]->symbol - '0') );
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