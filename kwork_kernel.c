#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <syscalls.h>
#define MEM_SIZE 10000
#ifndef MAX_THREAD_POOL
#define MAX_THREAD_POOL 1000
#endif
#ifndef MIN_THREAD_TIME
#define MIN_THREAD_TIME 150 //in nanosecs
#endif
#ifndef MAX_THREAD_TIME
#define MAX_THREAD_TIME 450 //in nanosecs
#endif
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
#define SYSCALL 70 //calls kwork system call, by operand as syscall id
/*
 * all commands saved followed by prevoius, strting at 0, if input is negative it counts as an adress for next command
 */


struct thread
{
	int savedstate;
	int id;
	int acc_s;
};
typedef struct thread THREAD;
typedef THREAD *THREADPTR;


int main(){
	long acc ;
	int instruction_counter;
	long  instruction_register;
	int operation_code;
	int operand;
	long memory[MEM_SIZE];
	// thread pool declaration
	THREADPTR thread_pool[MAX_THREAD_POOL];
	int active_threads[MAX_THREAD_POOL+1];
	active_threads[0]=0; //first element shows how many active threads
						 //there are
						 //other elements contain thread id for accesing thread in thread pool
						 // thread_pool[active_thread[1]] exmp	
						 // it's guaranteed by assembler that id does not exceed max thread pool size
	struct timespec *tp = malloc(sizeof (struct timespec)); //struct for clock_gettime() method
	long time_since_last_call=LONG_MAX; //time in nanosecs from last thread switch
	int thread_id=1; //curent thread
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
		scanf("%L",&instruction_register);
		if((int)instruction_register==-1)break;
		if(instruction_register<-1){
			adress=instruction_register*-1;
			scanf("%L",&instruction_register);
		}
		else adress=counter++;
		memory[adress] = instruction_register;
	
	}
	int HULT=1;
	printf("program loaded sucsesffuly\n");
	while(active_threads[0]){ //works till there's any running thread left
		switch_threads(active_threads,thread_pool,&instruction_counter,&time_since_last_call,&thread_id,tp);



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
				//printf("Enter value\n");
				scanf("%L",&memory[operand]);
                           break;
			case WRITE:
			  	printf("%L\n",memory[operand]);
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
				break;	
			case HALT:
				HULT=0;
				remove_thread(thread_id,thread_pool,active_threads,&time_since_last_call);
				break;
			case SYSCALL:
				switch (operand){
					case GETIC:
					acc=instruction_counter;
					break;

					case SETIC:
					instruction_counter=acc;
					break;

					case NEWTHREAD:
					active_threads[0]++; //increment total number of threads
					THREADPTR new_thread = malloc(sizeof(THREAD));
					new_thread->savedstate = acc; //instruction pointer is stored in acc
					new_thread->id = acc % MAX_THREAD_POOL; // here we convert insturction pointer to thread id
					active_threads[active_threads[0]] = acc % MAX_THREAD_POOL;
					thread_pool[acc % MAX_THREAD_POOL] = new_thread;
					//new thread scheduling done	
					break;
				}
				break;
		}
	}
	//memory dump here
	printf("value in acc is %L\n",acc);
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
		else printf("%L\t",memory[i]);
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
		else printf("%L\t",arr[i]);
	}
	printf("\n");			

}
//@thread_pool is pool for all threads
//@active_threads all active threads and their ids
//@ic instruction counter which saved and changed during threads switch
//@acc acc register which data needs to be preserved accross threads
//@time_since_last_call time since last thread switch was performed in nanosecs
//@thread_id is id of current thread
//@tp is timespec struct that's used by clock_gettime()
void switch_threads(THREADPTR thread_pool[],int *active_threads,int *ic,int *acc,int*time_since_last_call,int *thread_id,struct timespec *tp){
	clock_gettime(CLOCK_MONOTONIC,tp);
	if( ((tp->tv_nsec) - (*time_since_last_call)) > (MIN_THREAD_TIME + rand()%MAX_THREAD_TIME) ){
		(*time_since_last_call) = tp->tv_nsec; //reset timer
		thread_pool[(*thread_id)]->acc_s = (*acc); //save current value of acc to the coresponding thread
		thread_pool[(*thread_id)]->savedstate = (*ic); // save last thread insturction pointer so the proccess can be resumed

		int next_thread_id = 1 + rand() % active_threads[0]; // active_threads[0] contains number of all active threads, here we select
															 //next thread to be executed
		(*acc) = thread_pool[active_threads[next_thread_id]]->acc_s; //restore value of acc register of the thread
		(*thread_id) = next_thread_id; // set current thread to what we just selected
		(*ic) = thread_pool[active_threads[next_thread_id]]->savedstate; // restores the instuction pointer from thread saved value 
														 //(restore thread insruction execution from the moment thread was swiched)
		// now new thread is executing 

	}
}
void remove_thread(int thread_id,THREADPTR thread_pool[],int *active_threads,int *time_since_last_call){
	free(thread_pool[active_threads[thread_id]]);
	for(int i=1;i<active_threads[0];i++){
		if(active_threads[i]==thread_id)active_threads[i]=0; // find thread id in activethread map and remove it
	}
	active_threads[0]--; // decrement totaal number of threads
	(*time_since_last_call)=LONG_MAX; //set max value so the next thread is executed for sure
}
