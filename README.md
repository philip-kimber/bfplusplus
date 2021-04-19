# Brainfuck++
Brainfuck with functions

This interpreter supports the basic [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) concepts and instructions, that the program is run on a tape of numbers manipulated by the following single-character instructions:
```
+ increment the value at the current position
- decrement the value at the current position
> move the tape pointer to the right (ie point to the next cell)
< move the tape pointer to the left (ie point to the previous cell)
[ open loop: skips over the code until ] if the value at the current position is 0
] close loop: skips back to its paired [ if the value at the current position is not 0
, get a character from the standard input and put its ASCII value in the current position
. print a character to the standard output, using ASCII of value at current position
```

In addition to these, Brainfuck++ adds several instructions for dealing with functions:
```
{ open function
} close function

( open function call
) close function call
```

#### Function definitions

Within function brackets (between `{` and `}`), we simply write instructions to be run as that function. For example, a very simple function to print the character 'H' could (crudely) be defined as this:
```
{
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
}
```
(all indentation is optional and only for readability)

This defines a function at the current position, that can be called to simply print 'H'. Functions can include any of the BF++ instructions and work almost in the same way as the global tape does. When called, a function runs on a new tape independent of any others, and so any calculations, values etc are encapsulated and do not affect the global scope.

#### Calling functions

For a simple function taking no arguments, we would have to do the following:
* Load the address of the desired function into the current cell (ie increment the cell until its value is the cell index of a defined function)
* Run the sequence `()`

This would then run the function at that given address. Since each function call has its own tape, it has no access to data from other scopes. Therefore, we want to be able to pass values to and from functions. This is done by specifying the number of cells to be copied into (as arguments) and out of (as return values) the function tape, using `+` and `-` instructions inside the `()` call brackets.

The `-` instruction is used to indicate how many arguments to pass to the function. The number of minuses indicates how far back to decrement the caller's tape pointer before pushing arguments to the function as it is re-incremented to the original position. This might be better explained with an example.

Assume a function loaded into address 2 being called with `(---)`. The calling (global) tape might look like this before the call:
```
value:[ 0 ][ 0 ][ FN][ 0 ][120][34 ][27 ][ 2 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][
index:[ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ][ 8 ][ 9 ][10 ][11 ][12 ][...
ptr  :                                     ^
```
The caller's pointer is moved back 3 spaces to be position 4 (value here 120) and then arguments are pushed to the new tape for the function, incrementing both tapes' pointers at once. This means that the caller's pointer will be returned to index 7 as before, but the new tape will look like this:
```
value:[120][34 ][27 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][
index:[ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ][ 8 ][ 9 ][10 ][11 ][12 ][...
ptr  :                 ^
```

We are likely to want to return some values from the function. This is done based on the position of the function's pointer at the end of its execution, and the number of values to return must be specified in the call brackets with `+` characters in a similar way to how the argument count indicators are processed. If we imagine that the function in our example was called with `(---++)` instead and it finished executing with its tape looking like this:
```
value:[120][34 ][27 ][57 ][93 ][21 ][72 ][12 ][ 0 ][ 0 ][ 0 ][ 0 ][ 0 ][
index:[ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ][ 8 ][ 9 ][10 ][11 ][12 ][...
ptr  :                           ^
```
Two return values would be pulled from the function call starting with the one currently pointed to. The caller's pointer is also incremented although slightly asymmetrically, leading to it looking like this:
```
value:[ 0 ][ 0 ][ FN][ 0 ][120][34 ][27 ][ 2 ][21 ][72 ][ 0 ][ 0 ][ 0 ][
index:[ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ][ 8 ][ 9 ][10 ][11 ][12 ][...
ptr  :                                               ^
```
There is reason to the slight madness of exactly where the pointer points (that it increments the function's pointer *after* pulling a value, but increments the caller's pointer beforehand). The main aims are that:
* The address for the function call is preserved in case the caller wants to call again or use it for something
* The function can choose any position on its tape to return from: if its pointer were incremented before taking the return value, it would not be possible to return the first value on the tape, since it does not wrap
* Incrementing the function's pointer after pushing an argument means that the function code can assume that the its pointer begins by pointing at a free cell initialised to zero
    * A key caveat to this is that a function cannot know the index of this cell or how many arguments have been passed. A way to assert the presence of a certain number of arguments could be to move the tape pointer left with `<` until the intended first cell, and an error will be thrown if not enough arguments have been passed

