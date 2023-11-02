# Souffle: A C Test Framework

#### Warning: This Library is Currently a Work in Progress (WIP)

There are few rough edges and tweaks that are necessary for this library to be considered stable.


### Why Yet Another C Test Framework

There are many C Test Frameworks out there which are well respected and used by many open source projects. 

However, after viewing the majority of them, I found them to lack simplicity and tend to add unneccessary friction to the developer.

Souffle aims to be simple and easy to use while offering as much helpful features as possible.



### What to Expect From Souffle

1. Simple test declaration (No main function is needed).
2. Reasonably fast.
3. Type checking assertions.
  - All assert functions check for type mismatch at compile time.
4. Easy to integrate with your project (with and without build system).
5. Works across modern compilers and systems.


### Build from Source

#### Requirements:

- A modern C compiler such as GCC and Clang (Souffle currently targets C23).
- khash provided by klib (available in git submodule) for test registeration.


To build Souffle, simply create your test file and add souffle.c next to it when compiling.

```sh
  $ gcc examples/basic.c souffle.c -g
```


### WIP

- Add support for MSVC.
- Add more macro assertions to simplify testing.
- Implement setup and teardown phase.
