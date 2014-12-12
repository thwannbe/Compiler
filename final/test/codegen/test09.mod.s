##################################################
# test09
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

    # scope sum_rec
sum_rec:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $12, %esp

    movl    8(%ebp), %eax               #  0:     if     n > 0 goto 1_if_true
    cmpl    $0, %eax
    jg      l_sum_rec_1_if_true
    jmp     l_sum_rec_2_if_false        #  1:     goto   2_if_false
l_sum_rec_1_if_true:
    movl    8(%ebp), %eax               #  3:     sub    t0 <- n, 1
    subl    $1, %eax
    movl    %eax, -4(%ebp)
    pushl   -4(%ebp)                    #  4:     param  0 <- t0
    call    sum_rec                     #  5:     call   t1 <- sum_rec
    addl    $4, %esp
    movl    %eax, -8(%ebp)
    movl    8(%ebp), %eax               #  6:     add    t2 <- n, t1
    addl    -8(%ebp), %eax
    movl    %eax, -12(%ebp)
    movl    -12(%ebp), %eax
    jmp     l_sum_rec_exit              #  7:     return t2
    jmp     l_sum_rec_0                 #  8:     goto   0
l_sum_rec_2_if_false:
    movl    $0, %eax
    jmp     l_sum_rec_exit              # 10:     return 0
l_sum_rec_0:
l_sum_rec_exit:
    addl    $12, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope sum_iter
sum_iter:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $16, %esp

    movl    $0, %eax                    #  0:     assign sum <- 0
    movl    %eax, -8(%ebp)
    movl    $0, %eax                    #  1:     assign i <- 0
    movl    %eax, -4(%ebp)
l_sum_iter_3_while_cond:
    movl    -4(%ebp), %eax              #  3:     if     i <= n goto 4_while_body
    cmpl    8(%ebp), %eax
    jle     l_sum_iter_4_while_body
    jmp     l_sum_iter_2                #  4:     goto   2
l_sum_iter_4_while_body:
    movl    -8(%ebp), %eax              #  6:     add    t0 <- sum, i
    addl    -4(%ebp), %eax
    movl    %eax, -12(%ebp)
    movl    -12(%ebp), %eax             #  7:     assign sum <- t0
    movl    %eax, -8(%ebp)
    movl    -4(%ebp), %eax              #  8:     add    t1 <- i, 1
    addl    $1, %eax
    movl    %eax, -16(%ebp)
    movl    -16(%ebp), %eax             #  9:     assign i <- t1
    movl    %eax, -4(%ebp)
    jmp     l_sum_iter_3_while_cond     # 10:     goto   3_while_cond
l_sum_iter_2:
    movl    -8(%ebp), %eax
    jmp     l_sum_iter_exit             # 12:     return sum
l_sum_iter_exit:
    addl    $16, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope sum_alg
sum_alg:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $12, %esp

    movl    8(%ebp), %eax               #  0:     add    t0 <- n, 1
    addl    $1, %eax
    movl    %eax, -4(%ebp)
    movl    8(%ebp), %eax               #  1:     mul    t1 <- n, t0
    movl    -4(%ebp), %ebx
    imull   %ebx
    movl    %eax, -8(%ebp)
    movl    -8(%ebp), %eax              #  2:     div    t2 <- t1, 2
    cdq
    movl    $2, %ebx
    idivl   %ebx
    movl    %eax, -12(%ebp)
    movl    -12(%ebp), %eax
    jmp     l_sum_alg_exit              #  3:     return t2
l_sum_alg_exit:
    addl    $12, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope test09
main:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $20, %esp

    call    Input                       #  0:     call   t0 <- Input
    addl    $0, %esp
    movl    %eax, -4(%ebp)
    movl    -4(%ebp), %eax              #  1:     assign a <- t0
    movl    %eax, a
l_test09_2_while_cond:
    movl    a, %eax                     #  3:     if     a >= 0 goto 3_while_body
    cmpl    $0, %eax
    jge     l_test09_3_while_body
    jmp     l_test09_1                  #  4:     goto   1
l_test09_3_while_body:
    pushl   a                           #  6:     param  0 <- a
    call    sum_rec                     #  7:     call   t1 <- sum_rec
    addl    $4, %esp
    movl    %eax, -8(%ebp)
    pushl   -8(%ebp)                    #  8:     param  0 <- t1
    call    Output                      #  9:     call   Output
    addl    $4, %esp
    pushl   a                           # 10:     param  0 <- a
    call    sum_iter                    # 11:     call   t2 <- sum_iter
    addl    $4, %esp
    movl    %eax, -12(%ebp)
    pushl   -12(%ebp)                   # 12:     param  0 <- t2
    call    Output                      # 13:     call   Output
    addl    $4, %esp
    pushl   a                           # 14:     param  0 <- a
    call    sum_alg                     # 15:     call   t3 <- sum_alg
    addl    $4, %esp
    movl    %eax, -16(%ebp)
    pushl   -16(%ebp)                   # 16:     param  0 <- t3
    call    Output                      # 17:     call   Output
    addl    $4, %esp
    call    Input                       # 18:     call   t4 <- Input
    addl    $0, %esp
    movl    %eax, -20(%ebp)
    movl    -20(%ebp), %eax             # 19:     assign a <- t4
    movl    %eax, a
    jmp     l_test09_2_while_cond       # 20:     goto   2_while_cond
l_test09_1:
l_test09_exit:
    addl    $20, %esp
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

    # scope: test09

a:	.skip    4		    # <int>
    # scope: sum_rec

    # scope: sum_iter

    # scope: sum_alg

    # end of global data section
    #-----------------------------------------

    .end
##################################################
