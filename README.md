# systems-engineering-tasks

## Task 0
cloned my selfie repo to my machine.
added print my name to begin of selfie().
travis did not work straight away, but relogging to github did the job.

## Task 1
add -x option in selfie().
redirected to selfie_run(DOUBLEMIPSTER).
### selfie_run
create a similar second context and links them as a cyclic single linked list to easily traverse them (setNextContext(currentContext, previousContext) and the other way around).
selfie_run then starts the doublemipster(), which is a modified copy of mipster(). with either of the two contexts as input.
### doublemipster()
instead of stopping the context(s) every 10 000 000 instructions, we set the timeout to 1.
instead of using the same context all the time, every timeout, we go one step further in the single linked list of contexts (toContext = getNextContext(fromContext)).
in our case, with two contexts, it now switches between those two contexts, executing one instruction at a time.
the first time "handleSystemCalls(fromContext) == EXIT" is true, it should not yet return, but do another round for the second context to finish.
also, after the first context is finished, the switch does not take place anymore (if(firstEnded) toContext = fromContext;).

i was not sure how to test if the second program finishes, after the first one stopped, even if the second program is longer then the first. i decided to put a large number of instructions for the inital iteration, so the first program can finish sooner.
after the first iteration on the first context was finished, i set the timeout to 1 instruction. Now the second program should run concurrently to the first, but starting and finishing later. By enabling the debug_switch option, i could verify that behavior.

## Task 2
added global SYSCALL_LOCK and SYSCALL_UNLOCK.
### wrote emitLock() for setting up the mips code, analog for emitUnlock()
added procedurename and location to lib table, so it can be parsed.
made a syscall with id SYSCALL_LOCK.
need to call emitLock() function in selfie_compile() to make it part of the binary.
now selfie doesnt complain anymore about lock() being an unknown procedure, but an unknown syscall.
### wrote implementLock() for handling the lock() syscall
change the global variable writeLockContext to the caller context, if no other context is locked (if(writeLockContext == (uint64_t*) 0) { writeLockContext = context;} ).
if some context has already aquired the lock and it is not the caller, then the caller will now (at lock() in the sourcecode) be in a loop until the lock is released. (setPC(context, getPC(context) -INSTRUCTIONSIZE);).
### wrote implementUnlock() for handling the unlock() syscall
if the caller context is indeed the current owner of the lock (so it has the permission), we can safely remove it (writeLockContext = (uint64_t*) 0;).
### handleSystemCalls()
redirect the syscall SYSCALL_LOCK to implementLock(), now lock() is no longer an unknown syscall and redirects to implementLock(), analog for unlock().
redirect SYSCALL_WRITE to implementWrite().
	if no lock is set, as usual to implementWrite().
	if a lock is set (and some context want to write) we only let it write/redirect to implementWrite, if the caller is the owner of the lock.
	if the context isn't the owner, we let it loop. We do so by decreasing the pc of that context by one instruction, so the write syscall will come over and over again until at some point the lock isreleased and the looping context can take it. It will be the owner, execute the write and thereby break the loop.

i had troubles with finding a waiting/pausing strategy for my contexts and i feel like the looping method by decreasing the pc is suboptimal.

## Task 4
implemented doublehypster like doublemipster, but with bootlevelzero condition for the lowest execution level.
i had problems because of my previous implementation of doublemipster, because using the context's own nextContext field for linking the two contexts apparently was no good idea (two different uses for one data field).
so i created a simple null-terminated context(pointer) list and pass this to doublemipster/hypster.

## Task 5
implemented multihypster with my own contexts list in a round robin fashion.
### added fork system call. on implementFork.
create a deep copy of the currently executing context.
	set all int contexts values mit getter and setter.
	pointer, especially pagetable and register need their own space, Both of them need to be newly allocated and every entry (regstervalues, pages) need to be copied one by one to bea true deep copy.
add the copied context to my contexts list of executing contexts.
set the return register values accordingly, so on fork() return we can idetify if we are the copied or original context and do things depending on that information.