It's also worth noting that the convention of writing the argument count and return count like `(----+)` (send 4 arguments and return 1), although intuitive, is not compulsory: the interpreter just counts the number of `+` and `-` instructions between the brackets. So the above could equally be written as `(-+---)` and would have the same effect.

#### Call scope decorators
Brainfuck++ also adds two final instructions:
```
' (single quote, ASCII 0x27) scope up call decorator
@ global scope call decorator
```

These are placed within function call brackets to indicate where to find the function definition. Because functions are accessed as an index of a tape based on the current cell value, it is not easily possible for a function to call itself (recursion) or call other functions that are not local to its 'scope'.

To rectify this, we can direct a function call to look in scopes above by including a number of `'` instructions inside the `()`. For example:
```
Current cell value: 15
(--+) calls the fn at position 15 of current tape
('--+) calls the fn at position 15 of the parent's tape
(''--+) calls the fn at position 15 of the parent's parent's tape
(@--+) calls the fn at position 15 of the global/main tape
```

Therefore, from within a function, we can call functions from the caller's scope. Of course, if the function is a single non-nested function and being called from the global tape, then the `'` and `@` decorators have the same effect. But if this function is called from within another function without a function defined at the given address, it will cause errors, and so the global `@` decorator can be useful.

If at any point a function is called with an invalid address, or a `'` decorator is included where there is no parent scope, errors will be thrown. (Current error handling in the interpreter is not particularly good, however)

#### Other changes in BF++
Because of the way various things about functions are implemented, including address-based calling, there are some design changes where BF++ is not a 'true' Brainfuck interpreter.

* Cells hold 16-bit unsigned values (rather than the canonical single byte), because they need to be able to address the entire tape. Range is 0-65535, wrapping.
* The tape length is dynamic, beginning (with default options) at a length of 100 and growing by 1.5x each time the pointer is incremented beyond the end. A max length is specified, defaulted to the canonical 30,000. This limit can be changed but cannot be higher than 65,535 cells.
* Cells defined as functions cannot have the `+` and `-` operators run on them, and attempting to do so throws an error. This effectively means that functions cannot be destroyed at run-time, only replaced by other functions (this behaviour might need to be changed)
* As a result of 'typed cells', where each cell could be a function or a value, the interpreter is quite slow with a large overhead

#### Examples
Some very basic examples (mainly handling ASCII text) are included in the `tests` directory.

I haven't yet implemented any particularly complex algorithms because there's very little application for this, but the main aim of the function additions is to be able to reuse common algorithms , like a binary coded decimal function which might be called to output several numbers during the program.

#### I/O and 'raw mode'
The ``hello world`` of Brainfuck seems to be the `cat` program because of how simply it can be implemented:
```
,[.,]
```
For the ease of this example working as intended, the BF++ interpreter sends `EOF` as `0` regardless of how it is defined in C. All other characters are sent directly as they are received by `getchar()`, cast to `unsigned char`.

However, with a simple command line Brainfuck interpreter we notice that the input is (typically) line-buffered, so the `,` instruction prompts the user for a whole line but only takes the first character.

To fix this, we enter a raw mode, without line-buffering and probably (at least for `cat`) also without echoing the input. On POSIX this is fairly well defined and can be enabled with simple calls to the standard `<termios.h>` header, but on Windows there is no simple idea of raw mode.

In this repository the `rawmode.c` and `rawmode.h` implementation makes simple calls to the Windows API to change the terminal mode to a non-line buffered and non-echoing setting via the basic `enter_raw_mode()` and `exit_raw_mode()` functions.

To use this on Linux or Mac, it would be possible to redefine these functions with `<termios.h>` calls.

#### Why?
Well ... why not?

There are many [adaptations and extensions](https://esolangs.org/wiki/Category:Brainfuck_derivatives) to Brainfuck, including some that implement functions as well. The aim of this was to add function support to the language in a way that could be theoretically useful but stays true to Brainfuck's style.

All code compiles with GCC with default options, and in fact no warnings with `-Wall`.

----

*Philip Kimber, 2021*
