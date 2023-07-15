#define _GNU_SOURCE
#include <kwork_params.h>
#include <precompiler.h>
#include <assert.h>
#ifndef MAX_CODE_SIZE
#define MAX_CODE_SIZE 10000
#endif
#ifndef MAX_STATIC_SIZE
#define MAX_STATIC_SIZE 500 //max static space for each function, containging kworker assembly instructions and variables
#endif
#define input "READ \0"
#ifndef ARG_SEPARATOR
#define ARG_SEPARATOR ","
#endif
#ifndef MAX_ARG_L
#define MAX_ARG_L 100
#endif
#ifndef MAX_FUNC_L
#define MAX_FUNC_L 80
#endif
#define NOT_EQAL -30

int flags[MAX_CODE_SIZE]={0};
char *loop_increments[MAX_NESTED_LOOPS_VALUE];
int loop_inc_pointer;
char *KWAC[MAX_CODE_SIZE];
enum TYPES{IF,FOR,WHILE};
//pass pointer to struct
#define Table_EntryToString(entry)												\
			printf("Struct table_entry{\nint const_value %d\nint symbol %d;\nchar type %c;\nlong location %ld;\nchar fucn_name %s;\n}\n",entry->const_value, entry->symbol,entry->type,entry->location,entry->fucn_name)		\


