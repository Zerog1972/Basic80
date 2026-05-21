# Basic80

A simple interpreter for the classic BASIC language.

## Features

### Variables and Data Types
The language enforces strict typing based on variable names:
- **Numeric Variables** (double precision):
  - The name must **not** end with `$`.
  - Examples: `X`, `COUNT`, `PI`, `A1`.
  - Default value: `0.0`.
- **String Variables**:
  - The name **must** end with `$`.
  - Examples: `NAME$`, `TEXT$`, `A$`.
  - Default value: `""` (empty string).
  - Maximum string length is technically limited by memory; operations are handled dynamically.

### Supported BASIC commands:
- **PRINT** - Display text and variables
- **LET** - Assign a value to a variable
- **DIM** - Declare an array
- **INPUT** - Read a numeric value or a string (for `VAR$` variables) from the user
- **GOTO** - Jump to a numbered line
- **GOSUB** - Call a subroutine
- **RETURN** - Return from a subroutine
- **IF...THEN...ELSE** - Conditional execution
- **FOR...TO...STEP...NEXT** - Counter loops
- **DATA** - Declare constants in the program
- **READ** - Read DATA values into variables
- **RESTORE** - Reset the DATA read pointer (with or without a line number)
- **REM** - Comments
- **END** - End the program
- **HELP** - Display help on available commands

### Arithmetic operators:
- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Division
- `()` Parentheses for precedence

### Math functions:
- **SIN(x)** - Sine (x in radians)
- **COS(x)** - Cosine (x in radians)
- **TAN(x)** - Tangent (x in radians)
- **ATAN(x)** or **ATN(x)** - Arctangent (returns radians)
- **ASIN(x)** - Arcsine (returns radians)
- **ACOS(x)** - Arccosine (returns radians)
- **SINH(x)** - Hyperbolic sine
- **COSH(x)** - Hyperbolic cosine
- **TANH(x)** - Hyperbolic tangent
- **RAD(x)** - Convert degrees to radians
- **DEG(x)** - Convert radians to degrees
- **SQR(x)** - Square root
- **ABS(x)** - Absolute value
- **SGN(x)** - Sign of a number (-1, 0, or 1)
- **INT(x)** - Integer part (rounds down)
- **RND(x)** - Random integer between 0 and x-1
- **RND** - Random number between 0.0 and 1.0
- **LOG(x)** - Natural logarithm (ln)
- **EXP(x)** - Exponential (e^x)
- **POW(x, y)** - Power (x^y)
- **LOG10(x)** - Base-10 logarithm

### String functions:
These functions manipulate strings.
- **LEN(s$)** : Returns the length of string `s$`. (Returns a number)
- **ASC(s$)** : Returns the ASCII code of the first character of `s$`. (Returns a number)
- **CHR$(n)** : Returns the character corresponding to ASCII code `n`. (Returns a string)
- **MID$(s$, start, len)** : Returns a substring of `s$` starting at `start` (1-indexed) of length `len`. (Returns a string)
- **LEFT$(s$, n)** : Returns the first `n` characters of `s$`. (Returns a string)
- **RIGHT$(s$, n)** : Returns the last `n` characters of `s$`. (Returns a string)
- **STR$(x)** : Converts a number `x` to a string. (Returns a string)
- **VAL(s$)** : Converts a string `s$` to a number. (Returns a number)
- **SPACE$(n)** : Returns a string of `n` spaces. (Returns a string)
- **STRING$(n, c)** : Returns a string of `n` repetitions of character `c` (ASCII code or single-character string). (Returns a string)

Example:
```basic
LET A$ = "HELLO"
PRINT LEN(A$)       ' Outputs 5
PRINT LEFT$(A$, 3)  ' Outputs HEL
PRINT CHR$(65)      ' Outputs A
PRINT STR$(42)      ' Outputs 42
```
The `+` operator concatenates two strings.

### Comparison operators:
- `=` Equal
- `<` Less than
- `>` Greater than
- `<=` Less than or equal
- `>=` Greater than or equal
- `<>` Not equal

## Usage

### Compilation
```bash
clang -std=c89 -pedantic -Wall -g -Iinclude src/*.c -o basic80.exe
```

Or use the VS Code task: **C/C++: clang.exe build all files**

### Unit tests
Compile and run the tests:
```bash
clang -std=c89 -pedantic -Wall -g -Iinclude tests/tests.c src/interp.c src/lexer.c src/vars.c src/expr.c src/ctrlflow.c src/commands.c -o tests.exe
.\tests.exe
```

