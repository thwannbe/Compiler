##################################################
# test08
#

    #-----------------------------------------
    # text section
    #
    .text
    .align 4

    # entry point and pre-defined functions
    .global main
    .extern Input
    .extern Output

    # scope fib
fib:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $20, %esp

    movl    8(%ebp), %eax               #  0:     if     n <= 1 goto 1_if_true
    cmpl    $1, %eax
    jle     l_fib_1_if_true
    jmp     l_fib_2_if_false            #  1:     goto   2_if_false
l_fib_1_if_true:
    movl    8(%ebp), %eax
    jmp     l_fib_exit                  #  3:     return n
    jmp     l_fib_0                     #  4:     goto   0
l_fib_2_if_false:
    movl    8(%ebp), %eax               #  6:     sub    t0 <- n, 1
    subl    $1, %eax
    movl    %eax, -4(%ebp)
    pushl   -4(%ebp)                    #  7:     param  0 <- t0
    call    fib                         #  8:     call   t1 <- fib
    addl    $4, %esp
    movl    %eax, -8(%ebp)
    movl    8(%ebp), %eax               #  9:     sub    t2 <- n, 2
    subl    $2, %eax
    movl    %eax, -12(%ebp)
    pushl   -12(%ebp)                   # 10:     param  0 <- t2
    call    fib                         # 11:     call   t3 <- fib
    addl    $4, %esp
    movl    %eax, -16(%ebp)
    movl    -8(%ebp), %eax              # 12:     add    t4 <- t1, t3
    addl    -16(%ebp), %eax
    movl    %eax, -20(%ebp)
    movl    -20(%ebp), %eax
    jmp     l_fib_exit                  # 13:     return t4
l_fib_0:
l_fib_exit:
    addl    $20, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope fact
fact:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $12, %esp

    movl    8(%ebp), %eax               #  0:     if     n <= 0 goto 1_if_true
    cmpl    $0, %eax
    jle     l_fact_1_if_true
    jmp     l_fact_2_if_false           #  1:     goto   2_if_false
l_fact_1_if_true:
    movl    $0, %eax
    jmp     l_fact_exit                 #  3:     return 0
    jmp     l_fact_0                    #  4:     goto   0
l_fact_2_if_false:
    movl    8(%ebp), %eax               #  6:     if     n <= 1 goto 6_if_true
    cmpl    $1, %eax
    jle     l_fact_6_if_true
    jmp     l_fact_7_if_false           #  7:     goto   7_if_false
l_fact_6_if_true:
    movl    8(%ebp), %eax
    jmp     l_fact_exit                 #  9:     return n
    jmp     l_fact_5_else_next          # 10:     goto   5_else_next
l_fact_7_if_false:
    movl    8(%ebp), %eax               # 12:     sub    t0 <- n, 1
    subl    $1, %eax
    movl    %eax, -4(%ebp)
    pushl   -4(%ebp)                    # 13:     param  0 <- t0
    call    fact                        # 14:     call   t1 <- fact
    addl    $4, %esp
    movl    %eax, -8(%ebp)
    movl    8(%ebp), %eax               # 15:     mul    t2 <- n, t1
    movl    -8(%ebp), %ebx
    imull   %ebx
    movl    %eax, -12(%ebp)
    movl    -12(%ebp), %eax
    jmp     l_fact_exit                 # 16:     return t2
l_fact_5_else_next:
l_fact_0:
l_fact_exit:
    addl    $12, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope print
print:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $0, %esp

    pushl   8(%ebp)                     #  0:     param  0 <- x
    call    Output                      #  1:     call   Output
    addl    $4, %esp
l_print_exit:
    addl    $0, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope test08
main:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $24, %esp

    call    Input                       #  0:     call   t0 <- Input
    addl    $0, %esp
    movl    %eax, -4(%ebp)
    movl    -4(%ebp), %eax              #  1:     assign a <- t0
    movl    %eax, a
    movl    a, %eax                     #  2:     if     a >= 0 goto 2
    cmpl    $0, %eax
    jge     l_test08_2
    jmp     l_test08_3                  #  3:     goto   3
l_test08_2:
    movl    $1, %eax                    #  5:     assign t1 <- 1
    movl    %eax, -8(%ebp)
    jmp     l_test08_5                  #  6:     goto   5
l_test08_3:
    movl    $0, %eax                    #  8:     assign t1 <- 0
    movl    %eax, -8(%ebp)
l_test08_5:
    movl    -8(%ebp), %eax              # 10:     assign b <- t1
    movl    %eax, b
l_test08_7_while_cond:
    movl    b, %eax                     # 12:     if     b = 1 goto 8_while_body
    cmpl    $1, %eax
    je      l_test08_8_while_body
    jmp     l_test08_6                  # 13:     goto   6
l_test08_8_while_body:
    pushl   a                           # 15:     param  0 <- a
    call    fib                         # 16:     call   t2 <- fib
    addl    $4, %esp
    movl    %eax, -12(%ebp)
    pushl   -12(%ebp)                   # 17:     param  0 <- t2
    call    print                       # 18:     call   print
    addl    $4, %esp
    pushl   a                           # 19:     param  0 <- a
    call    fact                        # 20:     call   t3 <- fact
    addl    $4, %esp
    movl    %eax, -16(%ebp)
    pushl   -16(%ebp)                   # 21:     param  0 <- t3
    call    print                       # 22:     call   print
    addl    $4, %esp
    call    Input                       # 23:     call   t4 <- Input
    addl    $0, %esp
    movl    %eax, -20(%ebp)
    movl    -20(%ebp), %eax             # 24:     assign a <- t4
    movl    %eax, a
    movl    a, %eax                     # 25:     if     a >= 0 goto 14
    cmpl    $0, %eax
    jge     l_test08_14
    jmp     l_test08_15                 # 26:     goto   15
l_test08_14:
    movl    $1, %eax                    # 28:     assign t5 <- 1
    movl    %eax, -24(%ebp)
    jmp     l_test08_17                 # 29:     goto   17
l_test08_15:
    movl    $0, %eax                    # 31:     assign t5 <- 0
    movl    %eax, -24(%ebp)
l_test08_17:
    movl    -24(%ebp), %eax             # 33:     assign b <- t5
    movl    %eax, b
    jmp     l_test08_7_while_cond       # 34:     goto   7_while_cond
l_test08_6:
l_test08_exit:
    addl    $24, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # end of text section
    #-----------------------------------------

    #-----------------------------------------
    # global data section
    #
    .data
    .align 4

    # scope: test08

a:	.skip    4		    # <int>
b:	.skip    4		    # <bool>
    # scope: fib

    # scope: fact

    # scope: print

    # end of global data section
    #-----------------------------------------

    .end
##################################################
