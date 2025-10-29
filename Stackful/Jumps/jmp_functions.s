.file "jmp_functions.s"
.text
// int my_setjmp(my_jmp_buf env)
// Saves the context state to env and returns 0.
.globl my_setjmp
.type my_setjmp,@function

my_setjmp:
       movq %rsp, 0x0(%rdi)      /* Save rsp */
       movq %rbp, 0x8(%rdi)      /* Save rbp */
       movq %rbx, 0x10(%rdi)     /* Save rbx */
       movq %r12, 0x18(%rdi)     /* Save r12 */
       movq %r13, 0x20(%rdi)     /* Save r13 */ 
       movq %r14, 0x28(%rdi)     /* Save r14 */
       movq %r15, 0x30(%rdi)     /* Save r15 */
       movq (%rsp), %rcx       /* Read the return address to rcx */
       movq %rcx, 0x38(%rdi)     /* Save the return address */
       xorl %eax, %eax         /* Store eax = 0 */
       ret

// void my_longjmp(my_jmp_buf env, int val)
// Restores the context state from env and transfers control, returning val
.globl my_longjmp
.type my_longjmp,@function

my_longjmp:
       movq 0x0(%rdi), %rsp      /* Restore rsp */
       movq 0x8(%rdi), %rbp      /* Restore rbp */ 
       movq 0x10(%rdi), %rbx     /* Restore rbx */
       movq 0x18(%rdi), %r12     /* Restore r12 */
       movq 0x20(%rdi), %r13     /* Restore r13 */
       movq 0x28(%rdi), %r14     /* Restore r14 */
       movq 0x30(%rdi), %r15     /* Restore r15 */
       movq %rsi, %rax         /* Set rax = val */
       testl %eax, %eax        /* Chaeck If val == 0  */
       jnz to_exit             /* val != 0 go to_exit */
       incl %eax               /* Increment val */
to_exit:
        movq 0x38(%rdi), %rcx     /* Load return address*/
        pushq %rcx              /* Push return address to stack */
        ret                     /* Jump to the saved return address */

