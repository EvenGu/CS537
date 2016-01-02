#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// DW-P2 //////////////////////////////////////////////////////////////////////
// Helper functions //
int queue_process(int priority, struct proc *p, int add_to_front);
int dequeue_process(int priority, struct proc *p);
void next_process(int p);

// Add queues for each priority with back/front pointer
struct proc* pQ[4][NPROC];
int pQ_size[4] 	= {0,0,0,0};
int pQ_front[4] = {0,0,0,0};
int pQ_back[4] 	= {0,0,0,0};
int pQ_curr_process[4] = {0,0,0,0};

// used to add processes to queue of given priority //
// add_to_front: 0 - add to back 1 - add to front
int queue_process(int priority, struct proc *p, int add_to_front)
{
	//cprintf("Adding %s [pid %d] to p%d ...",p->name,p->pid,priority);
	// check empty
	p->queued = 1;
	if(pQ_size[priority] == 0)
	{
		pQ_front[priority] = pQ_back[priority] = 31;
		pQ_curr_process[priority] = pQ_front[priority];
		pQ_size[priority]++;
		memmove(&pQ[priority][31],&p,sizeof(struct proc*));

		return 0;
	}
	// queue up new process //
	if(add_to_front == 1)
	{
		// add to the front //
		int prevP = pQ_back[priority];
		int newP = prevP-1;
		if(newP < 0)
			newP = (NPROC-1);
		// update back //
		pQ_back[priority]--;
		if(pQ_back[priority] < 0)
			pQ_back[priority] = (NPROC-1);

		while(prevP != pQ_curr_process[priority])
		{
			// make space in front of the queue //
			memmove(&pQ[priority][newP],&pQ[priority][prevP],sizeof(struct proc*));

			prevP++;
			if(prevP >= 64)
				prevP = 0;

			newP++;
			if(newP >= 64)
				newP = 0;
		}
		// add new process in the front //
		memmove(&pQ[priority][newP],&p,sizeof(struct proc*));
		pQ_size[priority]++;
	}
	else
	{
		// add to back //
		int prevP = pQ_front[priority];
		int newP = prevP+1;
		if(newP >= 64)
			newP = 0;
		// update front //
		pQ_front[priority]++;
		if(pQ_front[priority] >= 64)
			pQ_front[priority] = 0;

		while(prevP != pQ_curr_process[priority])
		{
			// make space in queue - yes we could do a list but it's dynamic...
			memmove(&pQ[priority][newP],&pQ[priority][prevP],sizeof(struct proc*));

			newP--;
			if(newP < 0)
				newP = (NPROC-1);

			prevP--;
			if(prevP < 0)
				prevP = (NPROC-1);
		}
		// add new process in the back //
		memmove(&pQ[priority][newP],&p,sizeof(struct proc*));
		pQ_size[priority]++;
	}

	return 0;
}

