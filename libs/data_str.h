#include <stdlib.h>
struct stack
{
	struct stack *next;
	int data;
};
typedef struct stack STACK;
typedef STACK *STACKPTR;
STACKPTR new_stack();
void push(int,STACKPTR*);
int pop(STACKPTR*);
int peek(STACKPTR*);
int isEmpty(STACKPTR);
STACKPTR new_stack(){
	STACKPTR pt = malloc(sizeof (STACK));
	pt->data=0;
	pt->next=NULL;
	return pt;
}
void push(int data,STACKPTR *pt){
	STACKPTR newStack = malloc(sizeof (STACK));
	newStack->data=data;
	newStack->next=(*pt);
	(*pt)=newStack;
}
int pop(STACKPTR *pt){
	if (isEmpty(*pt))
	{
		perror("Poping empty stack!!\n");
		return 0;
	}
	STACKPTR temp = (*pt);
	(*pt)=(*pt)->next;
	int poped = temp->data;
	free(temp);
	return poped;
}
int peek(STACKPTR *pt){
	return (*pt)->data;
}
int isEmpty(STACKPTR pt){
	return pt->next==NULL;
}
