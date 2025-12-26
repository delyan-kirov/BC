# BC

Simple programming language and calculator

## Build

```sh
make
```

## TODO

1. Use arenas
  - Test more of the arena impl, make sure it's sound 
  - It's probably better to pass the arena around, rather then have a global allocator

2. Remove makefile
  - instead of compile_commands.json, you can use compile_flags.txt
  - using build.cpp instead of makefiles

3. Implement
  - arithmetic operators * / %
  - bit operators | & ~ ^
  - logical operators || && !
  - functions
  - structs
  - unions
  - patterns
  - if, for, when 
  - compile-time functionality

4. Add tracing

5. Add error reporting

6. Types should support streams

7, Interface functions should have top level comments
