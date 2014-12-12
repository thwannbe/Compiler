##################################################
# test10
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

    # scope gcd_iter
gcd_iter:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $8, %esp

l_gcd_iter_1_while_cond:
    movl    8(%ebp), %eax               #  1:     if     a # b goto 2_while_body
    cmpl    12(%ebp), %eax
    jne     l_gcd_iter_2_while_body
    jmp     l_gcd_iter_0                #  2:     goto   0
l_gcd_iter_2_while_body:
    movl    8(%ebp), %eax               #  4:     if     a > b goto 5_if_true
    cmpl    12(%ebp), %eax
    jg      l_gcd_iter_5_if_true
    jmp     l_gcd_iter_6_if_false       #  5:     goto   6_if_false
l_gcd_iter_5_if_true:
    movl    8(%ebp), %eax               #  7:     sub    t0 <- a, b
    subl    12(%ebp), %eax
    movl    %eax, -4(%ebp)
    movl    -4(%ebp), %eax              #  8:     assign a <- t0
    movl    %eax, 8(%ebp)
    jmp     l_gcd_iter_4_while_next     #  9:     goto   4_while_next
l_gcd_iter_6_if_false:
    movl    12(%ebp), %eax              # 11:     sub    t1 <- b, a
    subl    8(%ebp), %eax
    movl    %eax, -8(%ebp)
    movl    -8(%ebp), %eax              # 12:     assign b <- t1
    movl    %eax, 12(%ebp)
l_gcd_iter_4_while_next:
    jmp     l_gcd_iter_1_while_cond     # 14:     goto   1_while_cond
l_gcd_iter_0:
    movl    8(%ebp), %eax
    jmp     l_gcd_iter_exit             # 16:     return a
l_gcd_iter_exit:
    addl    $8, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope gcd_mod
gcd_mod:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $16, %esp

l_gcd_mod_1_while_cond:
    movl    12(%ebp), %eax              #  1:     if     b # 0 goto 2_while_body
    cmpl    $0, %eax
    jne     l_gcd_mod_2_while_body
    jmp     l_gcd_mod_0                 #  2:     goto   0
l_gcd_mod_2_while_body:
    movl    12(%ebp), %eax              #  4:     assign t <- b
    movl    %eax, -4(%ebp)
    movl    8(%ebp), %eax               #  5:     div    t0 <- a, t
    cdq
    movl    -4(%ebp), %ebx
    idivl   %ebx
    movl    %eax, -8(%ebp)
    movl    -8(%ebp), %eax              #  6:     mul    t1 <- t0, t
    movl    -4(%ebp), %ebx
    imull   %ebx
    movl    %eax, -12(%ebp)
    movl    8(%ebp), %eax               #  7:     sub    t2 <- a, t1
    subl    -12(%ebp), %eax
    movl    %eax, -16(%ebp)
    movl    -16(%ebp), %eax             #  8:     assign b <- t2
    movl    %eax, 12(%ebp)
    movl    -4(%ebp), %eax              #  9:     assign a <- t
    movl    %eax, 8(%ebp)
    jmp     l_gcd_mod_1_while_cond      # 10:     goto   1_while_cond
l_gcd_mod_0:
    movl    8(%ebp), %eax
    jmp     l_gcd_mod_exit              # 12:     return a
l_gcd_mod_exit:
    addl    $16, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope gcd_rec
gcd_rec:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    subl    $16, %esp

    movl    12(%ebp), %eax              #  0:     if     b = 0 goto 1_if_true
    cmpl    $0, %eax
    je      l_gcd_rec_1_if_true
    jmp     l_gcd_rec_2_if_false        #  1:     goto   2_if_false
l_gcd_rec_1_if_true:
    movl    8(%ebp), %eax
    jmp     l_gcd_rec_exit              #  3:     return a
    jmp     l_gcd_rec_0                 #  4:     goto   0
l_gcd_rec_2_if_false:
    movl    8(%ebp), %eax               #  6:     div    t0 <- a, b
    cdq
    movl    12(%ebp), %ebx
    idivl   %ebx
    movl    %eax, -4(%ebp)
    movl    -4(%ebp), %eax              #  7:     mul    t1 <- t0, b
    movl    12(%ebp), %ebx
    imull   %ebx
    movl    %eax, -8(%ebp)
    movl    8(%ebp), %eax               #  8:     sub    t2 <- a, t1
    subl    -8(%ebp), %eax
    movl    %eax, -12(%ebp)
    pushl   -12(%ebp)                   #  9:     param  1 <- t2
    pushl   12(%ebp)                    # 10:     param  0 <- b
    call    gcd_rec                     # 11:     call   t3 <- gcd_rec
    addl    $8, %esp
    movl    %eax, -16(%ebp)
    movl    -16(%ebp), %eax
    jmp     l_gcd_rec_exit              # 12:     return t3
l_gcd_rec_0:
l_gcd_rec_exit:
    addl    $16, %esp
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

    # scope test10
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
    call    Input                       #  2:     call   t1 <- Input
    addl    $0, %esp
    movl    %eax, -8(%ebp)
    movl    -8(%ebp), %eax              #  3:     assign b <- t1
    movl    %eax, b
    pushl   b                           #  4:     param  1 <- b
    pushl   a                           #  5:     param  0 <- a
    call    gcd_iter                    #  6:     call   t2 <- gcd_iter
    addl    $8, %esp
    movl    %eax, -12(%ebp)
    pushl   -12(%ebp)                   #  7:     param  0 <- t2
    call    Output                      #  8:     call   Output
    addl    $4, %esp
    pushl   b                           #  9:     param  1 <- b
    pushl   a                           # 10:     param  0 <- a
    call    gcd_mod                     # 11:     call   t3 <- gcd_mod
    addl    $8, %esp
    movl    %eax, -16(%ebp)
    pushl   -16(%ebp)                   # 12:     param  0 <- t3
    call    Output                      # 13:     call   Output
    addl    $4, %esp
    pushl   b                           # 14:     param  1 <- b
    pushl   a                           # 15:     param  0 <- a
    call    gcd_rec                     # 16:     call   t4 <- gcd_rec
    addl    $8, %esp
    movl    %eax, -20(%ebp)
    pushl   -20(%ebp)                   # 17:     param  0 <- t4
    call    Output                      # 18:     call   Output
    addl    $4, %esp
l_test10_exit:
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

    # scope: test10

a:	.skip    4		    # <int>
b:	.skip    4		    # <int>
    # scope: gcd_iter

    # scope: gcd_mod

    # scope: gcd_rec

    # end of global data section
    #-----------------------------------------

    .end
##################################################
