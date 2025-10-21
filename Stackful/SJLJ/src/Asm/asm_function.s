#
# Coroutine context switch.
#
#   %rdi contains the current context structure.
#   %rsi contains the context to switch to.
#
.file "asm_functions.s"
.text
.globl contextswitch
.type contextswitch,@function
contextswitch:

        movq     %rsp, 0x00(%rdi)
        movq     %r15, 0x08(%rdi)
        movq     %r14, 0x10(%rdi)
        movq     %r13, 0x18(%rdi)
        movq     %r12, 0x20(%rdi)
        movq     %rbx, 0x28(%rdi)
        movq     %rbp, 0x30(%rdi)
        stmxcsr  0x38(%rdi)      /* save MMX control- and status-word */
    	fnstcw   0x3C(%rdi)      /* save x87 control-word */


        movq     0x00(%rsi), %rsp
        movq     0x08(%rsi), %r15
        movq     0x10(%rsi), %r14
        movq     0x18(%rsi), %r13
        movq     0x20(%rsi), %r12
        movq     0x28(%rsi), %rbx
        movq     0x30(%rsi), %rbp
        ldmxcsr  0x38(%rsi)      /* restore MMX control- and status-word */
        fldcw    0x3C(%rsi)      /* restore x87 control-word */

        ret
