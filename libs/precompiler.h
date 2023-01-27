#define JOIN(A,B) A##B
#define LOAD_LIB(LIB_FILE,LIB_NAME) char temp_s[100];sprintf(temp_s,"%s/%s",LIB_PATH,LIB_NAME);LIB_FILE=fopen(temp_s,"r")


void addFunctions(FILE *source,unsigned int functions[],unsigned int added[],char**libs, int libs_ttl,char **prototypes)
{
	int lib_point=libs_ttl;
	char ASM_FLAG=0;
	int new_to_resolve=0;
	for(int a=0;a<libs_ttl;a++)
	{	
		LOAD_LIB(FILE *lib_file,libs[a]);
		if(!lib_file){
			fprintf(stderr,"Libary %s is not present\n",libs[a]);
			continue;
		}
		char flag=0;
		//open lib file
		char line[1000];
		while(!feof(lib_file))
		{
			fgets(line,1000,lib_file);
			char *rest = line;
			while(rest[0]==' ')rest++;
			//remove all white spaceses in the begining of the string

			if(!strncmp(rest,"function",sizeof("function")-1))
			{
				flag=0;
				rest+=(sizeof("function ")-1);
				//tok the string
				while(rest[0]==' ')rest++;
				//remove whitespaces at the begining
				char *t = strchr(rest,'(');
				t[0]='\0';
				char *function_name = strtok(rest,"(");
				//get the function name
				if(((char *)strchrnul(function_name,' '))[0])while(function_name[0]!=' ')function_name++;
				while(function_name[0]==' ')function_name++;
				for (int i = 0; i < MAX_LIB_FUNCTIONS; ++i)
				{
					//if key exists
					if(functions[i])
					{	
						//check for hash match
						if(functions[i]==(unsigned int)(hash((unsigned char*)function_name)) )
						{
							//set flag to true so we coppy current line (function declaration)
							//and the following lines untill new function declaration
							flag=1;
							//set entry to 0 so we dont search for it anymore
							functions[i]=0;
							int key = (hash((unsigned char*)function_name)%MAX_LIB_FUNCTIONS)|(1u<<32);
							//mark that function has been added to the source file
							added[key]=1;
							//change total_lib_counter only if we found function we were looking for
							libs_ttl=lib_point;

						}
					}
				}
				t[0]='(';

			}
			else if(!strncmp(rest,"#include ",sizeof("#include ")-1)){
				rest+=(sizeof("#include ")-1);
				char temp_s[100];
				int x=0;
				while(isprint((int)rest[0]))temp_s[x++]=rest++[0];
				temp_s[x]='\0';
				char lib_included=0;
				//check if specific lib is already included
				for (int i = 0; i < libs_ttl&&!lib_included; ++i)
				{
					if(!strcmp(libs[i],temp_s))lib_included=1;
				}
				//if lib is included dont add
				if(lib_included)continue;
				//otherwise add
				if(!libs[lib_point])libs[lib_point]=malloc(sizeof(temp_s));
				strcpy(libs[lib_point++],temp_s);
				//create a array of included libs so we can search for functions there later on
				continue;
			}
			//if flag is set we coppy lines to the source file
			if(flag)
			{
				while(!isprint((int)rest[0]))rest++;//remove all special symbols at the beginign
				if(!strncmp(rest,"KASM",sizeof("KASM")-1)){
				fprintf(source,"%s",rest);	
				ASM_FLAG=1;
				//set asm flag so the following lines are translated directly to asm
				continue;
				}
				if(ASM_FLAG){
					//close asm block
					if(rest[0]=='}'){ASM_FLAG=0;fprintf(source,"%s",rest);continue;}
					fprintf(source,"%s",rest);
					continue;
				}
				//special case cos SYSCALL hapens to trigger the block for function calls

				//check if there is any inner function calls
				while(strstr(rest, "CALL "))
				{
				char *function_name = strstr(rest, "CALL ")+(sizeof("CALL ")-1);	
				rest = function_name;
				while(rest[0]!='{')rest++;
				rest[0]='\0';
				int key = (hash((unsigned char*)function_name)%MAX_LIB_FUNCTIONS)|(1u<<32);
				//if function hasnt been added yet
				if(!functions[key] &&!added[key]){
					functions[key]=hash((unsigned char*)function_name);
					new_to_resolve++;

				}
				//restore the original string component
				rest[0]='{';
				}
				//printf("%s\n",line );
				if(!strncmp(line,"function",sizeof("function")-1)){
					//add function prototype to array;
					char *proto = line+(sizeof("function")-1);
					*prototypes=malloc(100);
					strcpy(*(prototypes++),proto);
					

				}
				fprintf(source,line);
				fprintf(source,"\n");
			}
		}
		//go thry every function call that was made iside this lib function and resolve it
		fclose(lib_file);
	}
	if (new_to_resolve>0){
		//called only if lib function calls other functions inside of it
		addFunctions(source,functions,added,libs,libs_ttl,prototypes);
	}
}	