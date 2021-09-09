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
#define READ 0
#define WRITE 1
#define PRINT 2
#define LOAD 3
#define STORE 4
#define ADD 5
#define SUB 6
#define DIV 7
#define MUL 8
#define MOD 9
#define BRANCH 10
#define BRANCHNEG 11
#define BRANCHZERO 12
#define HALT 13
#define DEBUG 14
#define SYSCALL 15
#define MAIN "0000"
#define FUNC "1111"

int opcodes[] = {10,11,12,20,21,30,31,32,33,34,40,41,42,43,60,70};
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
	char *command= malloc(50);
	int operand;
	while(!feof(inp)){
		fscanf(inp,"%s",command);
		//printf("%s\n",command);
		fscanf(inp,"%d",&operand);
		//fscanf(inp,"%s\n",command);//vuln to bufferoverflow fix later

		if(!strcmp(command,"READ"))fprintf(out, "%d%d\n",opcodes[READ],operand);
		else if(!strcmp(command,"WRITE"))fprintf(out, "%d%d\n",opcodes[WRITE],operand);
		else if(!strcmp(command,"PRINT"))fprintf(out,"%d%d\n",opcodes[PRINT],operand);
		else if(!strcmp(command,"LOAD"))fprintf(out, "%d%d\n",opcodes[LOAD],operand);
		else if(!strcmp(command,"STORE"))fprintf(out, "%d%d\n",opcodes[STORE],operand);
		else if(!strcmp(command,"ADD"))fprintf(out, "%d%d\n",opcodes[ADD],operand);
		else if(!strcmp(command,"SUB"))fprintf(out, "%d%d\n",opcodes[SUB],operand);
		else if(!strcmp(command,"DIV"))fprintf(out, "%d%d\n",opcodes[DIV],operand);
		else if(!strcmp(command,"MUL"))fprintf(out, "%d%d\n",opcodes[MUL],operand);
		else if(!strcmp(command,"MOD"))fprintf(out, "%d%d\n",opcodes[MOD],operand);
		else if(!strcmp(command,"BRANCH"))fprintf(out, "%d%d\n",opcodes[BRANCH],operand);
		else if(!strcmp(command,"BRANCHNEG"))fprintf(out, "%d%d\n",opcodes[BRANCHNEG],operand);
		else if(!strcmp(command,"BRANCHZERO"))fprintf(out,"%d%d\n",opcodes[BRANCHZERO],operand);
		else if(!strcmp(command,"HALT"))fprintf(out, "%d%d\n",opcodes[HALT],operand);
		else if(!strcmp(command,"DEBUG"))fprintf(out, "%d%d\n",opcodes[DEBUG],operand);
		else if(!strcmp(command,"SYSCALL"))fprintf(out, "%d%d\n",opcodes[SYSCALL],operand);
		else if(!strcmp(command,"MAIN"))fprintf(out, "%s\n",MAIN);
		else if(!strcmp(command,"FUNC"))fprintf(out, "%s\n",FUNC);
		//sets up addres datta at address
		else if(command[0]=='#'){
			command++;
			fprintf(out, "-%s\n",command);
			fscanf(inp,"%d",&operand);
			fprintf(out, "%d\n",operand);
		}
		//sets up adress for folowing command
		else if (command[0]=='&')
		{
			command++;
			fprintf(out, "-%s\n",command);
		}
		else{
			perror("Undefined command -> %s\n",command);
		}
	

	}
	free(command);
}
int getLine(char *buf,FILE *pt){
	char c;
	while( (c=getc(pt)!='\n')){
		buf=c;
		buf++;
	}
	buf='\0';
}