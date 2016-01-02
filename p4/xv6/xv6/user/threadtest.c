#include "types.h"
#include "stat.h"
#include "user.h"

int stdout = 1;

void foo(void *arg)
{
	int i;

	for(i = 0; i < 10; i++)
	{
		printf(stdout, "%d\n", i);
	}

	//return NULL;
}

int
main(int argc, char *argv[])
{
	void *arg = 0;
	lock_t *lock;

	lock = malloc(sizeof(lock));
	lock->locked = 1;

	printf(stdout, "THREAD TEST BEGIN---------------------------------\n");
	thread_create(foo, arg);
	thread_join();
	lock_init(lock);
	lock_acquire(lock);
	lock_release(lock);

	printf(stdout, "THREAD TEST END-----------------------------------\n");

	exit();
}
