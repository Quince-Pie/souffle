# Souffle: A C Test Framework

### Why Yet Another C Test Framework

There are many C Test Frameworks out there which are well respected and used by many open source projects.

However, after viewing the majority of them, I found them to lack simplicity and tend to add unneccessary friction to the developer.

Souffle aims to be simple and easy to use while offering as much helpful features as possible.



### What to Expect From Souffle

1. Simple test declaration (No main function is needed).
2. Test isolation through vfork to catch crashes (no setjmp/longjmp).
3. Reasonably fast (16k test runs under 300ms).
4. Easy to integrate with your project (with and without build system).
5. Works on modern C2x/C23 compilers and systems.



### Example Output

Output of [examples/basic.c](examples/basic.c):

<pre><code>
=== Test Run Started ===
____________________________________________________________________________

Running 15 tests in 4 suites
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

    🧪 string_test ........................................... <span style="color: #aa0000">[FAILED, 0ms]</span>
	  &gt; [<span style="text-decoration: underline">examples/basic.c:96</span>]:
	  &gt;&gt; Expected: "Hello, World"
	  &gt;&gt; Got: "Hello World!"

    🧪 log_on_pass ........................................... <span style="color: #00aa00">[PASSED, 0ms]</span>
	  This is a log message

⣿ Suite: arr_suite                                                         ⣿
    🧪 array_check ........................................... <span style="color: #aa0000">[FAILED, 0ms]</span>
	  &gt; [<span style="text-decoration: underline">examples/basic.c:69</span>]:
	  &gt;&gt; Expected: [ 1, 2, 3 ]
	  &gt;&gt; Got: [ 1, 4, 3 ]

⣿ Suite: suite_2                                                           ⣿
    🧪 is_true ............................................... <span style="color: #00aa00">[PASSED, 0ms]</span>

____________________________________________________________________________

=== Test Run Summary ===
Total Tests: 15 | <span style="color: #00aa00">Passed</span>: 6 | <span style="color: #aa0000">Failed</span>: 6 | <span style="color: #E850A8">Crashed</span>: 1 | <span style="color: #aa5500">Skipped</span>: 1 | <span style="color: #7f7f7f">Timeout</span>: 1
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


### Documentation


#### Environment Variables

- `SOUFFLE_TIMEOUT` - timeout in seconds.


#### Test Definitions

##### `TEST(suite, test_name)`

In order to define a test, all you simply need to do is by defining it using the macro followed by function brackets.


##### `SETUP(suite, test_name)`

Used for setting up the test before executing it.

every `TEST` and `SETUP` provides `void **ctx` field that may freely use for context between the test and the setup phase.


##### `TEARDOWN(suite, test_name)`

if your `SETUP` phase allocates or if you wish to clean up your test, `TEARDOWN` is used to define how you would teardown your setup/test.

#### Assertions

##### `ASSERT_TRUE(expected)`

checks: expected == true

##### `ASSERT_FALSE(expected)`

checks: expected == false

##### `ASSERT_EQ(expected, actual)`

checks: expected == actual

Used for generic assertions for various basic types such as int and floats.

##### `ASSERT_NE(expected, actual)`

checks: expected != actual

Used for generic assertions for various basic types such as int and floats.

##### `ASSERT_PTR_EQ(expected, actual)`

checks if both pointers are equal, will output the location for both pointers on failure.

##### `ASSERT_PTR_NE(expected, actual)`

checks if both pointers are NOT equal, will output the location for both pointers on failure.

##### `ASSERT_NULL(val)`

checks: val == NULL.

##### `ASSERT_NOT_NULL(val)`

checks: val != NULL.

##### `ASSERT_GT(expected, actual)`

checks: expected > actual

Used for generic assertions for various basic types such as int and floats.

##### `ASSERT_LT(expected, actual)`

checks: expected < actual

Used for generic assertions for various basic types such as int and floats.

##### `ASSERT_GTE(expected, actual)`

checks: expected >= actual

Used for generic assertions for various basic types such as int and floats.

##### `ASSERT_LTE(expected, actual)`

checks: expected <= actual

Used for generic assertions for various basic types such as int and floats.

##### `ASSERT_UINT_ARR_EQ(expected, actual)`

checks: expected == actual for every element in the array (any unsigned type).

Used for generic assertions for various basic types such as int and floats.

This assertion will print both arrays on failure.

##### `ASSERT_INT_ARR_EQ(expected, actual)`

checks: expected == actual for every element in the array (any signed type).

This assertion will print both arrays on failure.

##### `ASSERT_FLOAT_ARR_EQ(expected, actual)`

checks: expected == actual for every element in the array (any float type).

This assertion will print both arrays on failure.


##### `ASSERT_STR_EQ(str1, str2)`

checks: if both strings are equal.

This assertion will print both values on failure.

##### `ASSERT_STR_NE(str1, str2)`

checks: if both strings are NOT equal.

This assertion will print both values on failure.



#### Utility Functions

##### `LOG_MSG(msg, args)`

Can be used to log any message (this function should be used instead of printf for the test).

##### `LOG_TRACE_MSG(msg, args)`

Can be used to log any message with a trace header (file:line).


##### `SKIP_TEST()`

Can be used to skip the test at any time (unless a failure happens before it).

##### `FAIL_TEST()`

Causes the test to fail immediately. This can be helpful for creating a custom assertion when used with LOG_MSG() and LOG_TRACE_MSG().

