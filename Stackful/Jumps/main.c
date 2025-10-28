#include <stdio.h>
#include "my_setjmp.h"
#include <stdio.h>
my_jmp_buf env;

void some_risk_function() {
    puts("Inside risky_function\n");
    my_longjmp(env, 5); // Back jump to the main with return value of 5
    puts("This string never will be printed\n");
}

int main() {
    int ret;
    
    puts("Calling my_setjmp first time\n");
    ret = my_setjmp(env);

    if (ret == 0) {
        puts("my_setjmp returns 0, this is first time\n");
        some_risk_function();
    } else {
        printf("my_setjmp returns %d from my_longjmp\n", ret);
    }
    
    puts("The end\n");
    return 0;
}

