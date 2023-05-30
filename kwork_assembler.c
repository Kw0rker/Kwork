// 08.30.2021 
// Code assembler for kwork kernel by Ruslan Khamroev (c) no rights reserved no waranty
// ruslankhamroev@gmail.com
//-------------------------------------------------------------------------------------

#ifndef DEF_FILE_OUT_NAME
#define DEF_FILE_OUT_NAME "out.kw"
#endif
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int translate_code(FILE *,FILE *);
int getLine(char*,FILE *);
#define MAIN "0000"
#define FUNC "1111"
enum operators{READ,WRITE,PRINT,LOAD,STORE,ADD,SUB,DIV,MUL,MOD,BRANCH,
			  BRANCHNEG,BRANCHZERO,HALT,DEBUG,SYSCALL,BIT_S_R,BIT_S_L,
			  BIT_OR,BIT_AND,BIT_XOR,BIT_INV,PUSH,POP,CALL,PLOAD,PSTORE,LOG_LESS,LOG_LESSEQ,LOG_INV,ADD_F,SUB_F,MUL_F,DIV_F,WRITE_F,CAST_L_D,LOAD_F,LOG_LESS_F,LOG_LESSEQ_F};
int opcodes[] = {10,11,12,20,21,30,31,32,33,34,40,41,42,43,60,70,50,51,52,53,54,55,71,72,44,22,23,80,81,82,35,36,37,38,13,61,24,83,84};
// @argv[0] file_name.kwac
// @argv[1] output_name.kw
int main(int argc, char const *argv[])
{
	printf("%s\n",argv[1] );
	FILE *fp;
	FILE *out;

	//open file with kwork assembly code
	fp = fopen(argv[1],"r");
	if (fp==NULL)
	{
		perror("Could not open file specifed\n");
		return -1;
	}
	// if name is specifed creates output file with this name
	// otherwise uses default name 
	out = fopen(DEF_FILE_OUT_NAME,"w");
	//else out = fopen(argv[1],"w");
	if (out==NULL)
	{
		perror("Could not create file for write\n");
		return -1;
	}


	translate_code(fp,out);


	fclose(fp);
	fclose(out);
	return 0;
}
//translates kwork assembly code to kwork file 
translate_code(FILE *inp,FILE *out){
	//char *command[50];
	//char *save =command;
	long operand;
	while(!feof(inp)){
		char command[50];
		fscanf(inp,"%s",command);
		//printf("%s\n",command);
		fscanf(inp,"%ld",&operand);
		//fscanf(inp,"%s\n",command);//vuln to bufferoverflow fix later
		//todo change this ugly code
			 if(!strcmp(command,"READ"))fprintf(out, "%d%d\n",opcodes[READ],operand);
		else if(!strcmp(command,"WRITE"))fprintf(out, "%d%d\n",opcodes[WRITE],operand);
		else if(!strcmp(command,"WRITE_F"))fprintf(out, "%d%d\n",opcodes[WRITE_F],operand);
		else if(!strcmp(command,"PRINT"))fprintf(out,"%d%d\n",opcodes[PRINT],operand);
		else if(!strcmp(command,"LOAD"))fprintf(out, "%d%d\n",opcodes[LOAD],operand);
		else if(!strcmp(command,"LOAD_F"))fprintf(out, "%d%d\n",opcodes[LOAD_F],operand);
		else if(!strcmp(command,"STORE"))fprintf(out, "%d%d\n",opcodes[STORE],operand);
		else if(!strcmp(command,"ADD"))fprintf(out, "%d%d\n",opcodes[ADD],operand);
		else if(!strcmp(command,"ADD_F"))fprintf(out, "%d%d\n",opcodes[ADD_F],operand);
		else if(!strcmp(command,"SUB"))fprintf(out, "%d%d\n",opcodes[SUB],operand);
		else if(!strcmp(command,"SUB_F"))fprintf(out, "%d%d\n",opcodes[SUB_F],operand);
		else if(!strcmp(command,"DIV"))fprintf(out, "%d%d\n",opcodes[DIV],operand);
		else if(!strcmp(command,"DIV_F"))fprintf(out, "%d%d\n",opcodes[DIV_F],operand);
		else if(!strcmp(command,"MUL"))fprintf(out, "%d%d\n",opcodes[MUL],operand);
		else if(!strcmp(command,"MUL_F"))fprintf(out, "%d%d\n",opcodes[MUL_F],operand);
		else if(!strcmp(command,"MOD"))fprintf(out, "%d%d\n",opcodes[MOD],operand);
		else if(!strcmp(command,"BRANCH"))fprintf(out, "%d%d\n",opcodes[BRANCH],operand);
		else if(!strcmp(command,"BRANCHNEG"))fprintf(out, "%d%d\n",opcodes[BRANCHNEG],operand);
		else if(!strcmp(command,"BRANCHZERO"))fprintf(out,"%d%d\n",opcodes[BRANCHZERO],operand);
		else if(!strcmp(command,"HALT"))fprintf(out, "%d%d\n",opcodes[HALT],operand);
		else if(!strcmp(command,"DEBUG"))fprintf(out, "%d%d\n",opcodes[DEBUG],operand);
		else if(!strcmp(command,"SYSCALL"))fprintf(out, "%d%d\n",opcodes[SYSCALL],operand);
		else if(!strcmp(command,"BIT_OR"))fprintf(out, "%d%d\n",opcodes[BIT_OR],operand);
		else if(!strcmp(command,"BIT_AND"))fprintf(out, "%d%d\n",opcodes[BIT_AND],operand);
		else if(!strcmp(command,"BIT_INV"))fprintf(out, "%d%d\n",opcodes[BIT_INV],operand);
		else if(!strcmp(command,"BIT_XOR"))fprintf(out, "%d%d\n",opcodes[BIT_XOR],operand);
		else if(!strcmp(command,"BIT_S_L"))fprintf(out, "%d%d\n",opcodes[BIT_S_L],operand);
		else if(!strcmp(command,"BIT_S_R"))fprintf(out, "%d%d\n",opcodes[BIT_S_R],operand);
		else if(!strcmp(command,"PUSH"))fprintf(out, "%d%d\n",opcodes[PUSH],operand);
		else if(!strcmp(command,"POP"))fprintf(out, "%d%d\n",opcodes[POP],operand);
		else if(!strcmp(command,"CALL"))fprintf(out, "%d%d\n",opcodes[CALL],operand);
		else if(!strcmp(command,"PLOAD"))fprintf(out, "%d%d\n",opcodes[PLOAD],operand);
		else if(!strcmp(command,"PSTORE"))fprintf(out, "%d%d\n",opcodes[PSTORE],operand);
		else if(!strcmp(command,"PUSH"))fprintf(out, "%d%d\n",opcodes[PUSH],operand);
		else if(!strcmp(command,"POP"))fprintf(out, "%d%d\n",opcodes[POP],operand);
		else if(!strcmp(command,"CALL"))fprintf(out, "%d%d\n",opcodes[CALL],operand);
		else if(!strcmp(command,"LOG_LESS"))fprintf(out, "%d%d\n",opcodes[LOG_LESS],operand);
		else if(!strcmp(command,"LOG_LESS_F"))fprintf(out, "%d%d\n",opcodes[LOG_LESS_F],operand);
		else if(!strcmp(command,"LOG_LESSEQ"))fprintf(out, "%d%d\n",opcodes[LOG_LESSEQ],operand);
		else if(!strcmp(command,"LOG_LESSEQ_F"))fprintf(out, "%d%d\n",opcodes[LOG_LESSEQ_F],operand);
		else if(!strcmp(command,"LOG_INV"))fprintf(out, "%d%d\n",opcodes[LOG_INV],operand);
		else if(!strcmp(command,"CAST_L_D"))fprintf(out, "%d%d\n",opcodes[CAST_L_D],operand);
		else if(!strcmp(command,"MAIN"))fprintf(out, "%s\n",MAIN);
		else if(!strcmp(command,"FUNC"))fprintf(out, "%s\n",FUNC);
		//sets up addres datta at address
		else if(command[0]=='#'){
			fprintf(out, "-%s\n",command+1);
			fscanf(inp,"%ld",&operand);
			fprintf(out, "%ld\n",operand);
		}
		//sets up adress for folowing command
		else if (command[0]=='&')
		{
			fprintf(out, "-%s\n",command+1);
		}
		else{
			fprintf(stderr,"Undefined command -> %s\n",command);
		}
	

	}
	//free(save);
}
int getLine(char *buf,FILE *pt){
	char c;
	while( (c=getc(pt)!='\n')){
		buf=c;
		buf++;
	}
	buf='\0';
}