The unit tests include:
- **Lexer tests**: numbers, identifiers, keywords, operators, strings
- **Interpreter tests**: variables, lines, LET commands, expressions
- **IF...THEN...ELSE condition tests**:
  - Simple IF...THEN (true/false condition)
  - Full IF...THEN...ELSE
  - All comparison operators (<, >, <=, >=, =, <>)
  - IF...THEN GOTO
  - IF...THEN...ELSE with GOTO
- **DIM array tests**:
  - Declaration and assignment of simple elements (1D)
  - Filling arrays with a FOR loop
  - Reading array elements in expressions
  - Array calculations (sum of elements)
  - Using variables as indices
- **Multi-dimensional array tests**:
  - Simple 2D arrays (matrices)
  - 2D arrays with nested FOR loops
  - Identity matrices (with IF conditions)
  - Expressions in 2D array indices
  - Simple 3D arrays (cubes)
  - 3D arrays with triple FOR loops
- **Math function tests**:
  - Trigonometric functions (SIN, COS, TAN)
  - Math functions (SQR, ABS, INT)
  - Composite functions and complex expressions
- **GOSUB/RETURN tests**:
  - Simple GOSUB with a subroutine
  - GOSUB with calculations (reusing a subroutine)
  - Nested GOSUB (subroutines called from subroutines)
  - GOSUB inside a FOR loop
- **String tests**:
  - Assigning string literals
  - Copying strings between variables
  - Functions LEN(), ASC(), CHR$(), MID$(), LEFT$(), RIGHT$()
  - Functions STR$(), VAL(), SPACE$(), STRING$()
  - String concatenation with the + operator
  - Two-way conversions number ↔ string
- **FOR loop tests**:
  - Simple FOR loop (1 TO 5)
  - FOR loop with positive STEP (0 TO 10 STEP 2)
  - Descending FOR loop with negative STEP (10 TO 1 STEP -1)
  - FOR loops with edge cases (start = end, start > end)
  - Nested FOR loops
  - FOR loops with calculations (factorial, sum of squares)
  - FOR loops with variables as limits
- **New function tests**:
  - Math function ATN(x) - arctangent
  - Math function SGN(x) - sign of a number
  - String function STR$(x) - number to string
  - String function VAL(s) - string to number
  - String function SPACE$(n) - generate spaces
  - String function STRING$(n,c) - repeat a character
- **DATA/READ/RESTORE command tests**:
  - Reading numbers with DATA/READ
  - Reading strings with DATA/READ
  - Multiple consecutive DATA statements
  - RESTORE without parameter (rewind to beginning)
  - RESTORE with a line number
  - "Out of DATA" error detection

Total: **196 unit tests**

The code strictly conforms to the **C89/ANSI C** standard.

### Running
```bash
./basic80.exe
```

When you type `RUN`, the interpreter now displays the program execution time after it finishes. Short runs are shown in seconds, and longer runs are shown in minutes.

### Interactive mode

**Shell commands:**
- `LIST` - Display the program in memory
- `RUN` - Run the program
- `NEW` - Clear the program
- `SAVE "file.bas"` - Save the program to a file
- `LOAD "file.bas"` - Load a program from a file
- `HELP` - Display help on commands (use `HELP COMMAND` for more details)
- `EXIT` - Quit the interpreter

**Numbered lines:**
Add instructions to the program (e.g. `10 PRINT "Hello"`)

**Direct commands:**
Execute immediately (e.g. `PRINT "Hello"`)

## Examples

### Example 1: Simple program
```basic
10 PRINT "Hello, World!"
20 LET X = 42
30 PRINT "The answer is:", X
40 END
```

### Example 2: Calculations
```basic
10 LET A = 10
20 LET B = 5
30 LET C = A + B * 2
40 PRINT "Result:", C
50 END
```

### Example 3: Using INPUT
```basic
10 PRINT "Enter your age:"
20 INPUT AGE
30 LET YEARS = 100 - AGE
40 PRINT "You have approximately", YEARS, "years left"
50 END
```

### Example 4: Simple FOR loop
```basic
10 FOR I = 1 TO 5
20 PRINT "I =", I
30 NEXT I
40 END
```

### Example 5: FOR loop with STEP
```basic
10 FOR I = 0 TO 10 STEP 2
20 PRINT "I =", I
30 NEXT I
40 END
```

