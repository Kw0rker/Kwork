#include <kwork_params.h>
#define READ 10 // inserts data from terminal to memory address
#define WRITE 11 // prints data frm memory adress
#define PRINT 12 // prints a char from memory adress
#define LOAD 20 //inserts data to acc register from memory
#define STORE 21 // stores data from acc to memory address
#define PLOAD 22 //inserts data from pointer to memory to acc
#define PSTORE 23 //stores data from acc in memory pointer
#define ADD 30 //sums acc with data in mem address and stores result in acc
#define SUB 31 //subtracts accc with data in mem adress result stored in acc
#define DIV 32 //divedes acc with data in mem adress result stored in acc
#define MUL 33 // multiplies acc with data in mem adress result stored in acc
#define MOD 34 //takes acc mod mem adress, result stored in acc
#define BRANCH 40 // go to mem adress 
#define BRANCHNEG 41 // go to mem adress if acc < 0 
#define BRANCHZERO 42 // go to mem adress if acc==0
#define HALT 43 //stop, executes after programs finished
#define CALL 44 // calls a function from memory adress and expect return value on stack
#define BIT_S_R 50 //bit shifts acc to the right with the value from pointer to memory memory[operand], result stored in acc
#define BIT_S_L 51 //bit shifts acc to the left with the value from pointer to memory memory[operand], result stored in acc
#define BIT_OR 52 // Binary OR acc with the value from pointer to memory memory[operand], result stored in acc
#define BIT_AND 53 // Binary AND acc with the value from pointer to memory memory[operand], result stored in acc
#define BIT_XOR 54 // Binary XOR acc with the value from pointer to memory memory[operand], result stored in acc
#define BIT_INV 55 //inverts bytes of acc result stored in acc
#define LOG_LESS 80 //compares if acc less than zero
#define LOG_LESSEQ 81 // compares if acc less or equal zero
#define LOG_INV 82 //inverts value of acc
#define SYSCALL 70 //calls kwork system call, by operand as syscall id
#define PUSH 71 //pushes data from memory on the stack
#define POP 72 //pops data from the stack to memory adress
#define DEBUG 60 //dumps memory when reached
/*
 * all commands saved followed by prevoius, strting at 0, if input is negative it counts as an adress for next command
 */

struct thread
{

	unsigned savedstate; // 0 <= savestate < 10000
						 // 0 <= savestate < 2^13
	unsigned id;
	int acc_s;
};
typedef struct thread THREAD;
typedef THREAD *THREADPTR;

void switch_threads(THREADPTR [],int *,int *,long *,long*,int *,struct timespec *);
void remove_thread(int *,THREADPTR [],int *,long *);
void select_new_thread(int *,long *,THREADPTR[],int *,int *);
void dump_memory(long*);

