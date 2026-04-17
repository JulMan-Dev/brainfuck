# brainfuck

This is an experimental implementation of a brainfuck compiler targeting LLVM IR.

This relies on the cross-platform target of LLVM.

```bash
$ cat > A.bf <<EOF
>++++++++[<++++++++>-]<+.
EOF
$ brainfuck -llvm -o A.ll A.bf
$ clang -o A A.ll 
```

# For now

For now, it justs prints out the AST dump of the file.

```bash
$ cat > plus.bf <<EOF
++++----
EOF
$ brainfuck plus.bf
+ (x4)
- (x4)
<eof>
consumed 9 bytes
```
