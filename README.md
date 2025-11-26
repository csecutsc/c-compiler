# c-compiler
c-like language

# Compilation Command
```
clang++ -g compiler.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -O3 -o compiler
```
