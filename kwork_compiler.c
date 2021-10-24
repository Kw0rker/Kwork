#include <kwork_params.h>
#include <assert.h>
#ifndef MAX_CODE_SIZE
#define MAX_CODE_SIZE 10000
#endif
#ifndef MAX_STATIC_SIZE
#define MAX_STATIC_SIZE 500 //max static space for each function, containging kworker assembly instructions and variables
#endif
#define input "READ \0"

int flags[MAX_CODE_SIZE]={0};
char *KWAC[MAX_CODE_SIZE];
enum TYPES{IF,FOR,WHILE};
//pass pointer to struct
#define Table_EntryToString(entry)												\
			printf("Struct table_entry{\nint const_value %d\nint symbol %d;\nchar type %c;\nlong location %ld;\nchar fucn_name %s;\n}\n",entry->const_value, entry->symbol,entry->type,entry->location,entry->fucn_name)		\

/*
use in main function only ! as it depends on local variables
@comp is comparator == or <= etc
*/
#define GET_VARS(comp) char *f;\
				char *s;\
				f = strtok(operand,#comp);\
				s = strtok(NULL,#comp);\
				if(isdigit((int)f[0])){\
					v1 = find_entry('C',atoi(f),fucn_name,total_vars);\
					if(v1==NULL){\
					v1= create_new('C',atoi(f),fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));\
					symbolTable[MAX_CODE_SIZE-(++total_vars)] = v1;}\
				}\
				else{\
					v1 = find_entry('V',f[0],fucn_name,total_vars);\
					if(v1==NULL){\
					v1 = create_new('V',f[0],fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));\
					symbolTable[MAX_CODE_SIZE-(++total_vars)] = v1;}\
				}\
				if(isdigit((int)s[0])){\
					v2 = find_entry('C',atoi(s),fucn_name,total_vars);\
					if(v2==NULL){\
					v2 = create_new('C',atoi(s),fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));\
					symbolTable[MAX_CODE_SIZE-(++total_vars)] = v2;}\
				}\
				else {\
					v2 = find_entry('V',s[0],fucn_name,total_vars);\
					if(v2==NULL){\
					v2 = create_new('V',s[0],fucn_name,(function_pointer+MAX_STATIC_SIZE- (local_created++)));\
					symbolTable[MAX_CODE_SIZE-(++total_vars)] = v2;}\
				}\
			
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
								if(  strcmp(operator,"else") && strcmp(operator,"else\r\n") && !ELSE){\
									while(!isEmpty(if_stack)){\
										int adress = pop(&if_stack)-1;\
										char *update = strcat(symbolTable[adress]->fucn_name,new_addres);\
										strcpy(symbolTable[adress]->fucn_name,update);\
									}\
									if(peek(&stuck)){\
										while(!isEmpty(if_out_stack)){\
											int adress = pop(&if_out_stack) -1;\
											char *new_command = strcat(symbolTable[adress]->fucn_name,new_addres);\
											strcpy(symbolTable[adress]->fucn_name,new_command);\
										}\
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
TABLE_ENTRY_PTR find_entry(char ,int ,char *,int );
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
	STACKPTR for_stack = new_stack();
	STACKPTR if_stack = new_stack();
	STACKPTR return_stack = new_stack();
	STACKPTR if_out_stack = new_stack();
	STACKPTR stuck = new_stack();
	STACKPTR elif = new_stack();
	char *line = malloc(100);
	STACKPTR returns[MAX_CODE_SIZE];
	int local_created=0;
	int function_pointer;
	int line_n=-1;
	int R = 0;
	int ELSE = 0; //shows if were insdie if else block
	int local_comands =0 ;//number of commands in function and i have no fucking clue why it has to be  -8 lmao im fucking retard but it works this way
	int total_vars=0; //total ammount of vars declareted
	int total_comands = 0;
	char *operator = malloc(20);
	char *operand = malloc(20);
	char *fucn_name = malloc(50);
	char last_line[100];
	fucn_name = "main"; //main by defual
	while(!feof(file))
	{
	R=0;
	fgets(line,100,file);
	char *saved = line;
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
		// line_n=-1;
		// while(!feof(file)){
		// 	fscanf(file,"%100s\n",line);

		// 	operator = strtok(line," ");
		// 	operand = strtok(NULL," ");
		// 	if(!strcmp(operator,"function")){
		// 		fucn_name=operand;
		// 		TABLE_ENTRY_PTR func = malloc(sizeof(TABLE_ENTRY));
		// 		func->type='L';
		// 		if(!strcmp(operand,"main\r\n")){
		// 			function_pointer=0;
		// 			strcpy(func->fucn_name,"MAIN");
		// 		}
		// 		else {
		// 		function_pointer = function_pointer + MAX_STATIC_SIZE + 1;
		// 		strcpy(func->fucn_name,"FUNC");
		// 		}
		// 	func->location=function_pointer;
		// 	symbolTable[total_comands++] = func;
		// 	break; //we found next function now repead
		// 	}
		// }
		symbolTable[total_comands++] = create_new('L',0,"HALT 00",function_pointer+(local_comands++));
		UPDATE_IF_BLOCKS(1)
		//todo do while search for next function in file
	}
	else if(!strcmp(operator,"input")){
		TABLE_ENTRY_PTR table_entry =find_entry('V',operand[0],fucn_name,total_vars);
		// if not found create new
		if(table_entry==NULL){
			table_entry = create_new('V',operand[0],fucn_name,(function_pointer+MAX_STATIC_SIZE-(local_created++)));
			symbolTable[MAX_CODE_SIZE-(++total_vars)] = table_entry;
		}
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

			TABLE_ENTRY_PTR v1;
			TABLE_ENTRY_PTR v2;
			char command[30];
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
				GET_VARS(==)
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
				if(pop(&elif)){
					UPDATE_IF_BLOCKS(4)
					push(total_comands,&if_stack);
				}
				else{
					push(total_comands,&if_stack);
					UPDATE_IF_BLOCKS(4)
				}
			


			}
			else if(!strcmp(comparator,"<=")){
				GET_VARS(<=)
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

				sprintf(command,"BRANCHNEG %d",function_pointer+local_comands+3); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+4] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				if(pop(&elif)){
					UPDATE_IF_BLOCKS(5)
					push(total_comands,&if_stack);
				}
				else{
					push(total_comands,&if_stack);
					UPDATE_IF_BLOCKS(5)
				}
				

			}
			else if(!strcmp(comparator,">=")){
				GET_VARS(>=)
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

				sprintf(command,"BRANCHNEG %d",function_pointer+local_comands+3); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+4] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				if(pop(&elif)){
					UPDATE_IF_BLOCKS(5)
					push(total_comands,&if_stack);
				}
				else{
					push(total_comands,&if_stack);
					UPDATE_IF_BLOCKS(5)
				}
			}

			else if(!strcmp(comparator,">")){
				GET_VARS(>)
				sprintf(command,"LOAD %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//branzero and branch neg jump over branch statement 
				
				sprintf(command,"BRANCHNEG %d",function_pointer+local_comands+2); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+local_comands+2] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				if(pop(&elif)){
					UPDATE_IF_BLOCKS(4)
					push(total_comands,&if_stack);
				}
				else{
					push(total_comands,&if_stack);
					UPDATE_IF_BLOCKS(4)
				}

			}
			else if(!strcmp(comparator,"<")){
				GET_VARS(<)
				sprintf(command,"LOAD %ld",v1->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				sprintf(command,"SUB %ld",v2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//branzero and branch neg jump over branch statement 
				
				sprintf(command,"BRANCHNEG %d",function_pointer+local_comands+2); //jump over next branch command
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"BRANCH "); // jump forward to the end of if block we define adress on second compilation
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//flags[function_pointer+line_n+3] = total_comands;
				//push adreess of brach command so we can resolve jump forward address once '}' encountered
				if(pop(&elif)){
					UPDATE_IF_BLOCKS(4)
					push(total_comands,&if_stack);
				}
				else{
					push(total_comands,&if_stack);
					UPDATE_IF_BLOCKS(4)
				}
			}
			symbolTable[MAX_CODE_SIZE - (++total_vars)] = v1;
			symbolTable[MAX_CODE_SIZE - (++total_vars)] = v2;
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
			int adress = pop(&if_stack) -1;
			if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
			push(adress,&(returns[total_comands]));

		}
		else if(line[0]=='{'){
			if(!strcmp(last_line,"if") || !strcmp(last_line,"else") || !strcmp(last_line,"else\r\n"))push(IF,&stuck);
			else push(FOR,&stuck);
			ELSE = 1; //show that we intered if block
		}
		else if(line[0]=='}'){
				int if_or_for=0;
				if(!isEmpty(stuck)){
					if_or_for = peek(&stuck);
				}
				else{
					perror("Extra bracket found\n");
				}
				if(!isEmpty(if_stack) && flags[peek(&if_stack)]==IF){
						symbolTable[total_comands++] = create_new('L',0,"BRANCH ",function_pointer+(local_comands++));
						push(total_comands,&if_out_stack);
						//UPDATE_IF_BLOCKS(1)
				}
				//to do add last check if its loop closing bracket
				if(!isEmpty(stack) && !isEmpty(return_stack) &&if_or_for){
				R=1;
				int adress = pop(&stack) -1; //exit of loop
				int adress2 = pop(&return_stack) -1; //begining of loop
				char command[30];
				sprintf(command,"BRANCH %ld",symbolTable[adress2]->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				UPDATE_IF_BLOCKS(1)
				memset(command,0,sizeof(command));
					//apped brancha adress
				if(returns[total_comands]==NULL)returns[total_comands] = new_stack();
				push(adress,&(returns[total_comands]));
			}
			if(!isEmpty(stuck)){
					pop(&stuck);
			}
			ELSE=0;
	
		}
		//check if line starts as for loop
		else if (!strcmp(operator,"for")){
			//for exmaple
			// for x=0;x<10;1
			//	init value;comparasing;step
			char *var_nd_defV = malloc(12); //variable and its asign value
			char *comparator = malloc(3);
			char *comparator_saved = comparator; //so we can free it's later after pointer ariphmetics been done
			char *var_nd_defV_saved = var_nd_defV;
			char *step = malloc(8);
			var_nd_defV = strtok(operand,";");
			comparator = strtok(NULL,";");
			step = strtok(NULL,";");
			int step_value = atoi(step);
			char var = var_nd_defV[0];
			char var2 = comparator[0];
			var_nd_defV+=2;//increment pointer so it points to var deflaut value
			comparator++;
			//increment pointer by one so it points to 
			//so next element afer variable must be equaltiy sign 1 or 2 bits 
			char comp[] = {comparator[0],
							   (comparator[1]=='>'||
							   	comparator[1]=='<'||
							   	comparator[1]=='=')?comparator[1]++:'\0', //increment by one if there 2 signs
							   '\0'};
			comparator++;//increment pointer so it points to compare value
			//if one ony equality sign found incrment only by one, if 2 increment by 2				   
			TABLE_ENTRY_PTR VAR1 = find_entry('V',var,fucn_name,total_vars);
			if(VAR1==NULL){
				VAR1 = create_new('V',var,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = VAR1;
			}
			TABLE_ENTRY_PTR VAR2 = find_entry('V',var2,fucn_name,total_vars);
			if(VAR2==NULL){
				VAR2 = create_new('V',var2,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = VAR2;
			}
			int compareValue = atoi(comparator);
			TABLE_ENTRY_PTR CMP_VALUE;
			if(isdigit((int)comparator[0])){
				CMP_VALUE = find_entry('C',compareValue,fucn_name,total_vars);
			}
			else{
				CMP_VALUE = find_entry('V',comparator[0],fucn_name,total_vars);
			}
			if(CMP_VALUE==NULL){
				CMP_VALUE = create_new(isdigit((int)comparator[0])?'C':'V',compareValue,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = CMP_VALUE;
			}
			int defValue = atoi(var_nd_defV);
			TABLE_ENTRY_PTR DEF_VALUE;
			if(isdigit((int)var_nd_defV[0])){
				DEF_VALUE =  find_entry('C',defValue,fucn_name,total_vars);
			}
			else{
				DEF_VALUE =  find_entry('V',var_nd_defV[0],fucn_name,total_vars);
			}
			if(DEF_VALUE==NULL){
				DEF_VALUE = create_new(isdigit((int)var_nd_defV[0])?'C':'V',defValue,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = DEF_VALUE;
			}
			TABLE_ENTRY_PTR step_ = find_entry('C',step_value,fucn_name,total_vars);
			if(step_==NULL){
				step_ = create_new('C',step_value,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = step_;
			}
			TABLE_ENTRY_PTR one = find_entry('C',1,fucn_name,total_vars);
			//check if vars and consts was intialized 
			if(one==NULL){
				one = create_new('C',1,fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++)));
				symbolTable[MAX_CODE_SIZE-(++total_vars)] = one;
			}


			char command[40];
			//generate kwork asm code //
			//			asign variable its default value -1 ////
			//			as we pre increment 				////
			sprintf(command,"LOAD %ld",DEF_VALUE->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			sprintf(command,"SUB %ld",one->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			sprintf(command,"STORE %ld",VAR1->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));

			// 				pre increment the value 			////
			//				we have to jump here 				////
			sprintf(command,"LOAD %ld",VAR1->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			push(total_comands,&for_stack);
			push(total_comands,&return_stack);
			sprintf(command,"ADD %ld",step_->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));
			sprintf(command,"STORE %ld",VAR1->location);
			symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
			memset(command,0,sizeof(command));

			//                                           ////

			if(!strcmp(comp,"<=")){
				//			subtract value of compare value form Variable in equality			////
				sprintf(command,"LOAD %ld",CMP_VALUE->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"SUB %ld",VAR2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//																				////

				//blind jump forward if result is negative meaning that var > compare_vale which reuslts in the end of the cycle
				sprintf(command,"BRANCHNEG ");
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				push(total_comands,&stack); //push adress of bracnh command in sybol table so whe can reslove jump adress later
				UPDATE_IF_BLOCKS(9)



			}
			else if(!strcmp(comp,">=")){
				//			subtract varaiable from value in eauality			////
				sprintf(command,"LOAD %ld",VAR2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"SUB %ld",CMP_VALUE->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//																				////

				//blind jump forward if result is negative meaning that var > compare_vale which reuslts in the end of the cycle
				sprintf(command,"BRANCHNEG ");
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				push(total_comands,&stack); //push adress of bracnh command in sybol table so whe can reslove jump adress later
				UPDATE_IF_BLOCKS(9)


			}
			else if(!strcmp(comp,"==")){
				//			subtract varaiable from value in eauality			////
				sprintf(command,"LOAD %ld",VAR2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"SUB %ld",CMP_VALUE->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//																				////

				//			jump over next instuction if result of subtraction is 0 			////

				sprintf(command,"BRANCHZERO %d",function_pointer+local_comands+2);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//     																			////

				//			jump to end of loop otherwise				////
				sprintf(command,"BRANCH ");
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				push(total_comands,&stack);
				UPDATE_IF_BLOCKS(10)

			}
			else if(!strcmp(comp,"<")){
				//			subtract value of compare value form Variable in equality			////
				sprintf(command,"LOAD %ld",VAR2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"SUB %ld",CMP_VALUE->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//																				////

				//if result is negative meaning that variable is less than compare value jump over next command//
				sprintf(command,"BRANCHNEG %d",function_pointer+local_comands+2);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//			jump to end of loop otherwise				////
				sprintf(command,"BRANCH ");
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				push(total_comands,&stack);
				UPDATE_IF_BLOCKS(10)



			}
			else if(!strcmp(comp,">")){
				//			subtract value of compare value form Variable in equality			////
				sprintf(command,"LOAD %ld",CMP_VALUE->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));

				sprintf(command,"SUB %ld",VAR2->location);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//																				////

				//if result is negative meaning that variable is less than compare value jump over next command//
				sprintf(command,"BRANCHNEG %d",function_pointer+local_comands+2);
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				//			jump to end of loop otherwise				////
				sprintf(command,"BRANCH ");
				symbolTable[total_comands++] = create_new('L',0,command,function_pointer+(local_comands++));
				memset(command,0,sizeof(command));
				push(total_comands,&stack);
				UPDATE_IF_BLOCKS(10)

								
			}
			flags[total_comands] = FOR;




			//it causes sigabart
			// free(var_nd_defV_saved);
			// free(comparator_saved);
			// free(step);
		}
		else if(!strcmp(operator,"putc")){
			TABLE_ENTRY_PTR VAR; 
			if(isdigit((int)operand[0])){
				VAR = find_entry('C',atoi(operand),fucn_name,total_vars);
				if(VAR==NULL){
					VAR = create_new('C',atoi(operand),fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
					symbolTable[MAX_CODE_SIZE-(++total_vars)] = VAR;
				}
			}
			else{
			 	VAR = find_entry('V',operand[0],fucn_name,total_vars);
				if(VAR==NULL){
					VAR = create_new('V',operand[0],fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
					symbolTable[MAX_CODE_SIZE-(++total_vars)]= VAR;
				}
			}
			char temp[40];
			sprintf(temp,"PRINT %ld",VAR->location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			UPDATE_IF_BLOCKS(1);
		}
		else if(!strcmp(operator,"put")){
			TABLE_ENTRY_PTR VAR = NULL; 
			if(isdigit((int)operand[0])){
			VAR = find_entry('C',atoi(operand),fucn_name,total_vars);
				if(VAR==NULL){
					VAR = create_new('C',atoi(operand),fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
					symbolTable[MAX_CODE_SIZE-(++total_vars)] = VAR;
				}
			}
			else{
			VAR = find_entry('V',operand[0],fucn_name,total_vars);
				if(VAR==NULL){
					VAR =  create_new('V',operand[0],fucn_name,(function_pointer+MAX_STATIC_SIZE - (local_created++) ));
					symbolTable[MAX_CODE_SIZE-(++total_vars)]= VAR;
				}
			}
			char temp[40];
			sprintf(temp,"WRITE %ld",VAR->location);
			symbolTable[total_comands++]=create_new('L',0,temp,function_pointer+(local_comands++));
			UPDATE_IF_BLOCKS(1);
		}



		else{
			//dont count number of commands if operation is not defined in compiler
			printf("command -> %s is not defined\n" ,line);
			//line_n--;
		}
		line=saved;
		strcpy(last_line,operator);
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

TABLE_ENTRY_PTR find_entry(char type,int data,char *fucn_name,int total_vars){
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
TABLE_ENTRY_PTR create_new(char type,int data,char *fucn_name,long location){
	TABLE_ENTRY_PTR new = malloc(sizeof (TABLE_ENTRY));
	new->location=location;
	new->type=type;
	new->const_value=data;
	new->symbol=data;
	strcpy(new->fucn_name,fucn_name);
	return new;
}