#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#ifndef FAILURE
#define FAILURE 1
#endif
#ifndef SUCSESS
#define SUCSESS 0
#endif
#define TEST(x) compare_files ("tests\\" #x "_input.kws", "tests\\" #x "_output.kwac")
int compare_files(char *,char *);
int cmp_command(char *,char *);
int compile_and_wait(char *);
char mode;
int main(int argc, char const *argv[])
{
	if(argc>1)mode=argv[1][1];
	printf("%-40s%-10s%-8s\n\n", "  Test", "Status","Line");
	return 
	(
	//write tests	
	TEST(if_inside_loop) + TEST(nested_if) + TEST (loop_inside_if)
	);
}
int compare_files(char *f1,char* f2){
	int suc=1;
	pid_t id = fork();
	int r=0;
	if(id==0){
		int fd = open("/dev/null", O_WRONLY);
    	dup2(fd, 1);    /* make stdout a copy of fd (> /dev/null) */
   	    close(fd);      /* close fd */

		r=execl("./kwork_compiler","kwork_compiler",f1,(char*)NULL);
		perror("cannot start compiler\n");
		if(r==-1){
			 printf("Oh dear, something went wrong with exec()! %s\n", strerror(errno));
		}
	}
	else{
		int *stat=0;
		waitpid(id,stat,0);
		//wait();
		int STATUS = SUCSESS;
		FILE *file1 = fopen(f2,"r");
		if(file1==NULL)fprintf(stderr, "No file found on adress->%s\n",f1 );
		FILE *file2 = fopen("input.kwac","r");
		if(file2==NULL)fprintf(stderr, "No assembly generated!\n");
		char line1[100];
		int line_n=0;
		char line2[100];
		while(!feof(file1) && !feof(file2)){
			line_n++;
			char t1[40];
			fscanf(file1,"%s%s",line1,t1);
			strcat(line1,t1);
			fscanf(file2,"%s%s",line2,t1);
			strcat(line2,t1);
			if(cmp_command(line1,line2)){
			fprintf(stderr,"| %-38s%-10s%08d\n",f1,"FAILED",line_n);
			if(mode=='F')fprintf(stderr,"%s-----\n%s",line1,line2);
			STATUS = FAILURE; //return failure even if one line is deferent
			if(mode =='E'){
				fprintf(stderr,"%s-----\n%s",line1,line2);
				return STATUS;
			}
			}
		}
	fclose(file1);fclose(file2);
	if(STATUS != FAILURE)fprintf(stdout,"| %-38s%-10s%-8s\n",f1,"PASSED","--------");
	return STATUS;
	}
	return SUCSESS;
}
int cmp_command(char* line, char *line2){
	return strcmp(line,line2);
	
}
int compile_and_wait(char *name){

}
