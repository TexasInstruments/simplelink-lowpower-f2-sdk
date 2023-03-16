 *  Open Issues:
 *  @p(blist)
 *  - How do I go about making getHookContextPtr either inline or macro?
 *  - Make Task_exit() put task onto a terminated task list.
 *    Define an API to free all terminated task resources.
 *  - Make all TaskSupport_glue functions behave identically.
 *    first function invoked on task stack.
 *    calls Task_enter();
 *    calls Task_func(arg0, arg1);
 *    calls (or returns into) Task_exit();
 *  - Task_setPri needs new Q insert-at-start-of-Q func.
 *    Revisit setPri with Karl. Could use a redesign...
 *  - Too many embedded calls to Task_enter/restore
 *    Effects benchmarks.
 *    Only call Task_restore if key is 0? (tried this, no effect)
 *    Make Task_restore smaller so that it can be inlined (done).
 *    Consider letting the Swi_restore call in the dispatcher
 *    do the Task_restore().
 *  - Not happy with static task stack initialization.
 *    Don't know how to cast stack as a temp struct to hold fxn, args...
 *    Try union.
 *  - Handling of hookEnv storage is equivalent to stack space storage,
 *    which means that the stack can be explicitly specified, or if not
 *    specified, the heap from which it's allocated can be specified.
 *    This incurs some overhead (need to have hookEnv & hookEnvHeap
 *    storage in Task Handle obect & params object).  Should consider
 *    just using the same heap as was used for the Task Handle.
 *  @p
 *
 *  Closed Issues:
 *  @p(blist)
 *  - What to do if no tasks are ready?
 *    currently sits in spin loop in schedule().
 *    Call Error_Raise();
 *  - should last vitalTaskFlag cause Error_raise() or System_exit()?
 *    Quietly invoke System_exit();
 *  - Deal with glueAddr and HWI_D_ccmask in Task_asm.s62.
 *  - How to enable/disable Task Module?
 *    BIOS.taskEnabled.
 *  - Move target specific default stack size into TaskSupport. (like stackAlign)
 *  - Implement system shutdown logic for vitalTaskFlag.
 *  - Clean up Idle Task.
 *    Probably s/b in Idle module?
 *    Statically create Idle task in Idle.xs. (optionally)
 *    Make task func be Idle_loop(). Task.xs uses Idle.
 *    How to deal with first Task_switch()?
 *  - Task_restore needs to recheck work flag before returning???
 *    Nope, work flag is cleared while ints still disabled.
 *  - Normalize static and dynamic Task_create.
 *  - Task_create() args should be (fxn, params, [arg0,] ...).
 *    Can't coerce xdc into putting params anywhere but last in arg list.
 *    Dynamic Task_create() has fxn in params. If we want it to be separate
 *    arg (ie Task_create(fxn, params)), then static Task_create must
 *    have it also.
 *  - Task_funcPtr should be var args?
 *  - Eliminate dual-interrupt-context-on-Task-stack issue.
 *    Scheduler is executed with ints disabled. Caller of
 *    Task_restore is required to reenable interrupts.
 *    Must first implement Hwi stack...
 *  - Should all blocked tasks be put into inactiveQ? Why?
 *    No.
 *  - Consider internal function Task_unblock() that requires Task_disabled.
 *  - 40 cycles of the Hwi epilog time is due to Task_restore() taking forever to
 *    figure out it has nothing to do. Swi_restore takes less than 16 cycles.
 *    If Task_restore could be reduced to 16 cycles, epilog time would be 88 cycles.
 *    Adding work flag and checking it in Task_restore reduced Hwi_epilog time by
 *    31 cycles, disptacher -24 Sem_pend/post +12/29,
 *  - I don't see the need for Task_enter() which took the place of KNL_exit()
 *    in the initial stack frame.
 *  - Finish implementing Task_stat().
 *    Can't have Params in Stat for some reason.
 *  - Status pointer undefined.
 *  - Add func headers to all funcs.
 *  - Task_delete needs instance__finalize to free stack if alloc'd.
 *  - Haven't achieved Swi/Task scheduling uniformity yet.
 *    Must either morph Swi to be like Task or vice versa.
 *    There is no curMask in Task and there is no maxQ in Swi.
 *    If I use maxQ in Swi then Swis could have 32 priorities.
 *  - When is mode updated. Who uses it? Can it be eliminated?
 *  - Dynamic stack allocation.
 *  - Make fxn be a Task_fxn type.
 *  - Make number of task priorities configurable.
 *  @p

