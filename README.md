# SHREK Lang

SHREK is a programming language that only uses the characters "SHREK!" SHREK is stack based, where commands manipulate the top values on the stack.

## Reading this document

`{<num>}` will be used to represent an item on the stack *before* a command is executed. `{0}` represents the value on the top of the stack. The number will count up, so `{1}` represents the value just below the top of the stack.

Some commands use the top of the stack to determine sub-functionality. For example, the `jump` command uses `{0}` to determine the jump type. Some jump types will inspect the value after `{0}` (written as `{1}`) to conditionally jump. The numbers will always be based on the state of the stack before the command is executed.

`!!` will be used to represent a label.

## Examples

### Example 1: Counting

The below SHREK program will output the values 0, 1, 2 three times.

```text
SRRR # Counter value, set to 3

!R!
S # 0 value
SRE # Push 1/output, call func

R # Bump 0 to 1
SRE # Push 1/output, call func

R # Bump 1 to 2
SRE # Push 1/output, call func

H # Pop stack

# Subtract 1 from counter
SR # Push 1 to stack
SRRRE # Call subtract func (3) to {1} - {0}

SRK!E! # Jump if counter 0
SK!R! # Jump to !R!
!E!
```

## Syntax

|Letter|Command|Description|
|------|-------|-----------|
|S|push0|Pushes 0 to the top of the stack|
|H|pop|Pops the top of the stack|
|R|bump|Adds one to the top of the stack|
|E|func|Calls a function based on the value at `{0}`|
|K|jump|Jump to `!!` based on the value at `{0}`|
|!|label|Used to define a label|
|#|comment|Python-like comment. The `#` to the end of the line will be a comment (and can contain any character)

## Labels

Labels are defined with an opening and closing `!` character. Labels can only use the letters in "SHREK". For example `!S!` will define the label `S`.

## Jump Command

When jumping, the value at `{0}` defines what type of jump to perform. A jump command must be followed by a label. If the target label is not defined, the program will terminate upon the jump.

The jump command will remove `{0}` from the stack.

|`{0}` Value|Command|Description|
|-----------|-------|-----------|
|0|jump|Always jump to label|
|1|jump 0|Jump to label if `{1}` == 0|
|2|jump neg|Jump to label if `{1}` < 0|

## Func Command

The func command uses the top of the stack to determine which function to call. Functions are mapped in a function table. See the [C Extension API](#c-extension-api) for writing and registering custom functions.

The func command will remove `{0}` from the stack.

SHREK comes with the following built-in commands.

### 0. Input
Read string from stdin and place on stack. The string will be added to the stack in reverse order, so popping the stack will return the string in the correct order. Strings will be null terminated.

### 1. Output
|Write `{1}` to stdout. `{1}` will not be popped by this function

### 2. Add

Add `{2}` to `{1}`. `{1}` and `{2}` will be popped, and the result will be placed on the top of the stack.

### 3. Subtract
Subtract `{2}` from `{1}`. `{1}` and `{2}` will be popped, and the result will be placed on the top of the stack.

### 4. Multiply
Multiply `{2}` by `{1}`. `{1}` and `{2}` will be popped, and the result will be placed on the top of the stack.

### 5. Divide (integer division)
Divide `{2}` by `{1}`. `{1}` and `{2}` will be popped, and the result will be placed on the top of the stack.

### 6. Mod
Get the remainder of dividing `{2}` by `{1}`. `{1}` and `{2}` will be popped, and the result will be placed on the top of the stack.

### 7. Double

Double the value of `{1}`.

### 8. Negate

Multiple `{1}` by -1.

### 9. Clone

Put a copy of `{1}` on the top of the stack

## C Extension API

You might be thinking, "SHREK can't do everything." But, that's where you are wrong. SHREK comes with a C Extension API where you can write anything your heart desires.

### Creating an Extension

To make an extension, make a shared library and include `shrek.h`. This library must be linked against the shrek1 library (platform specific details below). All custom functions must be registered with a function with a signature of:

```
int <name of library file>_register(ShrekHandle* shrek)
```

This function must call `shrek_register_func` for each function to register. This call will set the function number in the SHREK function table.

Extension modules are discovered by looking in the current working directory for any file with a `.dnky` extension.

### Windows Linking

To link with Visual Studio, configure `Linker>General>Additional Library Directories` with the path to the location that contains `shrek1.dll` and `shrek1.lib`. Set `C/C++>General>Additional Include Directories` to include the path that contains `shrek.h`.

Remember to build the shared library with the same architecture as the shrek.exe runtime! The DLL/extension will fail to load at runtime if there is an architecture mismatch. There will be linker errors if the wrong architecture is used when building the extension.

### C Extension API

#### `typedef int (*ShrekFunc)(ShrekHandle*);`

The type signature for extension methods. Methods must take a shrek instance and return an integer. Use the stack functions to get values from the runtime.

Extension methods must return `SHREK_OK` on success and `SHREK_ERROR` on failure.

#### `int shrek_register_func(ShrekHandle* shrek, int func_number, ShrekFunc func);`

Used to register a function to the given shrek instance. Returns `SHREK_ERROR` if the function could not be registered.

The `func_number` will determine the number used to call the method in `func` (`E`) commands. If a function is already registered with the given function number, the function will error.

#### `void shrek_set_except(ShrekHandle* shrek, const char* errmsg);`

Set the error message to display if the extension function returns an unsuccessful code. For example, the following extension method will always make the runtime error.

```c
int always_error(ShrekHandle* shrek)
{
    shrek_set_except(shrek, "Oh noes, it broke");
    return SHREK_ERROR;
}
```

#### `int shrek_stack_size(ShrekHandle* shrek);`

Returns the number of values on the stack.

#### `int shrek_pop(ShrekHandle* shrek, int* out_value);`

Sets `out_value` to the value of the top of the stack, and pops the value from the stack. Returns `SHREK_ERROR` if a value was not popped.

#### `int shrek_push(ShrekHandle* shrek, int value);`

Push `value` to the stack. Returns `SHREK_ERROR` if the value could not be added.

#### `int shrek_peek(ShrekHandle* shrek, int* out_value);`

Sets `out_value` to the value at the top of the stack, but does not pop the stack. Returns `SHREK_ERROR` if the value could not be peeked.