//used after each set of kwork assembly coomands to update any potential jumps 
//coused by if or loop sturctures
//@offset is number of instruction entered for each individual command
#define UPDATE_IF_BLOCKS(offset)TABLE_ENTRY_PTR assm = symbolTable[total_comands-offset];\
				 				char new_addres[30];\
								sprintf(new_addres,"%ld",assm->location);\
								int flag_p=0;\
								if(returns[total_comands-offset]!=NULL ){\
								assert(assm!=NULL);\
									while(!isEmpty(returns[total_comands-offset])){\
										int adress = pop(&(returns[total_comands-offset]));\
										/*jump_map is neg for if statments*/ \
										if(jump_map[adress%5000]>0 && jump_map[adress%5000]!=peek(&scope)){\
											push(adress,&stack);\
											if(!flag_p){push(FOR,&stuck);flag_p++;}\
										}\
										else if(isEmpty(scope) || jump_map[adress%5000]<0 || jump_map[adress%5000]==peek(&scope)){\
											char *new_command = strcat(symbolTable[adress]->fucn_name,new_addres);\
											strcpy(symbolTable[adress]->fucn_name,new_command);\
										}\
										else{\
											push(adress,&stack);\
											if(!flag_p){push(FOR,&stuck);flag_p++;}\
										}\
									}\
								}\
								if(  strncmp(operator,"else",sizeof("else")-1)!=0 && !ELSE ){\
									if(!isEmpty(if_stack)){\
										if (peek(&if_stack)==-1){pop(&if_stack);}\
										int adress = pop(&if_stack)-1;\
										if (adress<0){perror("WTF if happedn with if stack\n");}\
										char *update = strcat(symbolTable[adress]->fucn_name,new_addres);\
										strcpy(symbolTable[adress]->fucn_name,update);\
									}\
									if(if_or_for==IF){\
										if(!isEmpty(if_out_stack)){\
											int adress = pop(&if_out_stack) -1;\
											/*fucking kludge here*/ \
											if(symbolTable[adress]->location==assm->location)sprintf(new_addres,"%ld",assm->location+1);\
											char *new_command = strcat(symbolTable[adress]->fucn_name,new_addres);\
											strcpy(symbolTable[adress]->fucn_name,new_command);\
										}\
									}\
								}\
								if(strcmp(operator,"if")!=0 && strncmp(operator,"else",sizeof("else")-1)!=0  && !ELSE){\
									while(!isEmpty(if_out_stack)){\
											int adress = pop(&if_out_stack) -1;\
											/*fucking kludge here*/ \
											if(symbolTable[adress]->location==assm->location)sprintf(new_addres,"%ld",assm->location+1);\
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
		F -> Function
		A -> Array
	  }
@location adress in the memory	 
@fucn_name servers as frame for local vars of function, but if type os L serves as LINE OF CODE 
*/

struct table_entry
{
	long const_value;
	unsigned int symbol;
	char type;
	long location;
	char fucn_name[50];	
};
typedef struct table_entry TABLE_ENTRY;

typedef TABLE_ENTRY *TABLE_ENTRY_PTR;

TABLE_ENTRY_PTR symbolTable[MAX_CODE_SIZE];
enum DATA_TYPES{Word=RANDOM_V-1,Double=RANDOM_V-2,Adress=RANDOM_V-3,Function=RANDOM_V-4,Array=RANDOM_V-5};

typedef struct{
	unsigned int function_name;
	int return_type;
	int arguments[MAX_F_ARGUMENTS];

}function_prototype;
typedef function_prototype *FPROTOTYPE;
//has map storing function_prototypes
FPROTOTYPE function_prototypes[AB_FUNC_MAX]={NULL};

/*
precompiles code
with defines and includes
*/
FILE * precompile(FILE *);

/*
compiles code to KWAC (kworker assembly code)
*/
void compile(FILE *);
void first_compile(FILE *);
void second_compile();
TABLE_ENTRY_PTR find_entry(char ,long ,char *,int );
int find_location(char ,long ,char *,int );
int EV_POSTFIX_EXPP(char *,TABLE_ENTRY_PTR);

int AllocateArray(STACKPTR *,int );
int reserveMemory(int,int);

TABLE_ENTRY_PTR create_new(char , long ,char *,long);
//global vars
	static STACKPTR stack = NULL;
	static STACKPTR for_stack = NULL;
	static STACKPTR if_stack = NULL;
	static STACKPTR return_stack = NULL;
	static STACKPTR if_out_stack = NULL;
	static STACKPTR stuck = NULL;
	static STACKPTR elif = NULL;
	static STACKPTR scope = NULL;
	static int jump_map[5000];
	static char *line = NULL;
	static STACKPTR returns[MAX_CODE_SIZE];
	static int local_created=0;
	static int function_pointer=0;
	static int if_or_for=0;
	static int total_const=1;
	static int line_n=-1;
	static int R = 0;
	static int ELSE = 0; //shows if were insdie if else block
	static int local_comands =0 ;//number of commands in function and i have no fucking clue why it has to be  -8 lmao im fucking retard but it works this way
	static int total_comands = 0;
	static int total_vars=0;
	static char *operator =NULL;
	static char *operand  =NULL;
	static char *fucn_name  =NULL;
	static char last_line[100];
	static char *rest =NULL;
	static int array_t=0;
	static char ASM_FLAG=0;
	static char goto_flag=0;
	static FILE *file = NULL;
//	
int main(int argc, char const *argv[])
{	
	if(argc<1){
	  perror("No source file provided\n");	
	  return -1;
	}
	file = NULL;
	file = fopen(argv[1],"r");
	if(file==NULL){
		perror("No file found\n");	
	    return -1;
	}
	file=precompile(file);
	if(argc>2 && argv[2][0]=='E')//very bad flag practice change in future!
	{
		char buffer[100];
		while(!feof(file))
		{
			fgets(buffer,100,file);
			printf("%s",buffer);
		}
		return 0;
	}		
	compile(file);
	memset(flags,-1,sizeof(flags));
	return 0;
}
FILE *precompile(FILE *file){
	char *libs[MAX_LIBS];
	unsigned int functions[MAX_LIB_FUNCTIONS];
	memset(functions,0u,sizeof functions);
	char *prototypes[MAX_LIB_FUNCTIONS];
	memset(prototypes,0,sizeof prototypes);
	unsigned int functions_added[MAX_LIB_FUNCTIONS];
	memset(functions_added,0u,sizeof functions_added);
	int lib_point=0;
	FILE * temp = tmpfile();
	if(!temp){
		fprintf(stderr,"ERROR while creating temp file");
		abort();
	}
	char line[512];
	while(!feof(file))
	{
		fgets(line,512,file);
		char *rest = line;
		while(rest[0]==' ')rest++;
		if(!strncmp(rest,"KASM",sizeof("KASM")-1)){
		ASM_FLAG=1;
		//set asm flag so the following lines are translated directly to asm
		}
		if(ASM_FLAG){
			//close asm block
			if(rest[0]=='}')ASM_FLAG=0;
			fprintf(temp,"%s",rest);
			continue;
		}
		if(!strncmp(rest,"#include ",sizeof("#include ")-1)){
			rest+=(sizeof("#include ")-1);

			char temp_s[100];
			int x=0;
			while(isprint((int)rest[0]))temp_s[x++]=rest++[0];
			temp_s[x]='\0';
			libs[lib_point]=malloc(sizeof(temp_s));
			strcpy(libs[lib_point++],temp_s);
			//create a array of included libs so we can search for functions there later on
		}
		else{
			fprintf(temp,"%s",rest);
			//else just coppy line from yorig file to the temp one
			while(strstr(rest, "CALL "))
			{
			char *function_name = strstr(rest, "CALL ")+(sizeof("CALL ")-1);	
			rest = function_name;
			while(rest[0]!='{')rest++;
			rest++[0]='\0';
			int key = (hash((unsigned char*)function_name)%MAX_LIB_FUNCTIONS)|(1u<<32);
			if(!functions[key]){
				functions[key]=hash((unsigned char*)function_name);
			}
			}
		}
	}
	fprintf(temp,"\n");
	addFunctions(temp,functions,functions_added,libs,lib_point,prototypes);
	rewind(temp);
	FILE * temp1 = tmpfile();
	int x=0;
	while(prototypes[x]!=0){
		*(strchr(prototypes[x],')')+1)='\0';
		fprintf(temp1,"prototype %s\n",prototypes[x++]);
	}
	while(!feof(temp))
	{
		fgets(line,100,temp);
		fprintf(temp1,"%s",line);
	}

	rewind(temp1);
	return temp1;
}


void compile(FILE *file){
	first_compile(file);
	second_compile();
}
void first_compile(FILE *file){
	stack = new_stack();
	for_stack = new_stack();
	if_stack = new_stack();
	return_stack = new_stack();
	if_out_stack = new_stack();
	stuck = new_stack();
	elif = new_stack();
	scope = new_stack();
	line = malloc(512);
	operator = malloc(20);
	operand = malloc(20);
	//fucn_name = malloc(50);
	rest= malloc(512);
	fucn_name = malloc(100);

	strcpy(fucn_name,"main"); //main by defual
	while(!feof(file))
	{
	int save_line=1;
	char *r=fgets(line,512,file);
	if(r==NULL||line[0]=='\0'||!strcmp(line," "))continue;

	char *saved = rest;
	strncpy(rest,line,512);
	//fscanf(file,"%100s\n",line);
	//printf("%s\n",line );

	while(line[0]==' ' || line[0]=='\t')line++;
	operator = strtok(line," ");
	operand = strtok(NULL," ");
	//if(operand == NULL)continue;
	line_n++;
	//shows that were in asm block
	if(ASM_FLAG){
		if(operator[0]=='}'){ASM_FLAG=0;//close the asm block
		continue;
		}
		else if(operator[0]=='{')continue;
		int ASM_NUMBER=0;
		while(operand[0]==' ')operand++;//remove any whitspaces at the begining
		if(operand[0]=='%'){
			//get adress of the var referenced
			operand++;
			char temp[100];
			int x=0;
			while(isprint((int)operand[0])){
				temp[x++]=operand[0];
				operand++;
			}
			temp[x]='\0';
			unsigned int hash_value = hash((unsigned char*)temp);		
			TABLE_ENTRY_PTR var =find_entry('V',hash_value,fucn_name,total_vars);
			if(var==NULL){
				fprintf(stderr,"Variable %s is undefined!",operand);
				continue;
			}
			ASM_NUMBER=var->location;

		}
		else {
			ASM_NUMBER=atoi(operand);
		}
		char temp [100];
		sprintf(temp,"%s %d",operator,ASM_NUMBER);
		symbolTable[total_comands++] = create_new('L',0,temp,function_pointer+(local_comands++));

		continue;
	}

	if(operator[0]=='/' && operator[1] == '/'){
		line_n--;
	}
	else if(!strncmp(operator,"KASM",sizeof("KASM")-1)){
		ASM_FLAG=1;
		//set asm flag so the following lines are translated directly to asm
	}
	else if(!strcmp(operator,"function")){
		strcpy(fucn_name,operand);
		//fucn_name=operand;

		if(!strcmp(operand,"main\r\n")){
			function_pointer=0;
		}
		else {
			function_pointer = function_pointer + MAX_STATIC_SIZE + 1;
			local_comands=0;
			while(rest[0]!=' ')rest++;
			while(rest[0]==' ')rest++;
			while(rest[0]!=' ')rest++;
			while(rest[0]==' ')rest++;
			// cut away function {data_type} from the string leaving only name and args
			strcpy(fucn_name,rest);
			char *fucntion_name = strtok(fucn_name,"(");
			char *function_args  = rest;
			while(function_args[0]!='\0' && function_args[0]!='(') function_args++;
			if(function_args[0]=='(')function_args++;
			function_args=strtok(function_args,")");
			char *arg  = strtok(function_args,ARG_SEPARATOR);
			int number_of_args=0;
			char temp[50];
			unsigned int params[MAX_F_ARGUMENTS];
			unsigned int data_[MAX_F_ARGUMENTS];
			unsigned int *data_types=data_;
			int p=0;
			while(arg!=NULL){
				char *t = strchr(arg,' ');
				if(!t){break;}
				*(t)='\0';
				char *data_t = arg;
				params[p++]=hash((unsigned char*)t+1);
				if (!strcmp(data_t,"double")){
					*(data_types++)=Double;
				}
				else if(!strcmp(data_t,"function")){
					*(data_types++)=Function;
				}
				else if(!strcmp(data_t,"adress")){
					*(data_types++)=Adress;
				}
				else if(!strcmp(data_t,"word")){
					*(data_types++)=Word;
				}
				else{
					fprintf(stderr,"%s is not supported data type\n",data_t);
				}
				arg  = strtok(NULL,ARG_SEPARATOR);

			}
			while(p>0){
				TABLE_ENTRY_PTR var = create_new('V',params[--p],fucn_name,(function_pointer+MAX_STATIC_SIZE-(local_created++)));
				var->const_value=*(--data_types);
				sprintf(temp,"POP %ld",var->location);//pop value from stack to the mem adress
				symbolTable[total_comands++] = create_new('L',0,temp,function_pointer+(local_comands++));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = var;
				number_of_args++;
			}
			int function_address = find_location('F',number_of_args,fucntion_name,total_vars);

			TABLE_ENTRY_PTR function = find_entry('C',function_pointer,fucn_name,total_vars);
			if(function==NULL){
				function = create_new('C',function_pointer,fucn_name,MAX_CODE_SIZE - total_const++);
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = function;
			}
			//if we find function we define it's adress
			if(function_address>0){
				if(symbolTable[function_address]->location!=-1){
					fprintf(stderr,"function %s is already defined at %d\n",fucntion_name,function_address);
				}
				symbolTable[function_address]->location=function_pointer;
			}
			else{
				function_address = MAX_CODE_SIZE - (++total_vars);
				symbolTable[function_address] = create_new('F',number_of_args,fucntion_name,function_pointer);
			}
			//this code block needs optimization
			for(int i=0;i<MAX_CODE_SIZE;i++){
				if(flags[i]>0 && symbolTable[flags[i]]!=NULL &&  symbolTable[flags[i]]->type=='F'
					&&!strcmp(symbolTable[flags[i]]->fucn_name,fucntion_name)){
					sprintf(symbolTable[i]->fucn_name,"CALL %ld",function->location);
				}
			}
		}
	}
	else if(!strncmp(operator,"end",2)){
		symbolTable[total_comands++] = create_new('L',0,"HALT 00",function_pointer+(local_comands++));
		UPDATE_IF_BLOCKS(1)
		//todo do while search for next function in file
	}
	else if(!strcmp(operator,"return")){
		char *r_expression = rest;
		//temp fix
		while(r_expression[0]=='\t')r_expression++;
		while(!isspace((int)r_expression[0]))r_expression++;
		r_expression++;
		TABLE_ENTRY_PTR ret_ = create_new('V',0,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created+1) ));
		char command[50];
		if(!isprint((int)r_expression[0]))continue;
		sprintf(command,"POP %ld",ret_->location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		UPDATE_IF_BLOCKS(1)
		TABLE_ENTRY return_;
		EV_POSTFIX_EXPP(r_expression,&return_);

		//check if function returns its type
		FPROTOTYPE func_p = function_prototypes[(int)hash((unsigned char*)fucn_name)%AB_FUNC_MAX];
		if(return_.const_value !=func_p->return_type){
			fprintf(stderr,"Function %s does not return its desired data type\n",fucn_name);
		}
		sprintf(command,"PUSH %ld",return_.location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		sprintf(command,"CALL %ld",ret_->location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		free(ret_);
		
		//todo do while search for next function in file
	}
	else if(!strcmp(operator,"input")){
		char KWAC_COMMAND [50];
		TABLE_ENTRY return_;
		EV_POSTFIX_EXPP(operand,&return_);
		sprintf(KWAC_COMMAND,"%s %ld",input,return_.location);
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
	else if(!strncmp(operator,"if",sizeof("if")-1) || (!strncmp(operator,"else",sizeof("else")-1) &&operand&& operand[0]=='i' && operand[1]=='f')){
			//only if we have else if consturction
			if(isEmpty(elif))push(1,&elif);
			if(!strncmp(operator,"else",sizeof("else")-1)){
				//inELSE=1;
				operand = strtok(NULL," ");
				if(isEmpty(if_stack)){
					fprintf(stderr,"else statment not followed by previus if block on line %d\n",line_n+1);
					abort();
				}
				if(peek(&if_stack)==-1){pop(&if_stack);} //we dont need -1 here
				int adress = pop(&if_stack) -1;
				if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
					push(adress,&(returns[total_comands]));
				jump_map[adress%5000]=-1;	

			}
			while(rest[0]!='\0' && rest[0]!=' ')rest++;
			//move rest pointer till whitespace char
			while(rest[0]!='\0'&&rest[0]==' ')rest++;
			//move till next operand
			if(
				rest[0]=='i'&&
				rest[1]=='f'&&
				rest[2]==' '
			)rest+=3;

			char comparingExpp[MAX_IF_EXPP_LEN];
			int x=1;
			comparingExpp[0]='(';


			//find a mid point of the expp
			//if '==' is not encloused by '(' ')' assume its as mid point
			char flag = 0 ; //shows ammount of braces
			int mid_point=0;
			for (int i = 0; rest[i]!='\0'; ++i)
			{
				if(rest[i]=='(')flag++;
				else if(rest[i]==')')flag--;
				else if(!flag){
					//if comparising is not enclosed
					#define STR_CMP(STR,CMP) !strncmp(STR,CMP,sizeof CMP-1)
					if(STR_CMP(&rest[i],"==")||STR_CMP(&rest[i],"!=")||
						STR_CMP(&rest[i],"<=")||STR_CMP(&rest[i],">=")||
						STR_CMP(&rest[i],"<")||STR_CMP(&rest[i],">")){
						mid_point=i;
					}
				}
			}
			if(mid_point){
				int counter=0;
				while(counter<mid_point){
					comparingExpp[x++] = *rest++;
					counter++;
				}
				comparingExpp[x++]=')';
				//copy compararator
				while(
				  rest[0]=='='||
				  rest[0]=='<'||
				  rest[0]=='>'||
				  rest[0]=='!'
				)comparingExpp[x++] = *rest++;
				comparingExpp[x++]='(';
				//copy the rest of the expression
				while(isprint((int)rest[0]))comparingExpp[x++]=*rest++;
				comparingExpp[x]=')';
				comparingExpp[x+1]='\0';


			}
			else{
				// if no mid point means we assume that usser knows whats hes doing
				while(isprint((int)rest[0]))comparingExpp[x++]=*rest++;
				comparingExpp[x]=')';
				comparingExpp[x+1]='\0';

			}
			TABLE_ENTRY decoy;
			EV_POSTFIX_EXPP(comparingExpp,&decoy);

			//result is 0/1 in acc
			char command[30];
			sprintf(command,"BRANCHZERO "); //cond jump to the end of if block
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));	


			push(total_comands,&if_stack);
			jump_map[(total_comands-1)%5000]=-1;
			if(!strncmp(operator,"else",sizeof("else")-1)){
				push(0,&elif);
			}
			else{
				push(1,&elif);
			}
			//flags[function_pointer+local_comands+2] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
		}
		else if(!strncmp(operator,"else",sizeof("else")-1)){
			//todo
			if(isEmpty(if_stack)){
					fprintf(stderr,"else statment not followed by previus if block on line %d\n",line_n+1);
					abort();
			}
			if(peek(&if_stack)==-1){pop(&if_stack);} //we dont need -1 here
			int adress = pop(&if_stack) -1;
			assert(adress>0);
			if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
			push(adress,&(returns[total_comands]));

		}
		else if(line[0]=='{'){
			if(!strcmp(last_line,"if") || !strcmp(last_line,"else") || !strcmp(last_line,"else\r\n")){
				push(IF,&stuck);
				if(isEmpty(scope)){
					//else frame must not increse the scope level
					if(!strncmp(last_line,"else",sizeof("else")-1)){
						push(0,&scope);
					}
					else
					{
						push(1,&scope);
					}
				}
				else
				{
					int last_scope_id = peek(&scope);
					//else frame must not increse the scope level
					if(!strncmp(last_line,"else",sizeof("else")-1)){
						push(last_scope_id,&scope);
					}
					else{
						push(last_scope_id+1,&scope);
					}
					
				}

			}
			else push(FOR,&stuck);
			ELSE = 1; //show that we intered if block
		}
		else if(line[0]=='}'){
				if(!isEmpty(stuck)){
					if_or_for = peek(&stuck) * peek(&stuck->next);
				}
				else{
					perror("Extra bracket found\n");
				}
				//this line causes fucking behavior fix uit 
				if(!isEmpty(if_stack) && flags[peek(&if_stack)]==IF && if_or_for==IF){
						symbolTable[total_comands++] = create_new('L',0,"BRANCH ",function_pointer+(local_comands++));
						push(total_comands,&if_out_stack);
						//push(-1,&if_stack);
						//UPDATE_IF_BLOCKS(1)
						TABLE_ENTRY_PTR assm = symbolTable[total_comands-1];
				 		char new_addres[30];\
						sprintf(new_addres,"%ld",assm->location);
						while(returns[total_comands-1]!=NULL && !isEmpty(returns[total_comands-1]))
						{
							int adress = pop(&(returns[total_comands-1]));
							if(isEmpty(scope) || jump_map[adress%5000]<0 || jump_map[adress%5000]==peek(&scope)){
								char *new_command = strcat(symbolTable[adress]->fucn_name,new_addres);
								strcpy(symbolTable[adress]->fucn_name,new_command);
							}
						}
						R=1;
						//pop value from the scope stack to indicate we exited curren scope
				}
				if(if_or_for==IF){
					pop(&scope);
				}
				//to do add last check if its loop closing bracket
				if(!isEmpty(stack) && !isEmpty(return_stack) &&if_or_for){
				if(!isEmpty(if_out_stack)) if_or_for=IF;
				int adress = pop(&stack) -1; //exit of loop
				int adress2 = pop(&return_stack) -1; //begining of loop
				char command[30];
				if(loop_inc_pointer>0){
					TABLE_ENTRY decoy;
					EV_POSTFIX_EXPP(loop_increments[--loop_inc_pointer],&decoy);
					free(loop_increments[loop_inc_pointer]);
				}
				sprintf(command,"BRANCH %ld",symbolTable[adress2]->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				UPDATE_IF_BLOCKS(1)
				if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
				push(adress,&(returns[total_comands]));
				memset(command,0,sizeof(command));
					//apped brancha adress
				}
				//is to be executed after the scopes are exited
				else if(!isEmpty(stack)&&if_or_for)
				{
					int adress = pop(&stack) -1; //exit of loop
					if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
					push(adress,&(returns[total_comands]));
				}
			//poping here results in incorect if exit!
			//but if we delete it it fucks of the entire loop
			//tho i might fixed it
			if(!isEmpty(stuck)){

					pop(&stuck);
			}
			ELSE=0;
	
		}
		else if(!strncmp(operator,"while",sizeof("while")-1)){
			char *comparingExpp;
			save_line=1;
			while (*rest && *rest!=' ')rest++;
			while (*rest && *rest==' ')rest++;
			comparingExpp=rest;
				//push next_asm_instruction to stack so we jump to it in order to iterate thru loop
			push(total_comands+1,&for_stack);
			push(total_comands+1,&return_stack);
			push(FOR,&stuck);
			TABLE_ENTRY decoy;
			char command[100];
			EV_POSTFIX_EXPP(comparingExpp,&decoy);
			//result in acc 0/1
			//jump to end if false
			sprintf(command,"BRANCHZERO ");
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			flags[total_comands]=FOR;
			push(total_comands,&stack); 
			//save the scope of the while loop
			jump_map[(total_comands-1)%5000]=peek(&scope);
		}

		else if(!strcmp(operator,"for")){
			//save_line=1;
			char *defaultValueExpp;
			char *comparingExpp=malloc(40);
			char *incrementExpp;
			while(rest[0]!=' ')rest++;
			while(rest[0]==' ')rest++;

			defaultValueExpp=strtok(rest,";");
			char *temp = strtok(NULL,";");
			int x=1;
			int y=0;
			comparingExpp[0]='(';
			while(temp[y]!='!'&&
				  temp[y]!='='&&
				  temp[y]!='<'&&
				  temp[y]!='>'
			)comparingExpp[x++] = temp[y++];
			comparingExpp[x++]=')';
			while(
				  temp[y]=='='||
				  temp[y]=='<'||
				  temp[y]=='>'
			)comparingExpp[x++] = temp[y++];
			comparingExpp[x++]='(';
			while(temp[y]!='\0')comparingExpp[x++]=temp[y++];
			comparingExpp[x]=')';
			comparingExpp[x+1]='\0';

			incrementExpp=strtok(NULL,";");

			char *left_side = strtok(defaultValueExpp,"=");
			char *right_side = strtok(NULL,"=");
			if(left_side==NULL||right_side==NULL||comparingExpp==NULL||incrementExpp==NULL){
				fprintf(stderr,"For loop is incomplete on line %s\n",rest);
				//todo error handle
			}
			int hash_value=hash((unsigned char*)left_side);
			int var_adress = find_location('V',hash_value,fucn_name,total_vars);
			if(var_adress<0){
				TABLE_ENTRY_PTR temp;
					temp = create_new('V',hash_value,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
					var_adress=MAX_CODE_SIZE-(++total_vars);
					temp->const_value=Word;
					symbolTable[var_adress] = temp;
			}
			var_adress = symbolTable[var_adress]->location;

			//this line is under investigation !
			//why did i code it this way
			TABLE_ENTRY decoy;
			int adress = EV_POSTFIX_EXPP(left_side,&decoy);
			if(adress==-1){
				//some shit happend i duno lol
			}
			else if(adress>=9200){
				fprintf(stderr,"lvalue required as left operand of assignment\n");
			}
			char command[40];


			
			//evaluate RHS//

			EV_POSTFIX_EXPP(right_side,&decoy);
			loop_increments[loop_inc_pointer] = malloc(50);
			strcpy(loop_increments[loop_inc_pointer++],incrementExpp);
			//result in acc store to var
			sprintf(command,"STORE %d",var_adress);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			
			//push next_asm_instruction to stack so we jump to it in order to iterate thru loop
				push(total_comands+1,&for_stack);
				push(total_comands+1,&return_stack);
				push(FOR,&stuck);
			EV_POSTFIX_EXPP(comparingExpp,&decoy);
			//result in acc 0/1
			//jump to end if false
			sprintf(command,"BRANCHZERO ");
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			flags[total_comands]=FOR;
			push(total_comands,&stack); 
			//save the scope of the for loop
			jump_map[(total_comands-1)%5000]=peek(&scope);

			//free(defaultValueExpp);
			free(comparingExpp);
			//free(incrementExpp);
		}

		else if(!strcmp(operator,"putc")){
			
			char temp[40];
			while(rest[0]!=' ')rest++;
			TABLE_ENTRY argument;
			EV_POSTFIX_EXPP(rest,&argument);
			sprintf(temp,"PRINT %ld",argument.location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			UPDATE_IF_BLOCKS(1);
		}
		else if(!strcmp(operator,"put")){
			char temp[40];
			while(rest[0]!=' ')rest++;
			TABLE_ENTRY argument;
			EV_POSTFIX_EXPP(rest,&argument);
			sprintf(temp,"WRITE %ld",argument.location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			UPDATE_IF_BLOCKS(1);
		}
		else if(!strcmp(operator,"put_f")){
			char temp[40];
			while(rest[0]!=' ')rest++;
			TABLE_ENTRY argument;
			EV_POSTFIX_EXPP(rest,&argument);
			sprintf(temp,"WRITE_F %ld",argument.location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			UPDATE_IF_BLOCKS(1);
		}
		else if(!strcmp(operator,"let"))
		{
			strcpy(operand,"");//temp fix
			while(rest[0]!='\0' && rest[0]==' ')rest++;
			while(rest[0]!=' ')rest++;
			while(rest[0]!='\0' && rest[0]==' ')rest++;
			operand = rest;
			char *var_n = operand;
			char *equation  = strchr(operand,'=') + 1;
			if(equation==NULL){
				fprintf(stderr,"invalid assign\n '=' missing in line\n%s\n ",rest);
				continue;
			}
			*(equation-1)='\0';
			while(equation[0]!='\0' && equation[0]==' ')equation++;
			while(var_n[0]!='\0' && var_n[0]==' ')var_n++;
			int data_type=Word;
			if (!strncmp(var_n,"double",sizeof("double")-1)){
				data_type=Double;
				var_n+=sizeof("double");
			}
			else if (!strncmp(var_n,"adress",sizeof("adress")-1)){
				data_type=Adress;
				var_n+=sizeof("adress");
			}
			else if (!strncmp(var_n,"function",sizeof("function")-1)){
				data_type=Function;
				var_n+=sizeof("function");
			}
			else if (!strncmp(var_n,"word",sizeof("word")-1)){
				data_type=Word;
				var_n+=sizeof("word");
			}
			if(var_n[0]=='@'){
				var_n++;
				//get adress where we store
				//make sure that var_n doesnt have whitespaces in it to prevent undef var error
				TABLE_ENTRY argument;
				EV_POSTFIX_EXPP(var_n,&argument);
				//evaluate right side of equation
				TABLE_ENTRY decoy;
				EV_POSTFIX_EXPP(equation,&decoy);
				//result in acc
				char temp[40];
				sprintf(temp,"PSTORE %ld",argument.location);
				symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));

			}
			//else its word
			else
			{
			//load adress of array first element
			unsigned char *var_name = (unsigned char*)var_n;
				while(var_n[0]!='\0'&& (!isOperator(var_n[0])&&var_n[0]!='['))
				{
					var_n++;
				}
			//(var_n-1)[0]='\0';
			char temp_char=var_n[0];	
			var_n[0]='\0';
			remove_spaces((char *)var_name);

			unsigned int hash_value = hash(var_name);
			var_n[0]=temp_char;			
			TABLE_ENTRY_PTR var =find_entry('V',hash_value,fucn_name,total_vars);
			char *save;
			char *array_sub = strtok(var_n,"[");
			// var_n = array_sub;
			// array_sub = strtok(NULL,"[");
			char temp[40];
			TABLE_ENTRY decoy;
			//if we find bracket that means that we have array subscriptoion assigment here
			if (array_sub)
			{
				var =find_entry('V',hash_value,fucn_name,total_vars);
				if (!var)
				{

					fprintf(stderr,"Array %c hasnt been initlized!\n",var_n[0]);
					continue;
				}
				TABLE_ENTRY_PTR temp_v = create_new('V',0,fucn_name,MAX_CODE_SIZE - total_const - 50);

				//IMPORTANT !!! here 258 is offset memory overide !
				int adress = MAX_CODE_SIZE-(total_const) -258;
				//symbolTable[adress] = temp_v;

				 // sprintf(temp,"LOAD %ld",var->location);
				 // symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
				while (array_sub)
				{
				    save=array_sub;
				    array_sub=strtok(NULL,"[");
				    if(!array_sub){
					//if save was last array_subsriction
					//we evaluate array subsription and add it with an adress of last element
				    EV_POSTFIX_EXPP(strtok(save,"]"),&decoy);	
					sprintf(temp,"ADD %ld",var->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
					sprintf(temp,"STORE %ld",temp_v->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
	
				    }
				    else
				    {
					// if there're more array subs
					// add adress with substicption (offset)
				    EV_POSTFIX_EXPP(strtok(save,"]"),&decoy);	
					sprintf(temp,"ADD %ld",var->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
					sprintf(temp,"STORE %ld",temp_v->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
					sprintf(temp,"PLOAD %ld",temp_v->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
					sprintf(temp,"STORE %ld",temp_v->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
					var=temp_v;
				    }
			    }

				// by the end of the loop we must have an adress memory where to store
				//asume its present in acc

				//todo store it in temp var
				EV_POSTFIX_EXPP(equation,&decoy);
				sprintf(temp,"PSTORE %ld",temp_v->location);
				symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
				free(temp_v);
			}
			else
			{
			// if not found create new
			int initlized=0;	
				if(var==NULL)
				{
				var = create_new('V',hash_value,fucn_name,(function_pointer+MAX_STATIC_SIZE-(local_created++)));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = var;
				initlized=1;
				}
			//if var has just beein 	
			if(initlized){
				var->const_value=data_type;
			}
			else if (var->const_value==data_type &&!initlized)
			{
				//just ignore
			}
			else if (var->const_value!=data_type &&!initlized){
				//fprintf(stderr,"var %s has alread been initlized and has diferent data type\n",var_name);
			}

			TABLE_ENTRY decoy;
			decoy.const_value=data_type;	
			EV_POSTFIX_EXPP(equation,&decoy);
			//UPDATE_IF_BLOCKS(total_comands - save);
			//store the data type of var
			if(var->const_value!=decoy.const_value&&decoy.const_value!=Array){
				fprintf(stderr,"the rvalue %s does not yeld desired data type\n",equation);
			}
			else{
				var->const_value=decoy.const_value;
			}
			sprintf(temp,"STORE %ld",var->location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			}	
		}
		}
		else if(!strcmp(operator,"NEW_THREAD")){

		}
		else if(!strcmp(operator,"prototype")){
			rest+=sizeof("prototype");

			while(rest[0]==' ')rest++;
			char *return_type = rest;
			while(rest[0]!=' ')rest++;
			rest[0]='\0';
			do {rest++;}while(rest[0]==' ');
			char *function_name = rest;
			while(rest[0]!=' '&&rest[0]!='(')rest++;
			rest[0]='\0';
			rest++;
			if(rest[0]==' ')while(rest[0]!='(')rest++;
			char *arguments = rest;
			//go over '(' asume there is!
			//remove the closing
			*(strchr(arguments,')'))='\0';
			//todo fix segfault when argumnet is just a type without name!

			FPROTOTYPE func_prot = malloc(sizeof(function_prototype));
			memset(func_prot->arguments,-1,sizeof(int));
			function_prototypes[(int)hash((unsigned char*)function_name)%AB_FUNC_MAX]=func_prot;
			func_prot->function_name = (int)hash((unsigned char*)function_name);
				if (!strcmp(return_type,"double")){
					func_prot->return_type=Double;
				}
				else if(!strcmp(return_type,"function")){
					func_prot->return_type=Function;
				}
				else if(!strcmp(return_type,"adress")){
					func_prot->return_type=Adress;
				}
				else if(!strcmp(return_type,"word")){
					func_prot->return_type=Word;
				}
				else{
					fprintf(stderr,"%s is not supported data type\n",return_type);
				}

			char *argument = strtok(arguments,ARG_SEPARATOR);
			int x=0;
			while(argument!=NULL){
				strchr(argument,' ')[0]='\0';
				if (!strcmp(argument,"double")){
					func_prot->arguments[x++]=Double;
				}
				else if(!strcmp(argument,"function")){
					func_prot->arguments[x++]=Function;
				}
				else if(!strcmp(argument,"adress")){
					func_prot->arguments[x++]=Adress;
				}
				else if(!strcmp(argument,"word")){
					func_prot->arguments[x++]=Word;
				}
				else{
					fprintf(stderr,"%s is not supported data type\n",argument);
				}
				argument=strtok(NULL,ARG_SEPARATOR);
			}	

		}


		else if(isprint((int)line[0])){
			//dont count number of commands if operation is not defined in compiler
			printf("command -> %s is not defined\n" ,line);
			//line_n--;
			save_line=0;
		}
		else{
			save_line=0;
		}
		rest=saved;
		if(save_line)strcpy(last_line,operator);
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
	file = fopen(ASEM_NAME,"w");
	if(file==NULL){
		perror("could not create kworker assembly file\n");
	}
	int last_adr = 0;
	fprintf(file,"&0\nMAIN\n");
	int const_numb=0;
	int array_al_size=0;
	//number of consts
	for(int a=0;a<MAX_CODE_SIZE;a++){
		if(symbolTable[a]!=NULL){
			#ifdef _DEBUG
			Table_EntryToString(symbolTable[a]);
			#endif
			switch (symbolTable[a]->type){
				case'T':
				case'R':
				array_al_size++;
				break; 
				case'A':
				case'C':
				//#location
				//number
				fprintf(file,"#%ld\n%ld\n",symbolTable[a]->location,(symbolTable[a]->const_value) );
				const_numb++;
				break;
				//case 'V':
				//do nothing as we dont need to alloc memory in kernel
				//break;
				case 'L':
				//if adress is different with preivus command we place & and specify andress
				//& - sets kernel instruction input adress by value
				if( (symbolTable[a]->location - last_adr ) >1){
					fprintf(file,"&%ld\n%s\n",symbolTable[a]->location, "FUNC");
					last_adr = symbolTable[a]->location;
				}
				if(1) {
					#ifdef DEBUG
					fprintf(file,"%d:		%s\n",symbolTable[a]->location,symbolTable[a]->fucn_name);
					#else
					fprintf(file,"%s\n",symbolTable[a]->fucn_name);
					#endif
					last_adr++;
				}
				break;

			}
		}
	}
	fclose(file);

	//chech if we allocated more consts then max const
	if(const_numb>=MAX_CONST_POOL){
		fprintf(stderr,"Attention size of constantant allocated memory(const ): %d has exceeded the max const pool:%d\nThis can cause undefined behavior and seg faults\nSuggested to define MAX_CONST_POOL to %d ",const_numb,MAX_CONST_POOL,const_numb);
	}

}
int AllocateArray(STACKPTR *stack,int data_type){
	//better check if stack is empty but i dont fucking care!
	int array_size = pop(stack);
	int zero_element = reserveMemory(array_size,data_type);

	if(isEmpty((*stack))){
		return symbolTable[zero_element]->location;
	}
	else{
		//sub array size
		int sub_arr_s = pop(stack);
		//push array_size of subarray sizes because for each member of array in array size 
		//distance we create sub array by poping
		for(int a=0;a<array_size;a++){
			push(sub_arr_s,stack);
			//fill created array with pointers to coresponding pointers to subarrays
			TABLE_ENTRY_PTR pointer = create_new('A',AllocateArray(stack,data_type),fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));
			symbolTable[MAX_CODE_SIZE-(++total_vars)]=pointer;
			symbolTable[zero_element+a]->const_value=pointer->location;
			symbolTable[zero_element+a]->type='A';

		}
		//return location of first element
		return symbolTable[zero_element+array_size-1]->location;
	}
}
/*
	(z1-1)*y*x+(y1-1)*x+x1 for 3D array to find

	pse function calc_add(stack subscr,sizes [], sizepoint)
	{
		x = pop(subscr)
		if subscr empty
		return x
		else
		y=1
		for(i<sizepoint-1)
		y*=size[i]
		return (x-1)*y+calc_add(subscr,sizes,sizepoint-1)
	}
	pse function getAdd(){
		fill stack with array subsriptions
		return calc_add(subs,get array sizes,get array dismention - 1)
	}
*/


int reserveMemory(int size,int data_type){
	for(int a=0;a<size;a++){
		TABLE_ENTRY_PTR array_element = create_new('R',data_type,fucn_name,(ESP_ADR-MAX_STACK_SIZE)- array_t++ );
		symbolTable[MAX_CODE_SIZE-(++total_vars)] = array_element;
	}
	//return adress in table
	return MAX_CODE_SIZE-(total_vars);
}
TABLE_ENTRY_PTR find_entry(char type,long data,char *fucn_name,int total_vars){
	switch(type){
		case'C':
		for(int x=1;x<=total_vars;x++){ //todo search from the top of array as there vars are stored
			int a =MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->const_value==data){
				return symbolTable[a]; // found
				break;
			}
		}
		break;

		case'V':
		for(int x=1;x<=total_vars;x++){ //todo search from the top of array as there vars are stored
			int a = MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->symbol==data &&(!strcmp(symbolTable[a]->fucn_name,fucn_name))){
				return symbolTable[a]; // found
			}
		}
		break;
	}
	// //if not found create new
	// TABLE_ENTRY_PTR new = malloc(sizeof (TABLE_ENTRY));
	// new->location=function_pointer+MAX_STATIC_SIZE-line_n;
	// new->type=type;
	// new->const_value=data;
	// new->symbol=data;
	// strcpy(new->fucn_name,fucn_name);
	return NULL;

}
TABLE_ENTRY_PTR create_new(char type,long data,char *fucn_name,long location){
	TABLE_ENTRY_PTR new = malloc(sizeof (TABLE_ENTRY));
	new->location=location;
	new->type=type;
	new->const_value=data;
	new->symbol=data;
	strcpy(new->fucn_name,fucn_name);
	return new;
}
int find_location(char type,long data,char *fucn_name,int total_vars){
	switch(type){
		case'F':
		for(int x=1;x<=total_vars;x++){ //todo search from the top of array as there vars are stored
			int a =MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->type=='F'&& symbolTable[a]->const_value==data &&!strcmp(symbolTable[a]->fucn_name,fucn_name)){
				return a; // found
				break;
			}
		}
		return -1;
		break;
		case'C':
		for(int x=1;x<=total_vars;x++){ //todo search from the top of array as there vars are stored
			int a =MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->type=='C' &&symbolTable[a]->const_value==data){
				return a; // found
				break;
			}
		}
		break;

		case'V':
		for(int x=1;x<=total_vars;x++){ //todo search from the top of array as there vars are stored
			int a = MAX_CODE_SIZE - x;
			if(symbolTable[a]!=NULL && symbolTable[a]->symbol==data &&(!strcmp(symbolTable[a]->fucn_name,fucn_name))){
				return a; // found
			}
		}
		break;
	}
	return -1;

}
int EV_POSTFIX_EXPP(char *expp,TABLE_ENTRY_PTR return_){
	if(goto_flag==1&&expp==NULL&&return_==NULL)goto array_goto;
	//todo after func call doesnt evalueate the rest of the expression
	int data_type=Word;
	if(expp[0]=='"'){
		data_type=Adress;
		STACKPTR array = new_stack();
		expp++;
		while(expp[0]!='\0' && expp[0]!='"'){
			//todo add special char support
			int value=0;
			if(expp[0]=='\\'){
				switch(expp[1]){
					case'\\':
					value=(int)'\\';
					break;
					case'n':
					value=(int)'\n';
					break;
					case'\'':
					value=(int)'\'';
					break;
					case'\"':
					value=(int)'\"';
					break;
					case'a':
					value=(int)'\a';
					break;
					case'f':
					value=(int)'\f';
					break;
					case'r':
					value=(int)'\r';
					break;
					case't':
					value=(int)'\t';
					break;
					case '0':
					value=(int)'\0';
					break;
					case'v':
					value=(int)'\v';
					break;
				}
				expp+=2;
			}
			else{
				value=(int)expp[0];
				expp++;
			}
			push(value,&array);
		}
		push( (int)'\0',&array);

		while(!isEmpty(array)){
			int value = pop(&array);
			TABLE_ENTRY_PTR CHAR = create_new('A',value,fucn_name,MAX_CODE_SIZE - total_const++);
			symbolTable[MAX_CODE_SIZE-(++total_vars)] = CHAR;
			if(isEmpty(array)){
				// if poped element is last
				char command[40];
				int string_pointer = find_location ('C',CHAR->location,fucn_name,total_vars);
				if(string_pointer<0){
					TABLE_ENTRY_PTR CONST = create_new('C',CHAR->location,fucn_name,MAX_CODE_SIZE - total_const++);
					string_pointer = MAX_CODE_SIZE-(++total_vars);
					symbolTable[string_pointer]=CONST;
				}
				sprintf(command,"LOAD %ld", symbolTable[string_pointer]->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				{UPDATE_IF_BLOCKS(1)}
				return_->location=symbolTable[string_pointer]->location;
				return_->const_value=data_type;
				return  symbolTable[string_pointer]->location;	
			}

		}
	}
	// if array declaration
	else if(expp[0]=='['){
		STACKPTR array_sizes = new_stack();
		while(rest[0]!='[')rest++;
		char *numb = strtok(rest,"]");
		int array_size;
		int dism = -1;
		while(numb!=NULL){
			dism++; 
			numb++; // skip opening bracket assume there is !
			//for now we only init arrays with consts values
			array_size=atoi(numb);
			if(array_size>0)push(array_size,&array_sizes);
			numb = strtok(NULL,"]");
		}
		int array_pointer = AllocateArray(&array_sizes,return_->const_value);
		char command[40];
		if(dism==1)
		{
		TABLE_ENTRY_PTR CONST = create_new('C',array_pointer,fucn_name,MAX_CODE_SIZE - total_const++);
		symbolTable[ MAX_CODE_SIZE-(++total_vars)]= CONST;
		sprintf(command,"LOAD %ld",CONST->location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		}
		else{
		sprintf(command,"LOAD %d",array_pointer);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		}
		return_->const_value=Array;
		return array_pointer;
	}
	//if array literal declaration
	else if (expp[0]=='{'){
		long values[512];
		int elemets_n=-1;
		int data_type=Word;
		while(rest[-1]!='{')rest++;
		array_goto:
		char coppy[512]={0};
		strncpy(coppy,rest,sizeof coppy);
		char *number=strtok(rest,",");
		while(number!=NULL){
			char dig[512]={0};
			long c_value=0;
			int x=0;
			//clean all the unprintable chars at the begining of substring
			while(!isprint(*number)&&*number)number++;
			if (isprint((int)number[0])&&number[0]!='}')
			{

				if(number[0]==NEGNUMBER||number[0]=='-')
				{
				number++;
				dig[x++]='-';
				}

				//if a char
				if(number[0]=='\''){
					//skip the opening apostrafy charecter
					number++;
					c_value=number[0];
					if (number[1]!='\''){
						//rise warning message
						fprintf(stderr,"Warning string enclosed in charecter apostrafy\n");
					}
					number+=2;

				}
				//if a number
				else
				{
					while(isspace((int)number[0]))number++;

					while(number[0]!='\0' && number[0]!='{'&&!isspace((int)number[0]) && (!isOperator(number[0]) ) )
					{
						dig[x++]=number[0];
						number++;
					}
					if(((char *)strchrnul(dig,'.'))[0]){
						double result = atof(dig);
						c_value=*(long*)&result;
						data_type=Double;
					}
					else{
						dig[x]='\0';
						c_value = atoi(dig);
					}
				}
				values[++elemets_n]=c_value;
		}
			number=strtok(NULL,",");
			if(number==NULL){
			// if there is no closing bracket continue reading lines till there;s
			if(!strchr(coppy,'}'))
			{
				//read strings while not found
				fgets(line,512,file);
				strncpy(coppy,line,sizeof coppy);
			char *rest = line;
			while(rest[0]==' ')rest++;
			//suposse rest is a good string
			number=strtok(rest,",");
			}
			else {
				//set the flag to zero
				goto_flag=0;
				break;
			}
			}

		}
		int zero_element = reserveMemory(elemets_n+1,data_type);
		TABLE_ENTRY_PTR pointer;
		TABLE_ENTRY_PTR first_el;
		for(int a=elemets_n;a>=0;a--){
			//fill created array with pointers to coresponding pointers to subarrays
			pointer = create_new('A',values[a],fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));
			symbolTable[MAX_CODE_SIZE-(++total_vars)]=pointer;
			if (a==0)first_el=pointer;
			// symbolTable[zero_element+a]->const_value=values[a];
			// symbolTable[zero_element+a]->type='A';

		}
		//return location of first element
		char command[100];
		int array_p = first_el->location;
		TABLE_ENTRY_PTR CONST = create_new('C',array_p,fucn_name,MAX_CODE_SIZE - total_const++);
		symbolTable[ MAX_CODE_SIZE-(++total_vars)]= CONST;
		sprintf(command,"LOAD %ld",CONST->location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		return_->const_value=Array;
		return CONST->location;


	}

	char expp_san[512];
	//i have no fucking clue why the expp_san is not freed after the slope exist and it has old info.'
	memset(expp_san,0,sizeof expp_san);
	remove_ws_fr_exp(expp,expp_san);
	char *postfix = convertToPostfix(expp_san);
	
	char *saved_postfix=postfix;
		STACKPTR stack = new_stack();
		STACKPTR operations = new_stack();
		char floats = 0;
		char double_aray=0;
		enum OPERATIONS{UNARY,BINARY};
		int created = total_vars;
		int code_lines=0;
		int negative_number=1;
		while(postfix!=NULL &&  postfix[0]!='\0'){
			if(postfix[0]<0 || !isprint((int)postfix[0])){postfix++;continue;}
			while(isspace((int)postfix[0]))postfix++;
			if(isdigit((int)postfix[0]) || (postfix[0]==NEGNUMBER || postfix[0]=='-' &&isdigit((int)postfix[1] ))){
				char *dig = malloc(50);
				int x=0;
				if(postfix[0]==NEGNUMBER||postfix[0]=='-'){
				postfix++;
				dig[x++]='-';
				}
				while(postfix[0]!='\0' && postfix[0]!='{'&&!isspace((int)postfix[0]) && (!isOperator(postfix[0]) || negative_number ) ){
					dig[x++]=postfix[0];
					postfix++;
				}
				int tem=0;
				long c_value=0;
				if(((char *)strchrnul(dig,'.'))[0]){
					data_type=Double;
					double result = atof(dig);
					c_value=*(long*)&result;
					floats=1;
					tem=1;
				}
				else{
					dig[x]='\0';
					c_value = atoi(dig);
				}
				int ad = find_location ('C',c_value,fucn_name,total_vars);
				if(ad<0){
					TABLE_ENTRY_PTR CONST = create_new('C',c_value,fucn_name,MAX_CODE_SIZE - total_const++);
					ad = MAX_CODE_SIZE-(++total_vars);
					symbolTable[ad]=CONST;
				}
				if(tem){
					symbolTable[ad]->symbol=Double;
				}
				free(dig);
				push(ad,&stack);
			}
			else if(postfix[0]=='\'')
			{
				int value=(int)postfix[1];
				if(postfix[1]=='\\'){
				switch(postfix[2])
				{
					case'\\':
					value=(int)'\\';
					break;
					case'n':
					value=(int)'\n';
					break;
					case'\'':
					value=(int)'\'';
					break;
					case'\"':
					value=(int)'\"';
					break;
					case'a':
					value=(int)'\a';
					break;
					case'f':
					value=(int)'\f';
					break;
					case'r':
					value=(int)'\r';
					break;
					case't':
					value=(int)'\t';
					break;
					case'v':
					value=(int)'\v';
					break;
					case'0':
					value=(int)'\0';
					break;
				}
				postfix+=4;
			}
			else
			{
				postfix+=3;
			}
				int ad = find_location ('C',value,fucn_name,total_vars);
				if(ad<0){
					TABLE_ENTRY_PTR CONST = create_new('C',value,fucn_name,MAX_CODE_SIZE - total_const++);
					ad = MAX_CODE_SIZE-(++total_vars);
					created++;
					symbolTable[ad]=CONST;
				}
				push(ad,&stack);
			}
			else if(!strncmp(postfix,"{f}",sizeof("{f}")-1)){
					//perform next operation as float operation

					//move pointer forward
					postfix+=sizeof("{f}")-1;
					data_type=Double;
					floats=1;
			}
			//data casts for returns
			else if(!strncmp(postfix,"{a}",sizeof("{a}")-1)){
					postfix+=sizeof("{a}")-1;
					data_type=Adress;
			}
			else if(!strncmp(postfix,"{w}",sizeof("{w}")-1)){
					postfix+=sizeof("{w}")-1;
					data_type=Word;
			}


			else if (postfix[0]>0 && isOperand(postfix[0])){
				if(postfix[0]=='C' && postfix[1]=='A' && postfix[2]=='L' && postfix[3]=='L'){
					//cut 'CALL'away from the beggining of the string
					postfix+=4;
					int x=0;
					char *fucntion_name = malloc(MAX_FUNC_L);
					memset(fucntion_name,0,MAX_FUNC_L);
					//move string to the begging of arguments
					if(!*postfix){
						fprintf(stderr,"SOME SHIT HAPPENED\n");
						continue;
					}
					while(postfix[0]!='\0'&&postfix[0]==' ')postfix++;
					while(postfix[0]!='{'){
						fucntion_name[x++]=postfix[0];
						postfix++;
					}
					int bracket = 0;
					//here we use bracket to see if we are inside brackets
					//for each { we increment by one and for each } decriment by one
					//so if bracket is 0 that means we can split the argument by comma 

					//cut first char
					postfix++;

					char *arguments = malloc(MAX_ARG_L);
					memset(arguments,0,MAX_ARG_L);
					x=0;
					int y=0;
					//while last closing bracket is found
					int last_bracket = strrchr(postfix,'}') - postfix;
					int apostrix=0;
					while(y<last_bracket){
						if (postfix[0]=='{'){
							bracket++;
							arguments[x++]=postfix[0];
						}
						else if (postfix[0]=='}'){
							bracket--;
							arguments[x++]=postfix[0];
						}
						else if (postfix[0]==',' &&!apostrix && !bracket){
							arguments[x++]='$';
						}
						else
						{
						arguments[x++]=postfix[0];
						if(postfix[0]=='\''){
							apostrix=!apostrix?1:0;
						}
						}
						postfix++;
						y++;
					}
					postfix++;
					char *arg = strtok(arguments,"$");
					int number_of_args =0;
					char *args [MAX_ARGS];
					while(arg!=NULL){
						args[number_of_args]=malloc(100);
						strcpy(args[number_of_args++],arg);
						//args[number_of_args++]=arg;
						arg=strtok(NULL,"$");
					}
					char command[40];
					//push return adress of the function calling
					TABLE_ENTRY_PTR function = find_entry('F',total_comands+1,fucn_name,total_vars);
					if(function==NULL){
						function = create_new('T',total_comands+1,fucn_name,MAX_CODE_SIZE - total_const++);
						if(symbolTable[MAX_CODE_SIZE-(1+total_vars)] && symbolTable[MAX_CODE_SIZE-(1+total_vars)]->type!='T'&& symbolTable[MAX_CODE_SIZE-(1+total_vars)]->type>0){
							fprintf(stderr,"memory overide caused by return adress");
							abort();
						}
						else if(symbolTable[MAX_CODE_SIZE-(1+total_vars)] && symbolTable[MAX_CODE_SIZE-(1+total_vars)]->type=='T'){
							free(symbolTable[MAX_CODE_SIZE-(1+total_vars)]);
						}
						symbolTable[MAX_CODE_SIZE-(++total_vars)] = function;
					}

					sprintf(command,"PUSH %ld",function->location);
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					//push the arguments on the stack
					UPDATE_IF_BLOCKS(1)

					FPROTOTYPE func_p = function_prototypes[(int)hash((unsigned char*)fucntion_name)%AB_FUNC_MAX];
					if (func_p==NULL){
						fprintf(stderr,"function %s is missing its prototype, all arguments types are assumed as words!",fucntion_name);
					}
					int *types = &(func_p->arguments[0]);
					for(int a=0;a<number_of_args;a++){
						//evaluate epression
						TABLE_ENTRY argument;
						EV_POSTFIX_EXPP(args[a],&argument);
						free(args[a]);
						char command[40];
						int data_type=0;
						if (func_p!=NULL){
							data_type=*types;
							types++;
						}
						//todo check the function prototype for type checks
						//and cast to double if required
						//and push it on the stack
						switch (data_type%127){
							case 0:
							case Word%127:
							//Word by default if no function prototype
							//or wrong signature
							//or argument is actuall word

							switch(argument.const_value%127){
								case Word%127:

								break;

								case Double%127:

								break;

								case Adress%127:

								break;

								case Function%127:

								break;	
							}
							break;
							case Double%127:
							switch(argument.const_value%127){
								case Word%127:
									if(!double_aray){
									//do the cast from int to it's IEEE representation
									sprintf(command,"CAST_L_D %ld",argument.location);
									symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
									//todo dont store at the same adress!!!
									sprintf(command,"STORE %ld",argument.location);
									symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
									}

								break;

								case Double%127:

								break;

								case Adress%127:

								break;

								case Function%127:

								break;	
							}
							break;
							case Adress%127:
							switch(argument.const_value%127){
								case Word%127:

								break;

								case Double%127:

								break;

								case Adress%127:

								break;

								case Function%127:

								break;	
							}
							break;
							case Function:
							switch(argument.const_value%127){
								case Word%127:

								break;

								case Double%127:

								break;

								case Adress%127:

								break;

								case Function%127:

								break;	
							}
							break;

						}
						sprintf(command,"PUSH %ld",argument.location);
						symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
						
					}
	
					free(arguments);
					//move pointer to the end of function args (to closing bracket)
					// EXMP sum(5,6) + 1324 
					// ->
					// + 1324
					
					// if(postfix!=NULL) strtok(postfix,"{");
					// if(postfix!=NULL) postfix=strtok(NULL,"}");
					//push return value on stack
					int call_address = total_comands++;

					int function_address = find_location('F',number_of_args,fucntion_name,total_vars);
					//if function not found or it's still undefined
					if(function_address < 0 || symbolTable[function_address]->location==-1){
						symbolTable[call_address] = create_new('L',0,"CALL ",function_pointer+(local_comands++));
						// create a undefined function with mem adress of -1 so we can resolve function call later
						TABLE_ENTRY_PTR func = create_new('F',number_of_args,fucntion_name,-1);
						int function_address = MAX_CODE_SIZE - (++total_vars);
						flags[call_address] = function_address;
						symbolTable[function_address]=func;
					}
					//if found we make function call to it's adress
					else{
						char command[50];
						TABLE_ENTRY_PTR function = find_entry('C',symbolTable[function_address]->location ,fucn_name,total_vars);
						if(function==NULL){
							function = create_new('C',symbolTable[function_address]->location,fucn_name,MAX_CODE_SIZE - total_const++);
							symbolTable[MAX_CODE_SIZE-(++total_vars)] = function;
						}
						sprintf(command,"CALL %ld",function->location);
						symbolTable[call_address] = create_new('L',0,command,function_pointer+(local_comands++));
					}
					created++;
					TABLE_ENTRY_PTR temp = create_new('T',0,fucn_name,MAX_CODE_SIZE - created - 50);
					int adress = MAX_CODE_SIZE-(created) -50;
					symbolTable[adress] = temp;
					temp->type=func_p?func_p->return_type:Word;
					sprintf(command,"POP %d",adress);
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					push(adress,&stack);
					// todo pop return value from stack and store in temp var
					function->const_value = function_pointer+ local_comands-1; 
					function->type='C';
					//set the return value it's coresponidng type
					temp->const_value=func_p->return_type;
					if(func_p->return_type==Double){
						data_type=Double;
						floats=1;
					}
					data_type=func_p->return_type;

					
				}

				else if(postfix[0]==' ' || postfix[0]<0)postfix++;
				else{
					//if var name 
					unsigned char *var_name = (unsigned char*)postfix;
					while(postfix[0]!='\0' && (isalpha((int)postfix[0])||postfix[0]=='_')){
						postfix++;
					}
					char temp_c=postfix[0];
					postfix[0]='\0';
					unsigned int hash_value = hash(var_name);
					postfix[0]=temp_c;
					int ad = find_location ('V',hash_value,fucn_name,total_vars);
					if(ad<0){
						fprintf(stderr,"Variable (%s) is undefined!\nAt line %s",var_name,expp);
						TABLE_ENTRY_PTR VAR = create_new('V',0,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++)));
						int ad = MAX_CODE_SIZE-(created++) -50;
						abort();
					symbolTable[ad]=VAR;
					}
					if(symbolTable[ad]->const_value==Array){
						//we assume that zero lengh arrays are not supported!!
						//we also assume that layout of the memory reserved blocks is not altered
						switch(symbolTable[ad-1]->const_value){
							case Double:
								double_aray=1;
								//floats=1;
							break;

							case Word:

							break;

							case Adress:

							break;

							case Function:

							break;
						}	
					}
					else if(symbolTable[ad]->const_value==Double){
						floats=1;
					} 
					push(ad,&stack);
				}
			}
			else if(postfix[0]=='['){
				postfix++;
				int x = 0;
				char array_sub[40];
				memset(array_sub,0,sizeof(array_sub));
				while(postfix[0]!=']'){
					array_sub[x++]=postfix[0];
					postfix++;
				}
				postfix--;//go back to elements so we rewrite them with corespponding operations
				postfix[0]='+';
				postfix[1]='@';
				TABLE_ENTRY decoy;
				EV_POSTFIX_EXPP(array_sub,&decoy);
				//push(temp,&stack); //push adress of temp var
				int arr_p= pop(&stack);
				push(UNARY,&operations);
				push(-1,&stack);
				push(arr_p,&stack);
				//so we sum it up with array pointer
				//then dereference


			}
			else if (postfix[0]>0 &&isOperator(postfix[0]))
			{
				int x=0;
				int y=0;
				int floats_was_set=0;//flag if we set the floats flag in this if statement!
				int pstore=0;
				int flag=0;
				if(!isEmpty(stack))x=pop(&stack);
				if(!isEmpty(stack) &&

				 ((isEmpty(operations) || pop(&operations)==BINARY))

				 ){
					y=pop(&stack);
					if(y>0)flag=1;
				}

				// else if (!isEmpty(stack) &&
				//  ((isEmpty(operations) || pop(&operations)==UNARY))
				//  ){
				// 	//asume stack has -1 and we dont need it any more
				// 	pop(&stack);
				// 	flag=0;
				// }
				if (symbolTable[x]->const_value==Double ||(flag&&symbolTable[y]->const_value==Double) ){
					data_type=Double;
					floats=1;
				}
				else if (symbolTable[x]->const_value==Adress ||(flag&&symbolTable[y]->const_value==Adress) ){
					data_type=Adress;
				}
				/*we pop adress of vars in table here*/ 
				created++;
				TABLE_ENTRY_PTR temp = create_new('T',0,fucn_name,(ESP_ADR-MAX_STACK_SIZE)- (created+local_created) );
				int adress = MAX_CODE_SIZE-(created) -50;
				if (symbolTable[adress] && symbolTable[adress]->symbol!=0){
					fprintf(stderr,"memory overide caused by temp var allocation!\n");
					abort();
				}
				symbolTable[adress] = temp;
				char command[50];
				char command2[50];
				char load[50];
				char temp_s [40];
				char double_t[40];
				memset(temp_s,0,sizeof temp_s);
				sprintf(command2,"STORE %ld",temp->location);/*result of all operations is stored in acc*/ 
				//we only load it if not unary operation
				if(flag > 0){
					sprintf(load,"LOAD %ld",symbolTable[y]->location); 
				}
				else{
					memset(load,0,sizeof load);
				}
				code_lines++;
				int t = 1;
				int any_operands =0;
				while(postfix[t]){
					if(isOperand(postfix[t++])){
						any_operands=1;
						//set flag that there are other operands
						t=1;
						break;
					}
				}
				int comp;

				//the code below is a complete mess and i have no clue wtf im doing :)
				if(any_operands){
					while(postfix[t])
					{
					if(postfix[t]==' ')t++;
					if (isOperator(postfix[t])){
						break;
					}
					else{
						t=0;
						break;
					}
				}
				if(postfix[0]=='!' && postfix[t]=='='){
					//special case ! since sum of ! and = is same as ^ char
					comp=NOT_EQAL;
				}

				else comp = (int) (postfix[0]) + ((postfix[t]=='=' ||  postfix[t]=='<' || postfix[t]=='>'|| postfix[t]=='+'|| postfix[t]=='-'|| postfix[t]=='&'|| postfix[t]=='|')*(t&&postfix[0]!='@'?postfix[t]:0));
				if(comp==NOT_EQAL||comp==76 || comp>=120){
					postfix++;
				}
				}
				else  {
					t=0;
					comp = (int) postfix[0];
					//special case
					if(postfix[0]=='!'&&postfix[1]=='='){
						comp=NOT_EQAL;
						t=2;
					}
					else if(postfix[0]=='+'&&postfix[1]=='@'){
						//special case
						//do nothing
					}
					//this shit is needed to catch expressions like x++
					else if( postfix[1]&&postfix[0]!='@'&&isDoubleOperator(postfix[1])){
						comp+=(int)postfix[1];
						t=2;
					}
					else {
						t=1;
					}
				}
				//shit code ends here but more to come!


				char apend[12];
				//if theres at least one float operation all other operations are done with floats
				//in order to prevent undefined behaviour
				char cast_ = 0;
				if(floats &&y>0){
					strcpy(apend,"_F");
					//perfrom cast to get IEEE representation
					if( (symbolTable[x]->type!='T'&&symbolTable[x]->const_value!=Double)&&(
						(symbolTable[x]->type=='C'&&symbolTable[x]->symbol!=Double) || (symbolTable[x]->type=='V'&&symbolTable[x]->symbol!=Double))
						){
					sprintf(temp_s,"CAST_L_D %ld",symbolTable[x]->location);
					//store it in temp
					sprintf(double_t,"STORE %d",temp->location);
					//perfrom cast
						if(temp_s[0]!=0){
						symbolTable[total_comands++] = create_new('L',0,temp_s,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
						//store
						symbolTable[total_comands++] = create_new('L',0,double_t,function_pointer+(local_comands++));
						}	
						//load
						if(load[0]!=0){
						symbolTable[total_comands++] = create_new('L',0,load,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
						}
					memset(temp_s,0,sizeof(temp_s));
					//set cast flag
					//so we use IEEE from temp var for the operations bellow
					x=adress;
					}
					if(
						(symbolTable[y]->type!='T'&&symbolTable[y]->const_value!=Double&&floats) &&
						(symbolTable[y]->type!='C'&&symbolTable[y]->symbol!=Double)&&
						(symbolTable[y]->type!='V'&&symbolTable[y]->symbol!=Double)&&
						!double_aray
					){
						sprintf(load,"LOAD_F %ld",symbolTable[y]->location);
						double_aray=0;
					}
					else sprintf(load,"LOAD %ld",symbolTable[y]->location);
				}
				else{
					strcpy(apend,"");
				}
				switch ( comp ){
					case'+':
					sprintf(command,"ADD%s %ld",apend,symbolTable[x]->location);
					push(BINARY,&operations);
					break;
					case'-':
					sprintf(command,"SUB%s %ld",apend,symbolTable[x]->location); 
					push(BINARY,&operations);
					break;
					case'*':
					sprintf(command,"MUL%s %ld",apend,symbolTable[x]->location); 
					push(BINARY,&operations);
					break;
					case'/':
					sprintf(command,"DIV%s %ld",apend,symbolTable[x]->location); 
					push(BINARY,&operations);
					break;
					case'%':
					sprintf(command,"MOD %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break;
					case'<'+'<': 
					sprintf(command,"BIT_S_L %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break; 
					case'>'+'>': 
					sprintf(command,"BIT_S_R %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break; 
					case'|'+'|': 
					sprintf(command,"BIT_OR %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break; 
					case'&'+'&': 
					sprintf(command,"BIT_AND %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break; 
					case'^': 
					sprintf(command,"BIT_XOR %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break; 
					case'<':
					sprintf(temp_s,"SUB %ld",symbolTable[x]->location);
					sprintf(command,"LOG_LESS%s %d",apend,0); 
					push(BINARY,&operations);
					break; 
					case'>':
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(temp_s,"SUB %ld",symbolTable[y]->location);
					sprintf(command,"LOG_LESS%s %d",apend,0); 
					push(BINARY,&operations);
					break;
					case'<'+'=':
					sprintf(temp_s,"SUB %ld",symbolTable[x]->location);
					sprintf(command,"LOG_LESSEQ%s %d",apend,0); 
					push(BINARY,&operations);
					break; 
					case'>'+'=':
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(temp_s,"SUB %ld",symbolTable[y]->location);
					sprintf(command,"LOG_LESSEQ%s %d",apend,0); 
					push(BINARY,&operations);
					break;
					case '='+'=':
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(temp_s,"SUB %ld",symbolTable[y]->location);
					sprintf(command,"LOG_INV %d",0); 
					push(BINARY,&operations);
					break;
					case NOT_EQAL:
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(command,"SUB %ld",symbolTable[y]->location); 
					push(BINARY,&operations);
					break;
					/*Unary operations*/ 
					case'!': 
					sprintf(command,"BIT_INV %ld",symbolTable[x]->location); 
					push(UNARY,&operations);
					break; 
					case'#': 
					/*get pointer*/ 
					data_type=Adress;
					adress = find_location ('C',(int)symbolTable[x]->location,fucn_name,total_vars);
					if(adress<0){
						TABLE_ENTRY_PTR VAR = create_new('C',(int)symbolTable[x]->location,fucn_name,MAX_CODE_SIZE - total_const++);
						adress = MAX_CODE_SIZE-(++total_vars);
					symbolTable[adress]=VAR;
					}
					pstore=1;
					push(UNARY,&operations);
					break; 
					case'@': 
					/*dereference pointer*/ 
					if(double_aray){
						data_type=Double;
						int x=1;
						int deref=0;//indicates if there are any dereferencing left
						while(postfix[x]){
							if(postfix[x++]=='@'){
								deref=1;//set the value to 1 to indicate that we have more to dereference
							}
						}
						//if no more deref change floats flag to 1
						floats=!deref;
						floats_was_set=floats;
						//chech if there are no more deref
					}
					else data_type=Word;
					if(any_operands)y=-1;
					sprintf(command,"PLOAD %ld",symbolTable[x]->location);
					push(UNARY,&operations);
				
					break; 

					case ('+'+'+'):
					//todo add cosnt assigment check
					sprintf(command,"LOAD %ld",symbolTable[x]->location); 
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					{UPDATE_IF_BLOCKS(1)}
					code_lines++;
					adress = find_location ('C',1,fucn_name,total_vars);
					if(adress<0){
						TABLE_ENTRY_PTR VAR = create_new('C',1,fucn_name,MAX_CODE_SIZE - total_const++);
						adress = MAX_CODE_SIZE-(++total_vars);
					symbolTable[adress]=VAR;
					}
					sprintf(command,"ADD%s %ld",apend,symbolTable[adress]->location);
					sprintf(command2,"STORE %ld",symbolTable[x]->location);/*overwrite same var*/ 
					push(UNARY,&operations);
					adress=x;
					break;

					case ('-'+'-'):
					sprintf(command,"LOAD %ld",symbolTable[x]->location); 
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					{UPDATE_IF_BLOCKS(1)}
					code_lines++;
					int adress = find_location ('C',1,fucn_name,total_vars);
					if(adress<0){
						TABLE_ENTRY_PTR VAR = create_new('C',1,fucn_name,MAX_CODE_SIZE - total_const++);
						adress = MAX_CODE_SIZE-(++total_vars);
					symbolTable[adress]=VAR;
					}
					sprintf(command,"SUB%s %ld",apend,symbolTable[adress]->location);
					sprintf(command2,"STORE %ld",symbolTable[x]->location);/*overwrite same var*/ 
					push(UNARY,&operations);
					adress=x;
					break;
				}
				postfix+=t?t:1;



				if(!floats &&!floats_was_set)
				{
					/*load first operand*/
					
					if(flag){
						symbolTable[total_comands++] = create_new('L',0,load,function_pointer+(local_comands++)); 
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
					}
					if(!pstore){
						/*perform operation*/
						if(temp_s[1]){
							symbolTable[total_comands++] = create_new('L',0,temp_s,function_pointer+(local_comands++));
							if(code_lines==1){UPDATE_IF_BLOCKS(1)}
							memset(temp_s,0,sizeof(temp_s));
						}
						symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
						/*store result in temp variable*/
						symbolTable[total_comands++] = create_new('L',0,command2,function_pointer+(local_comands++));
					}
				}
				//if double
				else
				{
					//todo: I dont know if this can affect evaluation of other expressions
					//but symbolTable[adress]->const_value=Word definatly breaks somthing. Suggested fix: use stack and push all adresses of temp
					//vars and then reset their type at the end of evaluation
					symbolTable[adress]->const_value=Double;
					if(cast_)
					{
						//perfrom cast
						if(temp_s[0]!=0){
						symbolTable[total_comands++] = create_new('L',0,temp_s,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
						//store
						symbolTable[total_comands++] = create_new('L',0,double_t,function_pointer+(local_comands++));
						}	
						//load
						if(load[0]!=0){
						symbolTable[total_comands++] = create_new('L',0,load,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
						}
					}
					else
					{
						if(load[0]!=0){
						symbolTable[total_comands++] = create_new('L',0,load,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
						}
						//perfrom 1st operation
						if(temp_s[0]!=0){
						symbolTable[total_comands++] = create_new('L',0,temp_s,function_pointer+(local_comands++));
						if(code_lines==1){UPDATE_IF_BLOCKS(1)}
					}
					//perform
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					//store
					symbolTable[total_comands++] = create_new('L',0,command2,function_pointer+(local_comands++));
					}
				}
				//used for array dereferencing
				if(y==-1){
					pop(&operations);
					push(BINARY,&operations);
					//fucking kludge here
				}
				push(adress,&stack);
			
			}
			negative_number=0;
		}
		int result=-1;
		while(!isEmpty(stack)){
			result = pop(&stack);
		}
		//printf("");
		free(saved_postfix);
		if(result>=0){
			char command[40];
			sprintf(command,"LOAD %ld",symbolTable[result]->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			if(code_lines==0){UPDATE_IF_BLOCKS(1)}
			/*todo: clear all temp vars */ 
			free(stack);

			return_->location=symbolTable[result]->location;
			return_->const_value=data_type;
			if(symbolTable[result]->type!='T'){

				if(symbolTable[result]->type!='C')return_->const_value=symbolTable[result]->const_value;
				else return_->const_value=symbolTable[result]->symbol;
				if(symbolTable[result]->type=='C')return_->const_value=data_type;
			}
			return symbolTable[result]->location;
		}\
		else{
			fprintf(stderr,"Stack is empty\n");
			return  -1;
		}
	}

