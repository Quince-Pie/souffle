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



### Example Output

Output of [examples/basic.c](examples/basic.c):

<pre><code>
=== Test Run Started ===
____________________________________________________________________________

Running 13 tests in 4 suites
____________________________________________________________________________

⣿ Suite: EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE            ⣿
    🧪 EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE ...... <span style="color: #00aa00">[PASSED, 0ms]</span>

⣿ Suite: main_suite                                                        ⣿
  ⚙ 🧪 TestCase1 ............................................. <span style="color: #00aa00">[PASSED, 0ms]</span>
    🧪 test_number_eq ........................................ <span style="color: #aa0000">[FAILED, 0ms]</span>
	  &gt; [<span style="text-decoration: underline">examples/basic.c:27</span>]:
	  &gt;&gt; Expected: "5"
	  &gt;&gt; Got: "1"

    🧪 exception_test ........................................ <span style="color: #E850A8">[CRASHED, ☠ ]</span>
    🧪 pass .................................................. <span style="color: #00aa00">[PASSED, 0ms]</span>
    🧪 pass_fail_pass ........................................ <span style="color: #aa0000">[FAILED, 0ms]</span>
	  &gt; [<span style="text-decoration: underline">examples/basic.c:43</span>]:
	  &gt;&gt; Expected: "2"
	  &gt;&gt; Got: "1"

    🧪 float_check ........................................... <span style="color: #aa0000">[FAILED, 0ms]</span>
	  &gt; [<span style="text-decoration: underline">examples/basic.c:47</span>]:
	  &gt;&gt; Expected: "1.500000"
	  &gt;&gt; Got: "2.500000"

    🧪 pass_fail ............................................. <span style="color: #aa0000">[FAILED, 0ms]</span>
	  &gt; [<span style="text-decoration: underline">examples/basic.c:51</span>]:
	  &gt;&gt; Expected: "2"
	  &gt;&gt; Got: "1"

    🧪 skip_me ............................................... <span style="color: #aa5500">[SKIPPED, ⏭ ]</span>
    🧪 long_test ............................................. <span style="color: #00aa00">[PASSED, 3000ms]</span>
    🧪 timeout_test .......................................... <span style="color: #7f7f7f">[TIMEOUT, ⧖ ]</span>

⣿ Suite: arr_suite                                                         ⣿
    🧪 array_check ........................................... <span style="color: #00aa00">[PASSED, 0ms]</span>

⣿ Suite: suite_2                                                           ⣿
    🧪 is_true ............................................... <span style="color: #00aa00">[PASSED, 0ms]</span>

____________________________________________________________________________
=== Test Run Summary ===
Total Tests: 13 | <span style="color: #00aa00">Passed</span>: 6 | <span style="color: #aa0000">Failed</span>: 4 | <span style="color: #E850A8">Crashed</span>: 1 | <span style="color: #aa5500">Skipped</span>: 1 | <span style="color: #7f7f7f">Timeout</span>: 1
____________________________________________________________________________
</code>
</pre>


### Build from Source

#### Requirements:

- A modern C compiler such as GCC and Clang (Souffle currently targets C23 but clang-cl is also supported).
- khash provided by klib (available in git submodule) for test registeration.


To build Souffle, simply create your test file and add souffle.c next to it when compiling.

```sh
  $ gcc examples/basic.c souffle.c -g    # Optional: -DSOUFFLE_NOCOLOR to disable color output
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

##### `ASSERT_FAIL(custom_msg)`

Causes test to fail immediately with a custom error message.

This can be useful for any user defined assertion.

##### `ASSERT_TRUE(expected)`

checks: expected == true

##### `ASSERT_FALSE(expected)`

checks: expected == false

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



##### `SKIP_TEST()`

Can be used to skip the test at any time (unless a failure happens before it).


#### Meson Integration

Souffle can be used with and without a build system.
To use Souffle inside your meson project you can use the following:

##### souffle.wrap:
```ini
[wrap-git]
url = https://codeberg.org/QuincePie/souffle.git
revision = head
depth = 1
clone-recursive = true

[provide]
souffle = souffle_dep
```

##### Your Meson.build:

```meson
project('meson_example', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c2x'])


souffle_dep = dependency('souffle',
  # default_options: ['no_color=true'], # OPTIONAL: If you wish to disable color output.
  fallback: ['souffle', 'souffle_dep'],
)

exe = executable('meson_example',
   'meson_example.c',
   dependencies: [souffle_dep])

test('basic', exe)
```