### Example 6: Descending FOR loop
```basic
10 FOR I = 10 TO 1 STEP -1
20 PRINT "Countdown:", I
30 NEXT I
40 PRINT "Liftoff!"
50 END
```

### Example 7: Calculation with FOR loop
```basic
10 LET TOTAL = 0
20 FOR I = 1 TO 10
30 LET TOTAL = TOTAL + I
40 NEXT I
50 PRINT "Sum from 1 to 10 =", TOTAL
60 END
```

### Example 8: IF...THEN condition
```basic
10 INPUT X
20 IF X > 0 THEN PRINT "Positive number"
30 END
```

### Example 9: IF...THEN...ELSE condition
```basic
10 PRINT "Enter your age:"
20 INPUT AGE
30 IF AGE >= 18 THEN PRINT "Adult" ELSE PRINT "Minor"
40 END
```

### Example 10: IF with GOTO
```basic
10 LET SCORE = 75
20 IF SCORE >= 60 THEN GOTO 50
30 PRINT "Fail"
40 GOTO 60
50 PRINT "Pass"
60 END
```

### Example 11: Simple arrays
```basic
10 DIM A(5)
20 LET A(0) = 10
30 LET A(1) = 20
40 LET A(2) = 30
50 PRINT "First element:", A(0)
60 PRINT "Second element:", A(1)
70 END
```

### Example 12: Arrays with a loop
```basic
10 DIM NUMBERS(10)
20 FOR I = 0 TO 10
30 LET NUMBERS(I) = I * I
40 NEXT I
50 PRINT "Squares from 0 to 10:"
60 FOR I = 0 TO 10
70 PRINT I, "squared =", NUMBERS(I)
80 NEXT I
90 END
```

### Example 13: Sum of array elements
```basic
10 DIM VALUES(5)
20 LET VALUES(0) = 10
30 LET VALUES(1) = 20
40 LET VALUES(2) = 30
50 LET VALUES(3) = 40
60 LET VALUES(4) = 50
70 LET TOTAL = 0
80 FOR I = 0 TO 4
90 LET TOTAL = TOTAL + VALUES(I)
100 NEXT I
110 PRINT "Total sum:", TOTAL
120 END
```

### Example 14: Multi-dimensional arrays - 3x3 matrix
```basic
10 REM 3x3 matrix
20 DIM M(3, 3)
30 REM Fill the matrix with loops
40 FOR I = 0 TO 2
50 FOR J = 0 TO 2
60 LET M(I, J) = I * 3 + J + 1
70 NEXT J
80 NEXT I
90 REM Display some elements
100 PRINT "M(0,0) =", M(0, 0)
110 PRINT "M(1,1) =", M(1, 1)
120 PRINT "M(2,2) =", M(2, 2)
130 END
```

### Example 15: Identity matrix
```basic
10 REM Create a 3x3 identity matrix
20 DIM ID(3, 3)
30 FOR I = 0 TO 2
40 FOR J = 0 TO 2
50 IF I = J THEN LET ID(I, J) = 1
60 IF I <> J THEN LET ID(I, J) = 0
70 NEXT J
80 NEXT I
90 PRINT "Diagonal:"
100 PRINT ID(0, 0), ID(1, 1), ID(2, 2)
110 END
```

### Example 16: 3D array (cube)
```basic
10 REM 3D array - 2x2x2 cube
20 DIM CUBE(2, 2, 2)
30 FOR I = 0 TO 1
40 FOR J = 0 TO 1
50 FOR K = 0 TO 1
60 LET CUBE(I, J, K) = I * 4 + J * 2 + K + 1
70 NEXT K
80 NEXT J
90 NEXT I
100 PRINT "CUBE(0,0,0) =", CUBE(0, 0, 0)
110 PRINT "CUBE(1,1,1) =", CUBE(1, 1, 1)
120 END
```

### Example 17: Math functions
```basic
10 LET PI = 3.14159
20 LET A = SIN(PI)
30 LET B = COS(0)
40 LET C = SQR(16)
50 LET D = ABS(-5)
60 LET E = INT(3.7)
70 PRINT "SIN(PI) =", A
80 PRINT "COS(0) =", B
90 PRINT "SQR(16) =", C
100 PRINT "ABS(-5) =", D
110 PRINT "INT(3.7) =", E
120 END
```

