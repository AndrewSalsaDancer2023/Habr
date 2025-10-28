.file "jmp_functions.s"
.text
// int my_setjmp(my_jmp_buf env);
// Сохраняет состояние контекста в env и возвращает 0.
.globl my_setjmp
.type my_setjmp,@function
my_setjmp:

       movq %rsp, 0(%rdi)      /* Сохранить указатель стека rsp */
       movq %rbp, 8(%rdi)      /* Сохранить указатель фрейма rbp */
       movq %rbx, 16(%rdi)     /* Сохранить rbx (callee-saved)  */
       movq %r12, 24(%rdi)     /* Сохранить r12 (callee-saved) */
       movq %r13, 32(%rdi)     /* Сохранить r13 (callee-saved) */ 
       movq %r14, 40(%rdi)     /* Сохранить r14 (callee-saved) */
       movq %r15, 48(%rdi)     /* Сохранить r15 (callee-saved) */
       movq (%rsp), %rcx       /* Прочитать адрес возврата */
       movq %rcx, 56(%rdi)     /* Сохранить адрес возврата */
       xorl %eax, %eax         /* Установить eax = 0 (возврат при первом вызове) */
       ret

// void my_longjmp(my_jmp_buf env, int val);
// Восстанавливает состояние контекста из env и передает управление,
// возвращая val
.globl my_longjmp
.type my_longjmp,@function
my_longjmp:
       movq 0(%rdi), %rsp      /* Восстановить указатель стека rsp  */
       movq 8(%rdi), %rbp      /* Восстановить указатель фрейма rbp */ 
       movq 16(%rdi), %rbx     /* Восстановить rbx                  */
       movq 24(%rdi), %r12     /* Восстановить r12*/
       movq 32(%rdi), %r13     /* Восстановить r13*/
       movq 40(%rdi), %r14     /* Восстановить r14*/
       movq 48(%rdi), %r15     /* Восстановить r15*/
       movq %rsi, %rax         /* Установить rax = val*/
       testl %eax, %eax        /* Если val == 0,*/
       jnz to_exit             /* то перейти к следующей инструкции.*/
       incl %eax               /* Если val == 0, возвращаем 1*/
    to_exit:
        movq 56(%rdi), %rcx     /* Загрузить адрес возврата*/
        pushq %rcx              /* Поместить его в стек*/
        ret                     /* Вернуться по адресу из стека Переход к сохраненному адресу возврата*/