## Task 6
### generalize my executable contexts list functions to be able to operate on different lists (to use it also for the childlist later)
added two systemcalls, done as usual.
make prints function for my own context lists.
added process status to context, initialize with ready.
adjusted my multimipster/multihypster to reflect process states.
if a context gets an exit call, i dont remove the context anymore but set its state to "exited" (this is valid for child- and parentcontexts).
only switch to ready contexts (inside getNextExecutableContext()).
now returns when no context is ready or waiting.
also returns if a context is waiting and there are no running (ready/waiting) children for that context (getNextExecutableContext will return a nullpointer).
### add childlist to context struct to remember created childcontexts to be able to resolve the wait() syscall
### add child on fork to parents childlist
### implementWait: set contexts state to waiting
### add check for waiting state in getNextExecutableContext and handle it in handleWaiting()
At first i want to check if the basic suspension of execution for the parent works.
	if context has an exited child (zombie), i just set the context to ready again. Now, after the wait call of the parent, it will be suspended until a child finishes.
	The waiting state suspends a context, because in those cases, getNextExecutableContext() will recursevly search the next element to return.
	This might also be the previously waiting context itself, if a handleWaiting() has meanwhile set the waiting contexts status back to ready.
Now i can also retrieve the childs pid, set it in return register of the parent, free the child and verify all of that in the testprogram.
(check if the process can still continue. if there is at least one child, check for every waiting child the same, else return nullpointer (shut down emulator/hypervisor)).
### changed from childList zu parentpointer in child, so i dont have to delete the context in two places. getChildList() now operates on execContexts and calculates the childs anew each time
### implementKill takes a pid (needs to be loaded in emit and retrieved in implement function) and calls killContext()
the contex to kill gets searched and if found, i put an exitcall in its registers and return 0, else 1.


i decided not to make an extra zombie_state as it seemed redundant to state_exited to me.
i increase maxpid and set pid in allocateContext, but (i think) this also gets called by hypster which causes it to tick up even though it shouldnt.
The ids are still unique and the ticks are just the number of processes, so its not much and therefore i dont see a problem.
i prefer this solution so the pid will definately be incresed, independently of where i create the context (at creating them "manually" in selfie_run for doublemipster or at a fork).

### my questions-list that arose during programming
? a childcontext (as well as secondContext, thirdcontext) now stays in the contextsToExecute/usedContexts List, even after they exited, if the parent does not wait-kill-combo it) ok?
? "A `kill` syscall forces the termination of the child process given in input" .. termination means only delete from the contextsToExecute/usedContexts List ?
? would kill also be able to delete a parentcontext.
? now my own list implementation is useful? ì dont need arguments vctxt and parent like in findcontext?
? i dont need a reference from child to parent?
? zombie not an extra process state?

## Task 7
### add syscall thread()
### make copyContextThread()
like copycontext, but dont deep copy all frames, only the pagetableentries (except stack). then all pages (except stack) of both pagetables (process and thread) map to the same physical frame.
### using mapOrLinkPage() instead of simple mapPage on a pagefault of a context, which handles mapping of pages differently for threads and processes.
processes get new pages allocated, whereas threads get the physical frames of their parents, if it is something else than a stack access.
	if it is a thread and it is no stack access, it tries to retrieve the threadParents (main-processes) physical frame to map into the threads pagetable.
		if the parent has no frame mapped there, also the parent needs to map it first. we can recursively call the function on the parent.
	if it is a thread and the pagefault is a stack access, a new page/frame hast to be allocated for the (unique) stack of our thread.
	if it is not a thread, we simply mapPage like before.
### handleExitSyscall()
removes threads from executableList and delete from usedContexts (to free it), if the parent exits.
removes threads that exited from the executableList and delete from usedContexts.

setting TIMESLICE on different values (2 / 4) produces the two possible outputs 2 and 1 for my testfile.
I think this is, because in one case the parent exits, without the thread having executed the x = x +1 statement.
With a different number of executions per switch, the thread can get "another round" (executing x = x + 1, on the shared memory), before the parent finishes.
Hence the values differ.

## Task 8
### add syscall compare_and_swap() as usual
### make a basic testfile: populate it with new, old, and address. set addresses value to old and pass them to compare_and_swap()
### compare_and_swap()
take the three values from the registers.
we also want the value, where the address is pointing to.
as it is a contexts' virtual address and not in OS' adressspace, we need to acces it through the pagetable.
compare if the old value equals the addressvalue, if so this means no other process meanwhile changed the value.
we can be sure of that, because we know on what value the contexts calculation was based on (old).
so using the (shared) address we can easily check that the value still is what the context "thinks" it is.
then it is safe for us to change it.
we also return true to tell the context that its try to (atomically) change the value succeeded.
	it will then stop trying (most likely get out of some loop) and can go on with its computation.
	also it can be certain that the last operation did not cause a race condition.
	this is, because the OS changes the value atomically, by first verifying the original contexts old value and then changing it.
	it is atomical, because there is no switch out of the OS to a context between those two steps.
