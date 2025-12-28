# BC

Simple programming language and calculator

## Build

```sh
make
```

## TODO

- Re-implement tokenizer
  - Token-groups should be generated explicitly by the lexer
  - Don't deal with offsets, use the groups themselves
  - The tokenizer should handle negative integers itself
  - The tokenizer should implement error reporting

- Remove makefile
  - instead of compile_commands.json, you can use compile_flags.txt
  - using build.cpp instead of makefiles

- Implement
  - bit operators | & ~ ^
  - logical operators || && !
  - functions
  - structs
  - unions
  - patterns
  - if, for, when
  - compile-time functionality

- Add tracing

- Add error reporting

- Types should support streams

- Interface functions should have top level comments
