; ModuleID = 'module_name'
source_filename = "module_name"

declare i32 @printf(i8*, ...)
@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

define double @test(double %x) {
entry:
  %addtmp = fadd double 3.000000e+00, %x
  ret double %addtmp
}


define i32 @main() #0 {
  %fmt_ptr = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i32 0, i32 0
  %calltmp = call double @test(double 1.500000e+00)

  %result = call i32 (i8*, ...) @printf(i8* %fmt_ptr, double %calltmp)
  ret i32 0 
}

