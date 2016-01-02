#include "types.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, stack_sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  //cprintf("\tPROCESS ID: %d\n", proc->pid);

  if((ip = namei(path)) == 0)
    return -1;
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = PGSIZE; //DW-P3 - just skip the first page for allocation
  
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
	// DW-P3
	//cprintf("\tallocuvm start: %x end: %x size: %x", sz, ph.va+ph.memsz,(ph.va+ph.memsz)-sz);

    if((sz = allocuvm(pgdir, sz, ph.va + ph.memsz)) == 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.va, ip, ph.offset, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  ip = 0;

  sz = PGROUNDUP(sz); // page align the sz
  //cprintf(" aligned at:%x\n", sz);
  
  // Allocate a one-page stack at the bottom of user memory space //DW-P3
  // 0x9FFFF is where sp should start and grow downward? or 0xA0000?
  stack_sz = USERTOP - PGSIZE;
  //cprintf("\tThe stack@: %x, size: %x\n",stack_sz, PGSIZE);
  if((stack_sz = allocuvm(pgdir, stack_sz, USERTOP)) == 0)
    goto bad;
  stack_sz = USERTOP - PGSIZE;
  //cprintf("\tStack pointer set to: %x stack_sz: %x\n",USERTOP,stack_sz);


  // Push argument strings, prepare rest of stack in ustack.
  sp = USERTOP; //DW-P3
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
	// check if sp will exceed the stack size //
	if(stack_sz > (sp - strlen(argv[argc])))
	{
		// grow the stack //
		if((stack_sz = allocuvm(pgdir, stack_sz-PGSIZE, stack_sz)) == 0)
    		goto bad;
		stack_sz -= PGSIZE;
		// what if we overflow the heap? //
	}
    sp -= strlen(argv[argc]) + 1;
    sp &= ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }

  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  //DW-P3 - this copies ustack to sp - ensure we have enough room //
  while((USERTOP-stack_sz) < (3+argc+1))
  {
	// grow the stack //
	if((stack_sz = allocuvm(pgdir, stack_sz-PGSIZE, stack_sz)) == 0)
    	goto bad;
	
	stack_sz -= PGSIZE;
	// what if we overflow heap??? //
  }
  // Now we can copy the ustack without issues //
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  // Commit to the user image.
  oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz;
  proc->stack_sz = stack_sz; //DW-P3
  proc->tf->eip = elf.entry;  // main
  proc->tf->esp = sp;
  switchuvm(proc);
  freevm(oldpgdir);

  //cprintf("Done with process %d\n", proc->pid);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip)
    iunlockput(ip);
  return -1;
}
