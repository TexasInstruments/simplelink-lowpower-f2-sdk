 *  Open Issues:
 *  @p(blist)
 *  - How do I go about making getHookContextPtr either inline or macro?
 *  - define/redefine Swi_isSwi().
 *  - revisit Swi_restorePri(). Seems vulnerable to interrupt.
 *  @p
 *
 *  Closed Issues:
 *  @p(blist)
 *  - Must implement switch to/from hwi stack.
 *    Swi/Hwi need APIs for switching to/from hwi stack.
 *    Where should this code reside?
 *    Local Swi/Hwi copies of this code?
 *    Where is Hwi stack defined and configured?
 *    Linux/C64 implementations.
 *  - Is Swi_post() required to save additional context as specified in BIOS 
 *    users manual 4.3.7? C6x doesn't do this.
 *  - review/clean up API comments.
 *  - think through interrupt enable/restore invocations. 
 *    Seems that ints are enabled by definition during Swi threads but
 *    Swi_post disables/restores and Swi_run disables/enables.
 *  - Swi_create() currently conforms to BIOS 5.20 arg list.
 *    UArgs should be (fxn, params, [arg0, arg1]) ?
 *  - I should use swi->readyQueue but I can't.
 *    readyQueue assignment causes lvalue error.
 *    If not used, remove readyQ ptr from instance object.
 *  - Make number of swi priorities configurable.
 *  - Use Queue module queues.
 *  - Eliminate Set and Get Params.
 *  - Get rid of nhook apis.
 *  - settle variable names. Camel case or not?
 *    Swi_curmask s/b Swi_curMask?, etc.
 *  - Global vars moved to module spec file.
 *  - rethink Swi_disable/restore logic. Should lock inc/dec?
 *    should locked be > 0, unlocked = 0? Yes.
 *  @p

