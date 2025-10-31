.file "jmp_functions.s"
.text
// int my_setjmp(my_jmp_buf env)
// Saves the context state to env and returns 0.
.globl my_setjmp
.type my_setjmp,@function

my_setjmp:
	movq %rbx,(%rdi)     /* Save rbx */
	movq %rbp,0x08(%rdi) /* Save rbp */
	movq %r12,0x10(%rdi) /* Save r12 */
	movq %r13,0x18(%rdi) /* Save r13 */
	movq %r14,0x20(%rdi) /* Save r14 */
	movq %r15,0x28(%rdi) /* Save r15 */
	leaq 8(%rsp),%rdx    
	movq %rdx,0x30(%rdi) 
	movq (%rsp),%rdx     /* save return addr to buffer */
	movq %rdx,0x38(%rdi)
	xor %eax,%eax        /* Store eax = 0 */
	ret

// void my_longjmp(my_jmp_buf env, int val)
// Restores the context state from env and transfers control, returning val
.globl my_longjmp
.type my_longjmp,@function

my_longjmp:
	movq (%rdi),%rbx         /* Restore rbx */
	movq 0x08(%rdi),%rbp	 /* Restore rbp */ 
	movq 0x10(%rdi),%r12	 /* Restore r12 */
	movq 0x18(%rdi),%r13	 /* Restore r13 */
	movq 0x20(%rdi),%r14	 /* Restore r14 */
	movq 0x28(%rdi),%r15	 /* Restore r15 */
	movq 0x30(%rdi),%rsp
	
	movq %rsi, %rax         /* Set rax = val */
	testl %eax, %eax        /* Chaeck If val == 0  */
	jnz to_exit             /* val != 0 go to_exit */
    incl %eax               /* Increment rax */
to_exit:    
	jmpq *0x38(%rdi)           /* goto restored address */
