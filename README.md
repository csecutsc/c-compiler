# c-compiler
c-like language with:
- built-in function printf
- implicit returns

# Compilation Command
```
clang++ -g compiler.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -O3 -o compiler
```

# Quick compile and interpret
```
./compiler ./example.c &> temp.ll && lli temp.ll
```
