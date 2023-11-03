# Souffle: A C Test Framework

#### Warning: This Library is Currently a Work in Progress (WIP)

There are few rough edges and tweaks that are necessary for this library to be considered stable.


### Why Yet Another C Test Framework

There are many C Test Frameworks out there which are well respected and used by many open source projects. 

However, after viewing the majority of them, I found them to lack simplicity and tend to add unneccessary friction to the developer.

Souffle aims to be simple and easy to use while offering as much helpful features as possible.



### What to Expect From Souffle

1. Simple test declaration (No main function is needed).
2. No setjmps / longjmps (vfork is utilized instead).
3. Reasonably fast.
4. Type checking assertions.
  - All assert functions check for type mismatch at compile time.
5. Easy to integrate with your project (with and without build system).
6. Works on modern C2x/C23 compilers and systems.


### Build from Source

#### Requirements:

- A modern C compiler such as GCC and Clang (Souffle currently targets C23 but clang-cl is also supported).
- khash provided by klib (available in git submodule) for test registeration.


To build Souffle, simply create your test file and add souffle.c next to it when compiling.

```sh
  $ gcc examples/basic.c souffle.c -g
```

### Documentation

Please view the examples under the examples folder for an example usage.

#### Environment Variables

- `SOUFFLE_TIMEOUT` - timeout in seconds.


#### Test Definitions

##### `TEST(suite, test_name)`

In order to define a test, all you simply need to do is by defining it using the macro followed by function brackets.


##### `SETUP(suite, test_name)`

Used for setting up the test before executing it.

every `TEST` and `SETUP` provides `void **ctx` field that may freely use for context between the test and the setup phase.


##### `TEARDOWN(suite, test_name)`

if your `SETUP` phase allocates or if you wish so clean up your test, `TEARDOWN` is used to define how you would teardown your setup/test.

#### Assertions

##### `ASSERT_EQ(expected, actual)`

checks: expected == actual

Used for generic assertions for various basic types such as int and floats. This function includes a static assertion for a type check, so your types must be matching or casted correctly.

 
##### `ASSERT_NE(expected, actual)`

checks: expected != actual

Used for generic assertions for various basic types such as int and floats. This function includes a static assertion for a type check, so your types must be matching or casted correctly.

##### `ASSERT_GE(expected, actual)`

checks: expected >= actual

Used for generic assertions for various basic types such as int and floats. This function includes a static assertion for a type check, so your types must be matching or casted correctly.

##### `ASSERT_LE(expected, actual)`

checks: expected <= actual

Used for generic assertions for various basic types such as int and floats. This function includes a static assertion for a type check, so your types must be matching or casted correctly.

##### `ASSERT_ARR_EQ(expected, actual)`

checks: expected == actual for every element in the array.

Used for generic assertions for various basic types such as int and floats. This function includes a static assertion for a type check on the first element, so your types must be matching or casted correctly.

This assertion currently fails immediately on the first mismatch.

(*NOTE: This behavior may change in the future)



##### `ASSERT_NULL(val)`

checks if val is null.

##### `ASSERT_NOT_NULL(val)`

checks if val is NOT null.