otherwise return false.
	we do not write.
	the context most likely will have to try again by going once more though a loop.
	this is, because we now know, that the value the contexts computation was based on, was changed.
	if we write now, we would overwrite some other contexts calculation, thus run into a race-condition.



## Bonus (hello-world.c)
in the last assignment we could force a race condition for shared memory by changing the value of timeslice.
because our calculation was incrementing the shared memorys value by 1, we were able to produce 1 and 2 as outputs.
if we replace the assignment (sharedMemory = sharedMemory + 1) of the example with compare_and_swap(), it should not be possible to force that race condition anymore.
(although it could still happen that the process finishes before the threads calculation, but with 2 short processes this is not a problem because a syscall also forces a switch?)
when i make tests with different timeslice values, like for the last assignment, but with compare_and_swap() i get no race conditions anymore.
this is no proper proof, but a very good sign.

### Tests
withoutCAS(): timeslice 2, exitcode 2 -> no race condition
withoutCAS(): timeslice 4, exitcode 1 -> race condition, calculations might have been overwritten
withoutCAS(): timeslice 8, exitcode 1 -> race condition, calculations might have been overwritten
withoutCAS(): timeslice 10, exitcode 1 -> race condition, calculations might have been overwritten
withoutCAS(): timeslice 103, exitcode 1 -> race condition, calculations might have been overwritten
withoutCAS(): timeslice 10000, exitcode 1 -> race condition, calculations might have been overwritten

withCAS(): timeslice 2, exitcode 2 -> no race condition
withCAS(): timeslice 4, exitcode 2 -> no race condition
withCAS(): timeslice 8, exitcode 2 -> no race condition
withCAS(): timeslice 10, exitcode 2 -> no race condition
withCAS(): timeslice 103, exitcode 2 -> no race condition
withCAS(): timeslice 10000, exitcode 2 -> no race condition


### Problems:
i had troubles until i remembered that multiple arguments (A0, A1, A2) are stored in the reverse order.
i had troubles until i realized, that the address i am getting from the context as argument is in a different address space then the OS.
Therefore i needed to access it through the pagetable -> load/storeVirtualMemory(pt,address,/data) instead of just *address.

## Task 9
### make malloc threadsafe
whenever a context mallocs, not only its own heap bump pointer should be incremented, as many contexts might share this memory.
as threads and its parent all have the same pid, we rotate thorugh all contexts and increment the "programBreak" variable for those who share a pid with the calling context.
### make Node struct
create, set and get of payload and pointer to next Node.
analogous to contexts linked list in selfie.
### create global treiberStack variable
our compare_and_swap method takes a memoryadress as input for the shared memory, in order to check *its content* against a given int (old).
if the treiberStack variable is designed in a way, that it is always the pointer to the most recent node on the stack (like usually linked lists in selfie), we have a problem.
	success = compare_and_swap((uint64_t) newHead, (uint64_t) oldHead, treiberStack).
	in this case, we have a problem because we would like to check if (treiberStack == oldHead).
	but compare_and_swap actually compares the content of treiberStack with oldHead (*treiberStack == oldHead) and this is not what we want.
	Because of this, we need to make "containervariable" treiberStack = malloc(8), that points to a shared memory word that holds the pointer to the first node.
### push()
create new Node with given value and the stacks topmost element as nextNode.
loop until the node could successfully be added to the stack. that is if the stack has not been modified by another thread during the Nodecreation.
then the top of the stack will be replaced atomically in the shared memory, "updating" it for every thread.
Because the check for modified shared memory and the modification of it happen atomically, no nodes will be lost, as no other thread can do anything in the meantime.
### pop()
analogous to push(), but instead of creating a node and chaining it to the existing stack, we take the topmosts node's nextNode and set is as the new stack.

### testing
filling the stack with different values, removing them and returning the lastPopReturnValue i could verify with different configurations that my implementation is semantically a proper stack.
i let my threads return getTreiberStackSize() that counts the nodes on the stack and made an alternative push method without compare_and_swap pushWithoutCAS().
looking at the treiberstacksize of the last finished thread.
	if i use the threadsafe version, the stacksize always reflects the amount pushs() that came from all threads (no nodes lost).
	if i use pushWithoutCAS(), the stacksize differs (nodes seemingly get lost).