### Example 18: Trigonometric circle
```basic
10 LET PI = 3.14159
20 FOR ANGLE = 0 TO 360 STEP 45
30 LET R = ANGLE * PI / 180
40 LET X = COS(R)
50 LET Y = SIN(R)
60 PRINT "Angle", ANGLE, "-> X=", X, "Y=", Y
70 NEXT ANGLE
80 END
```

### Example 19: Simple subroutine
```basic
10 PRINT "Main program"
20 GOSUB 100
30 PRINT "Back in main"
40 END
100 REM Subroutine
110 PRINT "Inside subroutine"
120 RETURN
```

### Example 20: Calculation subroutine
```basic
10 LET N = 5
20 GOSUB 100
30 PRINT "Factorial of 5 =", FACT
40 END
100 REM Factorial calculation
110 LET FACT = 1
120 FOR I = 1 TO N
130 LET FACT = FACT * I
140 NEXT I
150 RETURN
```

### Example 21: Strings
```basic
10 LET NAME$ = "Alice"
20 LET OTHER$ = "Bob"
30 PRINT "Name:", NAME$
40 PRINT "Other:", OTHER$
50 LET L = LEN(NAME$)
60 PRINT "Name length:", L
70 LET C = ASC("A")
80 PRINT "ASCII code of A:", C
90 END
```

### Example 22: Copying strings
```basic
10 LET MESSAGE$ = "Hello World"
20 LET COPY$ = MESSAGE$
30 PRINT "Original:", MESSAGE$
40 PRINT "Copy:", COPY$
50 PRINT "Length:", LEN(COPY$)
60 END
```

### Example 23: Using GOTO
```basic
10 LET X = 1
20 PRINT "Count:", X
30 LET X = X + 1
40 IF X < 5 THEN GOTO 20
50 PRINT "Done!"
60 END
```

### Example 24: String concatenation
```basic
10 LET A$ = "Hello" + " " + "World"
20 LET B$ = LEFT$("PROG", 2) + RIGHT$("RAM", 2)
30 PRINT "Phrase:", A$
40 PRINT "Merged:", B$
50 END
```

### Example 25: Advanced math functions
```basic
10 LET E = EXP(1)
20 LET L = LOG(E)
30 LET P = POW(2, 8)
40 LET L10 = LOG10(1000)
50 PRINT "e =", E
60 PRINT "ln(e) =", L
70 PRINT "2^8 =", P
80 PRINT "log10(1000) =", L10
90 END
```

### Example 26: Hyperbolic functions
```basic
10 LET S = SINH(0)
20 LET C = COSH(0)
30 LET T = TANH(0)
40 LET A = ATAN(1)
50 PRINT "SINH(0) =", S
60 PRINT "COSH(0) =", C
70 PRINT "TANH(0) =", T
80 PRINT "ATAN(1) (approx PI/4) =", A
90 END
```

### Example 27: Degrees and radians
```basic
10 LET ANGLE = 90
20 REM Convert degrees to radians for SIN
30 LET S = SIN(RAD(ANGLE))
40 PRINT "SIN(90 degrees) =", S
50 REM Convert inverse result to degrees
60 LET A = ASIN(1)
70 LET D = DEG(A)
80 PRINT "ASIN(1) in degrees =", D
90 END
```

### Example 28: DATA, READ and RESTORE
```basic
10 DATA 100, 200, 300, 400, 500
20 READ A, B, C
30 PRINT "First numbers:", A, B, C
40 RESTORE
50 READ X, Y
60 PRINT "After RESTORE:", X, Y
70 END
```

### Example 29: DATA with strings
```basic
10 DATA "Alice", "Bob", "Charlie"
20 DATA 25, 30, 35
30 READ NAME1$, NAME2$, NAME3$
40 READ AGE1, AGE2, AGE3
50 PRINT NAME1$, "is", AGE1, "years old"
60 PRINT NAME2$, "is", AGE2, "years old"
70 PRINT NAME3$, "is", AGE3, "years old"
80 END
```

### Example 30: RESTORE with a line number
```basic
10 DATA 1, 2, 3
20 DATA 4, 5, 6
30 READ A, B
40 PRINT "From line 10:", A, B
50 RESTORE 20
60 READ X, Y
70 PRINT "From line 20:", X, Y
80 END
```

