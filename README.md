# c-compiler
c-like language with:
- built-in function printf
- implicit returns

# Compilation Command
```
clang++ -g compiler.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -o compiler
```

# Bugs
- cant print multiple times, due to redeclaration of print ptr
- existing linker is hacky and within the compiler

# Quick compile and interpret
```
./compiler ./example.c &> temp.ll && lli temp.ll
```
