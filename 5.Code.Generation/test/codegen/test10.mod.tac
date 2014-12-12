../test/codegen/test10.mod:
[[ module: test10
  [[
    [ *Input() --> <int>     ]
    [ *Output(<int>) --> <NULL>     ]
    [ @a        <int>     ]
    [ @b        <int>     ]
    [ *gcd_iter(<int>,<int>) --> <int>     ]
    [ *gcd_mod(<int>,<int>) --> <int>     ]
    [ *gcd_rec(<int>,<int>) --> <int>     ]
    [ $t0       <int>     ]
    [ $t1       <int>     ]
    [ $t2       <int>     ]
    [ $t3       <int>     ]
    [ $t4       <int>     ]
    [ *test() --> <NULL>     ]
  ]]
  [[ test10
      0:     call   t0 <- Input
      1:     assign a <- t0
      2:     call   t1 <- Input
      3:     assign b <- t1
      4:     param  1 <- b
      5:     param  0 <- a
      6:     call   t2 <- gcd_iter
      7:     param  0 <- t2
      8:     call   Output
      9:     param  1 <- b
     10:     param  0 <- a
     11:     call   t3 <- gcd_mod
     12:     param  0 <- t3
     13:     call   Output
     14:     param  1 <- b
     15:     param  0 <- a
     16:     call   t4 <- gcd_rec
     17:     param  0 <- t4
     18:     call   Output
  ]]

  [[ procedure: gcd_iter
    [[
      [ %a        <int>       ]
      [ %b        <int>       ]
      [ $t0       <int>       ]
      [ $t1       <int>       ]
    ]]
    [[ gcd_iter
        0: 1_while_cond:
        1:     if     a # b goto 2_while_body
        2:     goto   0
        3: 2_while_body:
        4:     if     a > b goto 5_if_true
        5:     goto   6_if_false
        6: 5_if_true:
        7:     sub    t0 <- a, b
        8:     assign a <- t0
        9:     goto   4
       10: 6_if_false:
       11:     sub    t1 <- b, a
       12:     assign b <- t1
       13: 4:
       14:     goto   1_while_cond
       15: 0:
       16:     return a
    ]]
  ]]

  [[ procedure: gcd_mod
    [[
      [ %a        <int>       ]
      [ %b        <int>       ]
      [ $t        <int>       ]
      [ $t0       <int>       ]
      [ $t1       <int>       ]
      [ $t2       <int>       ]
    ]]
    [[ gcd_mod
        0: 1_while_cond:
        1:     if     b # 0 goto 2_while_body
        2:     goto   0
        3: 2_while_body:
        4:     assign t <- b
        5:     div    t0 <- a, t
        6:     mul    t1 <- t0, t
        7:     sub    t2 <- a, t1
        8:     assign b <- t2
        9:     assign a <- t
       10:     goto   1_while_cond
       11: 0:
       12:     return a
    ]]
  ]]

  [[ procedure: test
    [[
      [ $b1       <bool>       ]
      [ $b2       <bool>       ]
      [ $t        <int>       ]
      [ $t0       <int>       ]
      [ $t1       <int>       ]
      [ $t2       <int>       ]
      [ $t3       <int>       ]
      [ $t4       <int>       ]
      [ $t5       <int>       ]
      [ $t6       <bool>       ]
      [ $t7       <bool>       ]
    ]]
    [[ test
        0:     mul    t0 <- t, 2
        1:     assign a <- t0
        2:     mul    t1 <- 2, t
        3:     assign a <- t1
        4:     div    t2 <- t, 2
        5:     assign a <- t2
        6:     div    t3 <- 2, t
        7:     assign a <- t3
        8:     neg    t4 <- t
        9:     assign a <- t4
       10:     pos    t5 <- t
       11:     assign a <- t5
       12:     if     b2 = 1 goto 8
       13:     assign t6 <- 1
       14:     goto   9
       15: 8:
       16:     assign t6 <- 0
       17: 9:
       18:     assign b1 <- t6
       19:     if     b2 = 1 goto 11
       20:     goto   12
       21: 11:
       22:     assign t7 <- 1
       23:     goto   13
       24: 12:
       25:     assign t7 <- 0
       26: 13:
       27:     assign b1 <- t7
    ]]
  ]]

  [[ procedure: gcd_rec
    [[
      [ %a        <int>       ]
      [ %b        <int>       ]
      [ $t0       <int>       ]
      [ $t1       <int>       ]
      [ $t2       <int>       ]
      [ $t3       <int>       ]
    ]]
    [[ gcd_rec
        0:     if     b = 0 goto 1_if_true
        1:     goto   2_if_false
        2: 1_if_true:
        3:     return a
        4:     goto   0
        5: 2_if_false:
        6:     div    t0 <- a, b
        7:     mul    t1 <- t0, b
        8:     sub    t2 <- a, t1
        9:     param  1 <- t2
       10:     param  0 <- b
       11:     call   t3 <- gcd_rec
       12:     return t3
       13: 0:
    ]]
  ]]
]]