### Example 31: Using interactive help
```
> HELP
=== BASIC80 - Available commands ===

Basic commands:
  PRINT    - Display text or values
  LET      - Assign a value to a variable
  INPUT    - Read a value from the keyboard
  REM      - Add a comment
  ...

Type HELP [COMMAND] for more details.
Example: HELP PRINT

> HELP PRINT
=== PRINT ===

Syntax: PRINT [expression [,|; expression]...]

Description:
  Outputs one or more expressions to the screen.
  Expressions can be numeric or string.

Separators:
  , (comma)        : Space between expressions
  ; (semicolon)    : No space between expressions
  End of line      : Automatic newline

Examples:
  PRINT "Hello"           -> Outputs: Hello
  PRINT 42                -> Outputs: 42.00
  PRINT A, B              -> Outputs: 10.00 20.00
  PRINT "X="; X           -> Outputs: X=5.00

> HELP FOR
=== FOR / NEXT ===

Syntax: FOR variable = start TO end [STEP increment]
        ... statements ...
        NEXT [variable]

Description:
  Counter loop.
  STEP is optional (default 1).
```

### Example 32: Save and load a program
```
> 10 PRINT "Sample program"
> 20 FOR I = 1 TO 3
> 30 PRINT "Counter:", I
> 40 NEXT I
> 50 END
> LIST
10 PRINT "Sample program"
20 FOR I = 1 TO 3
30 PRINT "Counter:", I
40 NEXT I
50 END
> SAVE "myprog.bas"
Program saved to 'myprog.bas'.
> NEW
Program cleared.
> LOAD "myprog.bas"
Program loaded from 'myprog.bas'.
> RUN
Sample program
Counter: 1
Counter: 2
Counter: 3
```

## Architecture

The project is organised as follows:
- **src/** : Source files (`.c`)
- **include/** : Headers (`.h`)
- **tests/** : Unit tests and test data files
- **docs/** : Additional documentation

### Source files (src/):
- **main.c** - Main entry point and interactive loop
- **interp.c** - Interpreter core and orchestration
- **commands.c** - BASIC commands (PRINT, LET, DIM, INPUT, DATA, READ, RESTORE, HELP, CLS)
- **ctrlflow.c** - Control flow handling (IF/THEN/ELSE, FOR/NEXT, GOTO, GOSUB/RETURN)
- **expr.c** - Arithmetic and string expression evaluation
- **vars.c** - Variable and multi-dimensional array management
- **lexer.c** - Lexical analysis and tokenisation

### Components:
1. **Lexer** (`lexer.c`) - Tokenises source code (53 keywords, 73 token types)
2. **Variables** (`vars.c`) - Manages numeric variables, strings, and arrays up to 10 dimensions
3. **Expression** (`expr.c`) - Evaluation with operator precedence, math and string functions
4. **Control Flow** (`ctrlflow.c`) - Handles IF/THEN/ELSE, FOR/NEXT, GOTO, GOSUB/RETURN
5. **Commands** (`commands.c`) - Executes PRINT, LET, DIM, INPUT, DATA/READ/RESTORE, HELP, CLS
6. **Interpreter** (`interp.c`) - Orchestration, program storage, extension hook registry

### Extension hook system

The interpreter can be extended at runtime without modifying its source code.
Three types of hooks are available:

```c
/* Custom numeric function: PRINT DOUBLE(21) -> 42 */
double myDouble(Interpreter *interp, Token *tokens, int *pos) {
    double arg = evaluateExpression(interp, tokens, pos);
    return arg * 2.0;
}
registerCustomNumericFunction(interp, "DOUBLE", myDouble);

/* Custom string function: PRINT REVERSE$("Hello") -> olleH */
char* myReverse(Interpreter *interp, Token *tokens, int *pos) {
    /* ... */
}
registerCustomStringFunction(interp, "REVERSE$", myReverse);

/* Custom command: BEEP */
void myBeep(Interpreter *interp, Token *tokens) { printf("\a"); }
registerCustomCommand(interp, "BEEP", myBeep);
```

See `include/interp.h` and `extens/extens.c` for a full working example.

## Current limitations

- Filenames for `SAVE`/`LOAD` must conform to the DOS 8.3 format (max 8-character name, 3-character extension, no spaces)
- Only numeric arrays; string arrays (`DIM A$(10)`) are not supported
- No logical operators (`AND`, `OR`, `NOT`) in conditions
- No user-defined functions (`DEF FN`)
- `INPUT` accepts only one variable per statement
- Angles for trigonometric functions are in radians (use `RAD()`/`DEG()` for conversion)

## Possible extensions

- Logical operators (`AND`, `OR`, `NOT`) in `IF` conditions
- String arrays
- User-defined functions (`DEF FN`)
- `INPUT` with multiple comma-separated variables
