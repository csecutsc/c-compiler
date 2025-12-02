declare i32 @printf(i8*, ...)
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
define i64 @myfunadd(i64 %a, i64 %b) {
entry:
  %addreg = add i64 %a, %b
  ret i64 %addreg
}

define i64 @myfunsub(i64 %a, i64 %b) {
entry:
  %subreg = sub i64 %a, %b
  ret i64 %subreg
}

define i64 @myfunmul(i64 %a, i64 %b) {
entry:
  %mulreg = mul i64 %a, %b
  ret i64 %mulreg
}

define i64 @myfundiv(i64 %a, i64 %b) {
entry:
  %divreg = udiv exact i64 %a, %b
  ret i64 %divreg
}

define i64 @main() {
entry:
  %callreg = call i64 @myfunadd(i64 3, i64 2)
  %a = add i64 %callreg, 0
  %callreg1 = call i64 @myfunsub(i64 3, i64 2)
  %b = add i64 %callreg1, 0
  %callreg2 = call i64 @myfunmul(i64 3, i64 2)
  %c = add i64 %callreg2, 0
  %callreg3 = call i64 @myfundiv(i64 4, i64 2)
  %d = add i64 %callreg3, 0
  %callreg4 = call i32 (ptr, ...) @printf(ptr @.str, i64 %callreg3)
  ret i64 0
}