// used to remove processes from the queue that are no longer needed //
int dequeue_process(int priority, struct proc *p)
{
	// Remove current front process //
	//cprintf("Removing %s [pid %d] from p%d size: %d q:%d ...",p->name,p->pid,priority,pQ_size[priority],p->queued);
	
	// in case we concurrently got the same process removed //
	if(p->queued == 0)
		return 0;
	
	// set queued status //
	p->queued = 0;
	
	if(pQ_size[priority] == 0)
	{
		cprintf("\n***ERROR*** REMOVED FROM EMPTY QUEUE\n");
		return -1; //error...
	}
	
	int temp = pQ_curr_process[priority];
	int next = temp+1;
	if(next >= 64)
		next = 0;

	while(temp != pQ_front[priority])
	{
		// move the queue to keep array continguous //
		pQ[priority][temp] = pQ[priority][next];
		memmove(&pQ[priority][temp],&pQ[priority][next],sizeof(struct proc*));
		temp++;
		if(temp >= 64)
			temp = 0;

		next++;
		if(temp >= 64)
			temp = 0;
	}
	// update the front/current process ptr //
	if(pQ_front[priority] != pQ_back[priority])
	{
		// edge case where curr == front prior to remove //
		if(pQ_curr_process[priority] == pQ_front[priority]){
			pQ_curr_process[priority]--;
			if(pQ_curr_process[priority] < 0)
				pQ_curr_process[priority] = (NPROC-1);
		}

		pQ_front[priority]--;
		if(pQ_front[priority] < 0)
			pQ_front[priority] = (NPROC-1);
	}
	// update size //
	pQ_size[priority]--;

	return 0;
}
// move pointer across the priority queues to next process
// return the current process for execution
void next_process(int p){
	pQ_curr_process[p]--;
	// jump to front if we were at the back node //
	if(pQ_curr_process[p] == (pQ_back[p]-1))
		pQ_curr_process[p] = pQ_front[p];
}
///////////////////////////////////////////////////////////////////////////////

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;
  int pti = 0; //DW-P2 - index in ptable for process

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++,pti++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  // DW-P2 ////////////////////////////////////////////////////////////////////
  // Initialize process with highest priority and 0 ticks
  p->priority = 0;
  int i;
  for(i = 0; i <= 3; i++)
  	p->ticks[i] = 0;
  p->pti = pti;
  p->queued = 0;
  /////////////////////////////////////////////////////////////////////////////
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  // DW-P2 - potential spot for queueing up new processes (this was changed later)
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  //DW-P2 - another good spot to add initial processes, but this may be done in scheduler()
  //queue_process(0, p, 0);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// DW-P2 //////////////////////////////////////////////////////////////////////
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		// add any new processes to p0 queue //
		if(p->state == RUNNABLE && p->ticks[0] == 0 && p->queued == 0){
			queue_process(0,p,0);
		}
		// any returning processes to the front
		else if(p->state == RUNNABLE && p->queued == 0 && p->ticks[p->priority] > 0){
			queue_process(p->priority,p,1);
		}
		// any new processes at this priority //
		else if(p->state == RUNNABLE && p->queued == 0)
			queue_process(p->priority,p,0);
	}
	
	int s;
	// priority 0 //
	for(s = 0; s < pQ_size[0]; s++)
	{
		if( pQ[0][pQ_curr_process[0]]->state != RUNNABLE)
		{
			dequeue_process(0,pQ[0][pQ_curr_process[0]]);
		}
		else{
			p = pQ[0][pQ_curr_process[0]];
			goto runnable_process;
		}
	}
	// priority 1 //
	for(s = 0; s < pQ_size[1]; s++)
	{
		if( pQ[1][pQ_curr_process[1]]->state != RUNNABLE)
		{
			dequeue_process(1,pQ[1][pQ_curr_process[1]]);
		}
		else{
			p = pQ[1][pQ_curr_process[1]];
			goto runnable_process;
		}
	}
	// priority 2 //
	for(s = 0; s < pQ_size[2]; s++)
	{
		if( pQ[2][pQ_curr_process[2]]->state != RUNNABLE)
		{
			dequeue_process(2,pQ[2][pQ_curr_process[2]]);
		}
		else{
			p = pQ[2][pQ_curr_process[2]];
			goto runnable_process;
		}
	}
	// priority 3 //
	for(s = 0; s < pQ_size[3]; s++)
	{
		if( pQ[3][pQ_curr_process[3]]->state != RUNNABLE)
		{
			dequeue_process(3,pQ[3][pQ_curr_process[3]]);
		}
		else{
			p = pQ[3][pQ_curr_process[3]];
			goto runnable_process;
		}
	}
	goto no_runnable_process; // no processes to run at the moment

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
runnable_process:
	  proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
	  // Check zombie state //
	  if(p->state == ZOMBIE){
	    // wake up parents? //
	  	dequeue_process(p->priority,p);
		goto no_runnable_process;
	  }
	  
	  // Increment current processes ticks @priority when returning after 1 tick //
	  p->ticks[p->priority]++;
	  //procdump();

	  // Handle queue priority changes based on ticks //
	  // Also remove processes which gave up the CPU and are !RUNNABLE //
	  switch(p->priority){
		case(0):
			if(p->ticks[0] >= 1)
			{
				p->priority++;
				dequeue_process(0, p);
				queue_process(1,p,0);
			}
			break;
		case(1):
			if(p->ticks[1] >= 2)
			{
				p->priority++;
				dequeue_process(1, p);
				queue_process(2,p,0);
			}
			break;
		case(2):
			if(p->ticks[2] >= 4)
			{
				p->priority++;
				dequeue_process(2, p);
				queue_process(3,p,0);
			}
			break;
		case(3):
			// check each 8 ticks //
			if((p->ticks[3] % 8) == 0 && pQ_size[3] > 1)
			{
				//cprintf("\n ... RR switch\n");
				next_process(3);
			}
			break;
		default:
			cprintf("***ERROR*** bad priority\n");
			break;
  		}
 
      proc = 0;
no_runnable_process:
	release(&ptable.lock);

  } // forever loop end
}
///////////////////////////////////////////////////////////////////////////////

// DW-P2 //////////////////////////////////////////////////////////////////////
// Return some basic information about each process.
//
// pid - Process ID
// runNum = number of times process was chosen to run
// currQ = current queue of the process
int
getpinfo(struct pstat *ps)
{
	struct proc *p;

	int index = 0;

	acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++, index++){
		// iterate processes for information //
		ps->pid[index] = p->pid;
		ps->priority[index] = p->priority;
		int j;
		for(j = 0; j < 4; j++)
			ps->ticks[index][j] = p->ticks[j];
		if(p->state == UNUSED)
			ps->inuse[index] = 0;
		else
			ps->inuse[index] = 1;
	}

	release(&ptable.lock);

	return 0;
}
///////////////////////////////////////////////////////////////////////////////

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
	}
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  //cprintf("----------------------------------------------------------------------\n");
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    //cprintf("%d %s %s \t\t0:%d 1:%d 2:%d 3:%d Queued:%d @priority:%d", p->pid, state, p->name,p->ticks[0],p->ticks[1],p->ticks[2],p->ticks[3],p->queued,p->priority);
	cprintf("%d %s %s",p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
	//if(pQ_size[0] > 0)
	//cprintf("Queue Info:\np0 size: %d front: %d back: %d curr: %d:[pid %d]\n",pQ_size[0],pQ_front[0],pQ_back[0],pQ_curr_process[0],pQ[0][pQ_curr_process[0]]->pid);
	//if(pQ_size[1] > 0)
	//cprintf("p1 size: %d front: %d back: %d curr: %d:[pid %d]\n",pQ_size[1],pQ_front[1],pQ_back[1],pQ_curr_process[1],pQ[1][pQ_curr_process[1]]->pid);
	//if(pQ_size[2] > 0)	
	//cprintf("p2 size: %d front: %d back: %d curr: %d:[pid %d]\n",pQ_size[2],pQ_front[2],pQ_back[2],pQ_curr_process[2],pQ[2][pQ_curr_process[2]]->pid);
	//if(pQ_size[3] > 0)
	//cprintf("p3 size: %d front: %d back: %d curr: %d:[pid %d]\n",pQ_size[3],pQ_front[3],pQ_back[3],pQ_curr_process[3],pQ[3][pQ_curr_process[3]]->pid);
	//cprintf("----------------------------------------------------------------------\n");
}

