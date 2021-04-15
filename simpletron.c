#include <stdio.h>
#define READ 10 // inserts data from terminal to memory address
#define WRITE 11 // prints data frm memory adress
#define LOAD 20 //inserts data to acc register
#define STORE 21 // stores data from acc to memory address
#define ADD 30 //sums acc with data in mem address and stores result in acc
#define SUB 31 //subtracts accc with data in mem adress result stored in acc
#define DIV 32 //divedes acc with data in mem adress result stored in acc
#define MUL 33 // multiplies acc with data in mem adress result stored in acc
#define BRANCH 40 // go to mem adress 
#define BRANCHNEG 41 // go to mem adress if acc < 0 
#define BRANCHZERO 42 // go to mem adress if acc==0
#define HALT 43 //stop, executes after programs finished 
int main(){
	int acc ;
	int instruction_counter;
	int instruction_register;
	int operation_code;
	int operand;
	int memory[100];
	/*
	 * Beggining of initiaalization 
	 */
	acc =0;
	instruction_counter=0;
	instruction_register=0;
	operation_code=0;
	operand = 0;
	for(int i=0;i<100;i++)memory[i]=0;
	

	printf("***Kworker lang welcomes you\n 	please enter your program\n			to end please enter -1\n");
	while(instruction_register!=-1){
		scanf("%d",&instruction_register);
		int adress = instruction_register;
		if(adress<0){
			break;
		}
		if(adress<=9999){
			adress=0;
		}
		else {
			adress/=10000;
		}
		instruction_register%=10000;
		memory[adress] = instruction_register;
	
	}
	int HULT=1;
	printf("program loaded sucsesffuly\n");
	while(HULT){
		operation_code=memory[instruction_counter]/100;
		operand=memory[instruction_counter++]%100;
		int row=0;
		switch(operation_code){
			case READ:
				printf("Enter value\n");
				scanf("%d",&memory[operand]);
                           break;
			case WRITE:
			  	printf("%d\n",memory[operand]);
			       break;
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
			case BRANCH:
				instruction_counter=operand;
                                break;
			case BRANCHNEG:
				if(acc<0)instruction_counter=operand;
				break;
			case BRANCHZERO:
				if(acc==0)instruction_counter=operand;
				break;
			case HALT:
				HULT=0;
				break;
		}
	}
	//memory dump here
	printf("value in acc is %d\n",acc);
	printf("last instruction called is %d\n",instruction_counter);
	printf("last operation code is %d\n",operation_code);
	printf("last operand is %d\n",operand);
	int row =0;
	for(int i=0;i<10;i++)printf("\t%d",i);		
	printf("\n");					
	for(int i=0;i<100;i++)
	{
		if(i%10==0)printf("\n%d\t",10*row++);
		if(memory[i]==0)printf("0000\t");
		else printf("%d\t",memory[i]);
	}
	printf("\n");			
}


