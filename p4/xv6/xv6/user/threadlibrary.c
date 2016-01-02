#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define PGSIZE 4096

// Thread library needed for P4 //DW-P4
int thread_create(void (*function)(void*), void *arg)
{
	void *stack;
	uint overaligned;

	// Allocate a new stack (ALIGNED) //
	// Need twice the memory space to ensure we have the aligned
	// space as well as PGSIZE worth of memory
	stack = malloc(PGSIZE+PGSIZE);
	overaligned = (uint)stack % PGSIZE;

	if(overaligned)
		stack = stack + (PGSIZE - overaligned);

	return clone(function, arg, stack);
}

int thread_join()
{
	int ret;
	
	void* user_stack;
	ret = join(&user_stack);
	free(user_stack);
	
	return ret;
}

void lock_init(lock_t *l)
{
	l->locked = 0; // initialize to not locked
}

void lock_acquire(lock_t *l)
{
	// just as spinlock.c does with acquire //
	while(xchg(&l->locked, 1) != 0)
		;
}

void lock_release(lock_t *l)
{
	// just as spinlock.c does with release //
	xchg(&l->locked, 0);
}