int main(){
	long acc ;
	int ESP = ESP_ADR; //esp register
	int EBP = MEM_SIZE-1; //ebp register points to highest adressable memory
	int instruction_counter;
	long  instruction_register;
	int operation_code;
	int operand;
	long memory[MEM_SIZE];
	// thread pool declaration
	THREADPTR thread_pool[MAX_THREAD_POOL];
	int active_threads[MAX_THREAD_POOL+1];
	active_threads[0]=1; //first element shows how many active threads
						 //there are
						 //other elements contain thread id for accesing thread in thread pool
						 // thread_pool[active_thread[1]] exmp	
						 // it's guaranteed by assembler that id does not exceed max thread pool size
	struct timespec *tp = malloc(sizeof (struct timespec)); //struct for clock_gettime() method
	long time_since_last_call=LONG_MAX; //time in nanosecs from last thread switch
	//decalre main thread
	THREADPTR main_thread = malloc(sizeof(THREAD));
	main_thread->savedstate=0;
	main_thread->id = 1;
	instruction_register = 0; 
	main_thread->acc_s=0;
	thread_pool[0]=main_thread;
	active_threads[1]=0; //reference to main thread in thread pool
	//
	int thread_id=0; //curent thread



	int waiting_threads[MAX_THREAD_POOL+1]; //map containg ids of threads that are on wait
	waiting_threads[0]=0; //first element shows amount of threads on wait
	/*
	 * Beggining of initiaalization 
	 */
	acc =0;
	instruction_counter=0;
	instruction_register=0;
	operation_code=0;
	operand = 0;
	memset(memory,0,sizeof(long) * MEM_SIZE);
	//for(int i=0;i<MEM_SIZE;i++)memory[i]=0;
	
	int counter=0;
	//printf("***Kworker lang welcomes you\n 	please enter your program\n			to end please enter -1\n");
	FILE *source;
	source = fopen(DEFNAME,"r");
	if(source==NULL){
		perror("No kwork code found\n");
		return -1;
	}
	int adress=0;
	while(!feof(source)){
		fscanf(source,"%ld\n",&instruction_register);
		if((int)instruction_register <= 0){
			adress=instruction_register*-1;
			counter = adress;
			fscanf(source,"%ld\n",&instruction_register);
			memory[adress]=instruction_register;

		}
		else{
			adress=counter++;
			memory[adress] = instruction_register;
		}
	
	}
	//dump_memory(memory);

	while(active_threads[0]>0){ //works till there's any running thread left
		switch_threads(thread_pool,active_threads,&instruction_counter,&acc,&time_since_last_call,&thread_id,tp);



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
				
				scanf("%ld",&memory[operand]);
                           break;
			case WRITE:
			  	printf("%d",(int)memory[operand]);
			       break;
			case PRINT:
				//fwrite(&memory[operand],sizeof(char),acc,stdout);
				putc(memory[operand],stdout);
				//printf("%c",(char) memory[operand]);  // from pointer to adress and resolving pointer
			case LOAD:
				acc=memory[operand];
		 		break;
			case STORE:
				memory[operand] = acc;
				break;
			case PLOAD:
				acc=memory[memory[operand]];
				break;
			case PSTORE:
				memory[memory[operand]] = acc;
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
				remove_thread(&thread_id,thread_pool,active_threads,&time_since_last_call);
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
					//printf("new thread created\n");
					active_threads[0]++; //increment total number of threads
					THREADPTR new_thread = malloc(sizeof(THREAD));
					new_thread->savedstate = acc; //instruction pointer is stored in acc
					new_thread->id = CONVERT_THREAD_POINTER_TO_ID(acc); // here we convert insturction pointer to thread id
					active_threads[active_threads[0]] = CONVERT_THREAD_POINTER_TO_ID(acc);
					thread_pool[CONVERT_THREAD_POINTER_TO_ID(acc)] = new_thread;
					//new thread scheduling done	
					break;
					//sets curent thread to wait by temproary removing its id from selection pool @active_threads
					//and releases lock to execute next thread
					case WAIT:
					waiting_threads[++waiting_threads[0]] = thread_id; //store current thread id in map
					for(int i = thread_id;i <= active_threads[0];i++){
					active_threads[i] = active_threads[i+1];
					}
					time_since_last_call = LONG_MAX;
					active_threads[0]--;
					break;

					//invokes thread with id from acc 
					//see @page libs/syscalls.h
					case INVOKE:
					active_threads[++active_threads[0]] = CONVERT_THREAD_POINTER_TO_ID(acc); 
					break;

					case INVOKE_ALL:
					for(int i = 1; i <= waiting_threads[0];i++){
						active_threads[++active_threads[0]] = waiting_threads[i]; //place all removed thread's id back to selection pool @active_threads
					}

					break;
					case GETESP:
					acc = ESP;
					break;
					case SETESP:
					ESP = acc;
					break;
					case GETEBP:
					acc = EBP;
					break;
					case SETEBP:
					EBP = acc;
					break;

				}
				break;

				//bitwise operation
				case BIT_OR:
				acc|=memory[operand];
				break;
				case BIT_AND:
				acc&=memory[operand];
				break;
				case BIT_XOR:
				acc^=memory[operand];
				break;
				case BIT_INV:
				acc= ~acc;
				break;
				case BIT_S_L:
				acc<<=memory[operand];
				break;
				case BIT_S_R:
				acc>>=memory[operand];
				break;
				//

				case PUSH:
				memory[ESP--] = memory[operand]; //as stack grows upward to the memory
				break;
				case POP:
				memory[operand] = memory[++ESP]; // pops value from stack
				break;
				case CALL:
				instruction_counter = memory[operand];
				break;
				case LOG_LESS:
				acc=acc<0;
				break;
				case LOG_LESSEQ:
				acc=acc<=0;
				break;
				case LOG_INV:
				acc=!acc;
				break;


		}
	}
	//memory dump here
	printf("\n-------------MEMORY DUMP---------------------\n");
	printf("value in acc is %d\n",(int)acc);
	printf("last instruction called is %d\n",instruction_counter);
	printf("last operation code is %d\n",operation_code);
	printf("last operand is %d\n",operand);
	printf("---------------------------------------------\n");
	//dump_memory(memory);
}
void dump_memory(long *arr){
	printf("\r");
	int row =0;
	for(int i=0;i<10;i++)printf("\t%d",i);		
	printf("\n");					
	for(int i=0;i<MEM_SIZE;i++)
	{
		if(i%10==0)printf("\n%d\t",10*row++);
		if(arr[i]==0)printf("0000\t");
		else printf("%ld\t",arr[i]);
	}
	printf("\n");			

}
//@thread_pool is pool for all threads
//@active_threads all active threads and their ids
//@ic instruction counter which saved and changed during threads switch
//@acc acc register which data needs to be preserved accross threads
//@time_since_last_call time since last thread switch was performed in nanosecs
//@thread_id is id of current thread that is been swiched from
//@tp is timespec struct that's used by clock_gettime()
void switch_threads(THREADPTR thread_pool[],int *active_threads,int *ic,long *acc,long*time_since_last_call,int *thread_id,struct timespec *tp){
	clock_gettime(CLOCK_MONOTONIC,tp);
	if( abs((tp->tv_nsec) - (*time_since_last_call)) > (MIN_THREAD_TIME + rand()%MAX_THREAD_TIME) ){
		(*time_since_last_call) = tp->tv_nsec; 								//reset timer
		if(*thread_id > 0){													//check if thread id is positive to prevent SIGSEGV
		thread_pool[active_threads[(*thread_id)]]->acc_s = (*acc); 			//save current value of acc to the coresponding thread
		thread_pool[active_threads[(*thread_id)]]->savedstate = (*ic);		// save last thread insturction pointer so the proccess can be resumed
	    }
		select_new_thread(active_threads,acc,thread_pool,thread_id,ic);
	}
}
void select_new_thread(int *active_threads,long *acc,THREADPTR thread_pool[],int *thread_id,int *ic){
		int next_thread_id = 1 + rand() % active_threads[0]; 				// active_threads[0] contains number of all active threads, here we select
															 				//next thread to be executed
		(*acc) = thread_pool[active_threads[next_thread_id]]->acc_s; 		//restore value of acc register of the thread
		(*thread_id) = next_thread_id; 										// set current thread to what we just selected
		(*ic) = thread_pool[active_threads[next_thread_id]]->savedstate; 	// restores the instuction pointer from thread saved value 
														 					//(restore thread insruction execution from the moment thread was swiched)
																			// now new thread is executing 
}
void remove_thread(int *thread_id,THREADPTR thread_pool[],int *active_threads,long *time_since_last_call){
	free(thread_pool[active_threads[(*thread_id)]]);
	for(int i = (*thread_id);i <= active_threads[0];i++){
		active_threads[i] = active_threads[i+1];
	}
	active_threads[0]--; 											// decrement totaal number of threads
	(*thread_id) = -1 ; 											//set id to negative number to prevent SIGSEGV
	(*time_since_last_call) = LONG_MAX;

}
