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
								if(returns[total_comands-offset]!=NULL){\
								assert(assm!=NULL);\
									while(!isEmpty(returns[total_comands-offset])){\
										int adress = pop(&(returns[total_comands-offset]));\
										char *new_command = strcat(symbolTable[adress]->fucn_name,new_addres);\
										strcpy(symbolTable[adress]->fucn_name,new_command);\
									}\
								}\
								if(  strcmp(operator,"else")!=0 && strcmp(operator,"else\r\n")!=0 && !ELSE ){\
									if(!isEmpty(if_stack)){\
										int adress = pop(&if_stack)-1;\
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
								if(strcmp(operator,"if")!=0 && strcmp(operator,"else")!=0  && !ELSE){\
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
	int const_value;
	unsigned int symbol;
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
FILE * precompile(FILE *);

/*
compiles code to KWAC (kworker assembly code)
*/
void compile(FILE *);
void first_compile(FILE *);
void second_compile();
TABLE_ENTRY_PTR find_entry(char ,unsigned int ,char *,int );
int find_location(char ,unsigned int ,char *,int );
int EV_POSTFIX_EXPP(char *);

TABLE_ENTRY_PTR create_new(char , unsigned int ,char *,long);
//global vars
	static STACKPTR stack = NULL;
	static STACKPTR for_stack = NULL;
	static STACKPTR if_stack = NULL;
	static STACKPTR return_stack = NULL;
	static STACKPTR if_out_stack = NULL;
	static STACKPTR stuck = NULL;
	static STACKPTR elif = NULL;
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
//	
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
	file=precompile(file);
	compile(file);
	memset(flags,-1,sizeof(flags));
	return 0;
}
FILE *precompile(FILE *file){
	char *libs[MAX_LIBS];
	unsigned int functions[MAX_LIB_FUNCTIONS];
	memset(functions,0u,sizeof functions);

	unsigned int functions_added[MAX_LIB_FUNCTIONS];
	memset(functions_added,0u,sizeof functions_added);
	int lib_point=0;
	FILE * temp = tmpfile();
	if(!temp){
		fprintf(stderr,"ERROR while creating temp file");
		abort();
	}
	char line[100];
	while(!feof(file))
	{
		fgets(line,100,file);
		char *rest = line;
		while(rest[0]==' ')rest++;
		if(!strncmp(rest,"#include ",sizeof("#include ")-1)){
			rest+=(sizeof("#include ")-1);

			char temp_s[100];
			int x=0;
			while(isprint(rest[0]))temp_s[x++]=rest++[0];
			temp_s[x]='\0';
			libs[lib_point]=malloc(sizeof(temp_s));
			strcpy(libs[lib_point++],temp_s);
			//create a array of included libs so we can search for functions there later on
		}
		else{
			fprintf(temp,rest);
			//else just coppy line from orig file to the temp one
			while(strstr(rest, "CALL "))
			{
			char *function_name = strstr(rest, "CALL ")+(sizeof("CALL ")-1);	
			rest = function_name;
			while(rest[0]!='{')rest++;
			rest++[0]='\0';
			char *temp_line;
			int key = (hash((unsigned char*)function_name)%MAX_LIB_FUNCTIONS)|(1u<<32);
			if(!functions[key]){
				functions[key]=hash((unsigned char*)function_name);
			}
			}
		}
	}
	fprintf(temp,"\n");
	addFunctions(temp,functions,functions_added,libs,lib_point);


	rewind(temp);
	return temp;
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
	line = malloc(100);
	operator = malloc(20);
	operand = malloc(20);
	//fucn_name = malloc(50);
	rest= malloc(100);
	fucn_name = malloc(100);

	strcpy(fucn_name,"main"); //main by defual
	while(!feof(file))
	{
	int save_line=1;
	fgets(line,100,file);
	if(line[0]=='\0')continue;
	char *saved = rest;
	strcpy(rest,line);
	//fscanf(file,"%100s\n",line);
	//printf("%s\n",line );

	while(line[0]==' ' || line[0]=='\t')line++;
	operator = strtok(line," ");
	operand = strtok(NULL," ");
	//if(operand == NULL)continue;
	line_n++;
	if(operator[0]=='/' && operator[1] == '/'){
		line_n--;
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
			strcpy(fucn_name,operand);
			char *fucntion_name = strtok(fucn_name,"(");
			char *function_args  = rest;
			while(function_args[0]!='\0' && function_args[0]!='(') function_args++;
			if(function_args[0]=='(')function_args++;
			function_args=strtok(function_args,")");
			char *arg  = strtok(function_args,ARG_SEPARATOR);
			int number_of_args=0;
			char temp[50];
			unsigned int params[123];
			int p=0;
			while(arg!=NULL){
				params[p++]=hash((unsigned char*)arg);
				arg  = strtok(NULL,ARG_SEPARATOR);
			}
			while(p>0){
				TABLE_ENTRY_PTR var = create_new('V',params[--p],fucn_name,(function_pointer+MAX_STATIC_SIZE-(local_created++)));
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
		//this line is causing memory overwrite
		TABLE_ENTRY_PTR ret_ = create_new('V',0,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created+1) ));
		char command[50];
		if(!isprint((int)r_expression[0]))continue;
		sprintf(command,"POP %ld",ret_->location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		UPDATE_IF_BLOCKS(1)
		int r_value_adress = EV_POSTFIX_EXPP(r_expression);
		sprintf(command,"PUSH %d",r_value_adress);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		sprintf(command,"CALL %ld",ret_->location);
		symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
		free(ret_);
		
		//todo do while search for next function in file
	}
	else if(!strcmp(operator,"input")){
		char KWAC_COMMAND [50];
		sprintf(KWAC_COMMAND,"%s %d",input,EV_POSTFIX_EXPP(operand));
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
	else if(!strcmp(operator,"if") || (!strcmp(operator,"else") && operand[0]=='i' && operand[1]=='f')){
			//only if we have else if consturction
			if(isEmpty(elif))push(1,&elif);
			if(!strcmp(operator,"else")){
				//inELSE=1;
				operand = strtok(NULL," ");
				if(isEmpty(if_stack)){
					fprintf(stderr,"else statment not followed by previus if block on line %d\n",line_n+1);
					abort();
				}
				int adress = pop(&if_stack) -1;
				if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
					push(adress,&(returns[total_comands]));

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
			while(rest[0]!='!'&&
				  rest[0]!='='&&
				  rest[0]!='<'&&
				  rest[0]!='>'
			)comparingExpp[x++] = *rest++;
			comparingExpp[x++]=')';
			while(
				  rest[0]=='='||
				  rest[0]=='<'||
				  rest[0]=='>'||
				  rest[0]=='!'
			)comparingExpp[x++] = *rest++;
			comparingExpp[x++]='(';
			while(isprint((int)rest[0]))comparingExpp[x++]=*rest++;
			comparingExpp[x]=')';
			comparingExpp[x+1]='\0';

			EV_POSTFIX_EXPP(comparingExpp);
			//result is 0/1 in acc
			char command[30];
			sprintf(command,"BRANCHZERO "); //cond jump to the end of if block
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));	


			push(total_comands,&if_stack);
			if(!strcmp(operator,"else")){
				push(0,&elif);
			}
			else{
				push(1,&elif);
			}
			//flags[function_pointer+local_comands+2] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
		}
		else if(!strcmp(operator,"else\r\n")){
			//todo
			if(isEmpty(if_stack)){
					fprintf(stderr,"else statment not followed by previus if block on line %d\n",line_n+1);
					abort();
			}
			int adress = pop(&if_stack) -1;
			if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
			push(adress,&(returns[total_comands]));

		}
		else if(line[0]=='{'){
			if(!strcmp(last_line,"if") || !strcmp(last_line,"else") || !strcmp(last_line,"else\r\n")){
				push(IF,&stuck);
			}
			else push(FOR,&stuck);
			ELSE = 1; //show that we intered if block
		}
		else if(line[0]=='}'){
				if(!isEmpty(stuck)){
					if_or_for = peek(&stuck);
				}
				else{
					perror("Extra bracket found\n");
				}
				//this line causes fucking behavior fix uit 
				if(!isEmpty(if_stack) && flags[peek(&if_stack)]==IF && if_or_for==IF){
						symbolTable[total_comands++] = create_new('L',0,"BRANCH ",function_pointer+(local_comands++));
						push(total_comands,&if_out_stack);
						UPDATE_IF_BLOCKS(1)
						R=1;
				}
				//to do add last check if its loop closing bracket
				if(!isEmpty(stack) && !isEmpty(return_stack) &&if_or_for){
				if(!isEmpty(if_out_stack)) if_or_for=IF;
				int adress = pop(&stack) -1; //exit of loop
				int adress2 = pop(&return_stack) -1; //begining of loop
				char command[30];
				if(loop_inc_pointer>0){
					EV_POSTFIX_EXPP(loop_increments[--loop_inc_pointer]);
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
			//poping here results in incorect if exit!
			//but if we delete it it fucks of the entire loop
			//tho i might fixed it
			if(!isEmpty(stuck)){
					pop(&stuck);
			}
			ELSE=0;
	
		}
		else if(!strcmp(operator,"while")){
			char *left_eq = malloc(50);
			char *right_eq = malloc(50);
			char *comparator = malloc(3);
			int x=0;
			int y=0;
			while(operand[x]!='!' ||
				 operand[x]!='='  ||
				 operand[x]!='<'  ||
				 operand[x]!='>')
			{
			left_eq[x]=operand[x];
			x++;
			}
			comparator[0]=operand[x++];
			if(operand[x]=='=' || operand[x]=='<' || operand[x]=='>'){
				comparator[1]=operand[x++];
			}
			while(operand[x]!='\0'){
				right_eq[y++] = operand[x++];
			}

			free(left_eq);
			free(right_eq);
			free(comparator);
		}

		else if(!strcmp(operator,"for")){
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
					symbolTable[var_adress] = temp;
			}
			var_adress = symbolTable[var_adress]->location;

			//this line is under investigation !
			//why did i code it this way
			int adress = EV_POSTFIX_EXPP(left_side);
			if(adress==-1){
				//some shit happend i duno lol
			}
			else if(adress>=9200){
				fprintf(stderr,"lvalue required as left operand of assignment\n");
			}
			char command[40];


			
			//evaluate RHS//
			EV_POSTFIX_EXPP(right_side);
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
			EV_POSTFIX_EXPP(comparingExpp);
			//result in acc 0/1
			//jump to end if false
			sprintf(command,"BRANCHZERO ");
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			flags[total_comands]=FOR;
			push(total_comands,&stack); 

			//free(defaultValueExpp);
			free(comparingExpp);
			//free(incrementExpp);
		}

		else if(!strcmp(operator,"putc")){
			
			char temp[40];
			while(rest[0]!=' ')rest++;
			sprintf(temp,"PRINT %d",EV_POSTFIX_EXPP(rest));
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			UPDATE_IF_BLOCKS(1);
		}
		else if(!strcmp(operator,"put")){
			char temp[40];
			while(rest[0]!=' ')rest++;
			sprintf(temp,"WRITE %d",EV_POSTFIX_EXPP(rest));
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
			if(var_n[0]=='@'){
				var_n++;
				//get adress where we store
				//make sure that var_n doesnt have whitespaces in it to prevent undef var error
				int adress = EV_POSTFIX_EXPP(var_n);
				//evaluate right side of equation
				EV_POSTFIX_EXPP(equation);
				//result in acc
				char temp[40];
				sprintf(temp,"PSTORE %d",adress);
				symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));

			}

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
				int adress = MAX_CODE_SIZE-(total_const) -50;
				symbolTable[adress] = temp_v;

				 // sprintf(temp,"LOAD %ld",var->location);
				 // symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
				while (array_sub)
				{
				    save=array_sub;
				    array_sub=strtok(NULL,"[");
				    if(!array_sub){
					//if save was last array_subsriction
					//we evaluate array subsription and add it with an adress of last element
				    EV_POSTFIX_EXPP(strtok(save,"]"));	
					sprintf(temp,"ADD %ld",var->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
					sprintf(temp,"STORE %ld",temp_v->location);
					symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
	
				    }
				    else
				    {
					// if there're more array subs
					// add adress with substicption (offset)
				    EV_POSTFIX_EXPP(strtok(save,"]"));	
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
				EV_POSTFIX_EXPP(equation);
				sprintf(temp,"PSTORE %ld",temp_v->location);
				symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			}
			else
			{
			// if not found create new
				if(var==NULL)
				{
				var = create_new('V',hash_value,fucn_name,(function_pointer+MAX_STATIC_SIZE-(local_created++)));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = var;
				}	
			EV_POSTFIX_EXPP(equation);
			//UPDATE_IF_BLOCKS(total_comands - save);
			sprintf(temp,"STORE %ld",var->location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			}	
		}
		}
		else if(!strcmp(operator,"NEW_THREAD")){

		}


		else if(isprint(line[0])){
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
	for(int a=0;a<MAX_CODE_SIZE;a++){
		if(symbolTable[a]!=NULL){
			#ifdef _DEBUG
			Table_EntryToString(symbolTable[a]);
			#endif
			switch (symbolTable[a]->type){
				case'A':
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

}
int AllocateArray(STACKPTR *stack){
	//better check if stack is empty but i dont fucking care!
	int array_size = pop(stack);
	int zero_element = reserveMemory(array_size);

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
			TABLE_ENTRY_PTR pointer = create_new('A',AllocateArray(stack),fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));
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


int reserveMemory(int size){
	for(int a=0;a<size;a++){
		TABLE_ENTRY_PTR array_element = create_new('R',0,fucn_name,MAX_CODE_SIZE - total_const++);
		symbolTable[MAX_CODE_SIZE-(++total_vars)] = array_element;
	}
	//return adress in table
	return MAX_CODE_SIZE-(total_vars);
}
TABLE_ENTRY_PTR find_entry(char type,unsigned data,char *fucn_name,int total_vars){
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
TABLE_ENTRY_PTR create_new(char type,unsigned int data,char *fucn_name,long location){
	TABLE_ENTRY_PTR new = malloc(sizeof (TABLE_ENTRY));
	new->location=location;
	new->type=type;
	new->const_value=data;
	new->symbol=data;
	strcpy(new->fucn_name,fucn_name);
	return new;
}
int find_location(char type,unsigned int data,char *fucn_name,int total_vars){
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
int EV_POSTFIX_EXPP(char *expp){
	//todo after func call doesnt evalueate the rest of the expression
	if(expp[0]=='"'){
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
		int array_pointer = AllocateArray(&array_sizes);
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
		return array_pointer;
	}


	char *postfix = convertToPostfix(expp);
		STACKPTR stack = new_stack();
		STACKPTR operations = new_stack();
		enum OPERATIONS{UNARY,BINARY};
		int created = total_vars;
		int code_lines=0;
		int negative_number=1;
		while(postfix!=NULL &&  postfix[0]!='\0'){
			while(isspace((int)postfix[0]))postfix++;
			if(isdigit((int)postfix[0]) || (postfix[0]=='-' && negative_number)){
				char *dig = malloc(50);
				int x=0;
				while(postfix[0]!='\0' && !isspace((int)postfix[0]) && (!isOperator(postfix[0]) || negative_number ) ){
					dig[x++]=postfix[0];
					postfix++;
				}
				dig[x]='\0';
				int c_value = atoi(dig);
				int ad = find_location ('C',c_value,fucn_name,total_vars);
				if(ad<0){
					TABLE_ENTRY_PTR CONST = create_new('C',c_value,fucn_name,MAX_CODE_SIZE - total_const++);
					ad = MAX_CODE_SIZE-(++total_vars);
					symbolTable[ad]=CONST;
				}
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
			else if (isOperand(postfix[0])){
				if(postfix[0]=='C' && postfix[1]=='A' && postfix[2]=='L' && postfix[3]=='L'){
					//cut 'CALL'away from the beggining of the string
					postfix+=4;
					int x=0;
					char *fucntion_name = malloc(MAX_FUNC_L);
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
					x=0;
					int y=0;
					//while last closing bracket is found
					int last_bracket = strrchr(postfix,'}') - postfix;
					while(y<last_bracket){
						if (postfix[0]=='{'){
							bracket++;
							arguments[x++]=postfix[0];
						}
						else if (postfix[0]=='}'){
							bracket--;
							arguments[x++]=postfix[0];
						}
						else if (postfix[0]==',' &&postfix[-1]!='\'' && !bracket){
							arguments[x++]='$';
						}
						else
						{
						arguments[x++]=postfix[0];
						}
						postfix++;
						y++;
					}
					postfix++;
					char *arg = strtok(arguments,"$");
					int number_of_args =0;
					char *args [MAX_ARGS];
					while(arg!=NULL){
						args[number_of_args++]=arg;
						arg=strtok(NULL,"$");
					}
					char command[40];
					//push return adress of the function calling
					TABLE_ENTRY_PTR function = find_entry('F',total_comands+1,fucn_name,total_vars);
					if(function==NULL){
						function = create_new('C',total_comands+1,fucn_name,MAX_CODE_SIZE - total_const++);
						if(symbolTable[MAX_CODE_SIZE-(1+total_vars)]){
							fprintf(stderr,"memory overide caused by return adress");
							abort();
						}
						symbolTable[MAX_CODE_SIZE-(++total_vars)] = function;
					}

					sprintf(command,"PUSH %ld",function->location);
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					//push the arguments on the stack
					UPDATE_IF_BLOCKS(1)
					for(int a=0;a<number_of_args;a++){
						//evaluate epression
						int adress = EV_POSTFIX_EXPP(args[a]);
						char command[40];
						//and push it on the stack
						sprintf(command,"PUSH %d",adress);
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
					TABLE_ENTRY_PTR temp = create_new('V',0,fucn_name,MAX_CODE_SIZE - created - 50);
					int adress = MAX_CODE_SIZE-(created) -50;
					symbolTable[adress] = temp;
					sprintf(command,"POP %d",adress);
					symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
					push(adress,&stack);
					// todo pop return value from stack and store in temp var
					function->const_value = function_pointer+ local_comands-1; 


					
				}
				else if(postfix[0]==' ')postfix++;
				else{
					//if var name 
					unsigned char *var_name = (unsigned char*)postfix;
					while(postfix[0]!='\0' && (isalpha(postfix[0])||postfix[0]=='_')){
						postfix++;
					}
					char temp_c=postfix[0];
					postfix[0]='\0';
					printf("");
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
				EV_POSTFIX_EXPP(array_sub);
				//push(temp,&stack); //push adress of temp var
				int arr_p= pop(&stack);
				push(UNARY,&operations);
				push(-1,&stack);
				push(arr_p,&stack);
				//so we sum it up with array pointer
				//then dereference
			}
			else if (isOperator(postfix[0]))
			{
				int x=0;
				int y=0;
				int pstore=0;
				int flag=0;
				if(!isEmpty(stack))x=pop(&stack);
				if(!isEmpty(stack) &&

				 ((isEmpty(operations) || pop(&operations)==BINARY))

				 ){
					y=pop(&stack);
					if(y>0)flag=1;
				}
				/*we pop adress of vars in table here*/ 
				created++;
				TABLE_ENTRY_PTR temp = create_new('V',0,fucn_name,MAX_CODE_SIZE - created - 50);
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
				memset(temp_s,0,sizeof temp_s);
				sprintf(command2,"STORE %ld",temp->location);/*result of all operations is stored in acc*/ 
				//we only load it if not unary operation
				if(flag > 0){
					sprintf(load,"LOAD %ld",symbolTable[y]->location); 
				}
				code_lines++;
				int t = postfix[1]==' ';
				int comp = (int) (postfix[0]) + ((postfix[1+t]=='=' ||  postfix[1+t]=='<' || postfix[1+t]=='>'|| postfix[1+t]=='+'|| postfix[1+t]=='-')*postfix[1+t]);
				comp+=(postfix[0]=='!')*(postfix[1+t]=='=');
				switch ( comp ){
					case'+':
					sprintf(command,"ADD %ld",symbolTable[x]->location);
					push(BINARY,&operations);
					break;
					case'-':
					sprintf(command,"SUB %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break;
					case'*':
					sprintf(command,"MUL %ld",symbolTable[x]->location); 
					push(BINARY,&operations);
					break;
					case'/':
					sprintf(command,"DIV %ld",symbolTable[x]->location); 
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
					sprintf(command,"LOG_LESS %d",0); 
					push(BINARY,&operations);
					break; 
					case'>':
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(temp_s,"SUB %ld",symbolTable[y]->location);
					sprintf(command,"LOG_LESS %d",0); 
					push(BINARY,&operations);
					break;
					case'<'+'=':
					sprintf(temp_s,"SUB %ld",symbolTable[x]->location);
					sprintf(command,"LOG_LESSEQ %d",0); 
					push(BINARY,&operations);
					break; 
					case'>'+'=':
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(temp_s,"SUB %ld",symbolTable[y]->location);
					sprintf(command,"LOG_LESSEQ %d",0); 
					push(BINARY,&operations);
					break;
					case '='+'=':
					sprintf(load,"LOAD %ld",symbolTable[x]->location);
					sprintf(temp_s,"SUB %ld",symbolTable[y]->location);
					sprintf(command,"LOG_INV %d",0); 
					push(BINARY,&operations);
					break;
					case '!'+'='+1:
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
					sprintf(command,"ADD %ld",symbolTable[adress]->location);
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
					sprintf(command,"SUB %ld",symbolTable[adress]->location);
					sprintf(command2,"STORE %ld",symbolTable[x]->location);/*overwrite same var*/ 
					push(UNARY,&operations);
					adress=x;
					break;
				}
				postfix+= 1 + (comp>'@')*2;
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
		if(result>=0){
			char command[40];
			sprintf(command,"LOAD %ld",symbolTable[result]->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			if(code_lines==0){UPDATE_IF_BLOCKS(1)}
			/*todo: clear all temp vars */ 
			free(stack);
			return symbolTable[result]->location;
		}\
		else{
			fprintf(stderr,"Stack is empty\n");
			return -1;
		}
	}

