#include <stdio.h>
#include <math.h>
#define MEM_SIZE 1000
#define READ 10 // inserts data from terminal to memory address
#define WRITE 11 // prints data frm memory adress
#define PRINT 12 // prints a char from memory adress
#define LOAD 20 //inserts data to acc register
#define STORE 21 // stores data from acc to memory address
#define ADD 30 //sums acc with data in mem address and stores result in acc
#define SUB 31 //subtracts accc with data in mem adress result stored in acc
#define DIV 32 //divedes acc with data in mem adress result stored in acc
#define MUL 33 // multiplies acc with data in mem adress result stored in acc
#define MOD 34 //takes acc mod mem adress, result stored in acc
#define BRANCH 40 // go to mem adress 
#define BRANCHNEG 41 // go to mem adress if acc < 0 
#define BRANCHZERO 42 // go to mem adress if acc==0
#define HALT 43 //stop, executes after programs finished 
#define DEBUG 60//dumps memory when reached
/*
 * all commands saved followed by prevoius, strting at 0, if input is negative it counts as an adress for next command
 */
int main(){
	long double acc ;
	int instruction_counter;
	long double instruction_register;
	int operation_code;
	int operand;
	long double memory[MEM_SIZE];
	/*
	 * Beggining of initiaalization 
	 */
	acc =0;
	instruction_counter=0;
	instruction_register=0;
	operation_code=0;
	operand = 0;
	for(int i=0;i<MEM_SIZE;i++)memory[i]=0;
	
	int counter=0;
	printf("***Kworker lang welcomes you\n 	please enter your program\n			to end please enter -1\n");
	int adress=0;
	while(instruction_register!=-1){
		scanf("%Lf",&instruction_register);
		if((int)instruction_register==-1)break;
		if(instruction_register<-1){
			adress=instruction_register*-1;
			scanf("%Lf",&instruction_register);
		}
		else adress=counter++;
		memory[adress] = instruction_register;
	
	}
	int HULT=1;
	printf("program loaded sucsesffuly\n");
	while(HULT){
		operation_code=memory[instruction_counter];
		operand=(int)memory[instruction_counter++];
		int x=1;
		while(operation_code>100){
			x*=10;
			operation_code/=10;
		}
		operand%=x;
		switch(operation_code){
			case READ:
				printf("Enter value\n");
				scanf("%Lf",&memory[operand]);
                           break;
			case WRITE:
			  	printf("%Lf\n",memory[operand]);
			       break;
			case PRINT:
				printf("%c",(char) memory[(int)memory[operand]]);  // from pointer to adress and resolving pointer
			case LOAD:
				acc=memory[operand];
		 		break;
			case STORE:
				memory[operand] = acc;
				break;
			case ADD:
				acc+=memory[operand];
				break;
			case SUB:
				acc-=memory[operand];
				break;
			case DIV:
				acc/=memory[operand];
				break;
			case MUL:
				acc*=memory[operand];
				break;
			case MOD:
				acc=(int)acc%(int)memory[operand];
				break;
			case BRANCH:
				instruction_counter=operand;
                                break;
			case BRANCHNEG:
				if(acc<0)instruction_counter=operand;
				break;
			case BRANCHZERO:
				if(acc==0)instruction_counter=operand;
				break;
			case DEBUG:
				dump_memory(memory);	
			case HALT:
				HULT=0;
				break;
		}
	}
	//memory dump here
	printf("value in acc is %Lf\n",acc);
	printf("last instruction called is %d\n",instruction_counter);
	printf("last operation code is %d\n",operation_code);
	printf("last operand is %d\n",operand);
	int row =0;
	for(int i=0;i<10;i++)printf("\t%d",i);		
	printf("\n");					
	for(int i=0;i<MEM_SIZE;i++)
	{
		if(i%10==0)printf("\n%d\t",10*row++);
		if(memory[i]==0)printf("0000\t");
		else printf("%Lf\t",memory[i]);
	}
	printf("\n");			
}
void dump_memory(long double *arr){
	printf("\r");
	int row =0;
	for(int i=0;i<10;i++)printf("\t%d",i);		
	printf("\n");					
	for(int i=0;i<MEM_SIZE;i++)
	{
		if(i%10==0)printf("\n%d\t",10*row++);
		if(arr[i]==0)printf("0000\t");
		else printf("%Lf\t",arr[i]);
	}
	printf("\n");			

}
