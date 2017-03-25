# The Smile Parser

The Smile parser is a complicated piece of machinery.  To help you understand it better, here is a rough roadmap of its overall design.

-----------------

## The Lexer

The Smile lexical analyzer (in the `lexer` subfolder) is exclusively responsible for turning plain characters into parse tokens and raw values, which, given the complexity of the language rules, is a nontrivial task.  The lexical analyzer is structured similarly to one produced by _lex_, in that each invocation of it returns the next token, but unlike a _lex_ analyzer, ours is hand-written.

The core lexer can be found in `lexer.c`, and its primary interface consists of these functions:

- `Lexer_Create()` constructs a new `Lexer` based on a given input string.
- `Lexer_Next()` returns the _kind_ of the next token in the input.  It stores related data about that token in a `Token` object contained within the `Lexer` object itself; for example, if it returns `TOKEN_INTEGER64`, the `Token` will have the actual number sitting in its `data.int64` property.
- `Lexer_Unget()` (in `lexer.h`) pushes the most-recently read token back onto the input stream, so that it will be returned again by the next call to `Lexer_Next()`.  It uses a proper stack, and you can unget up to 15 tokens before running out of space.
- `Lexer_Peek()` (also in `lexer.h`) is the equivalent of calling next/unget in sequence:  It retrieves the _kind_ (and potentially the value) of the next token without consuming it.

In addition, if the Lexer has been given access to a `SymbolTable` instance via a call to `Lexer_SetSymbolTable()`, calls to `Lexer_Next()` that result in a symbol will also resolve the symbol ID through the given symbol table, and will store the symbol's ID in the token's `data.symbol` field.

The Lexer goes to great effort to track the correct line and column number in the given source code.  That information is stored in an embedded form inside both it and in every token it returns.  To access the source coordinates of a token, you can call `Token_GetPosition()`, which will return a new object (on the heap) that contains the source coordinates.

When all tokens have been read from the input stream, calls to `Lexer_Next()` are still valid, but will always return `TOKEN_EOF`.

**Important note:**  The `Token` objects stored inside the `Lexer` itself are _ephemeral_.  Subsequent calls to `Lexer_Next()` (or any related function) will _overwrite_ them.  (Actually, they're maintained in a buffer, so they won't expire until 15 more tokens have been read, but they _will_ still expire.)  If you need to keep the data stored in a `Token` that comes out of the `Lexer`, you need to either copy the data yourself or `Token_Clone()` it.  (This also explains why `Token_GetPosition()` returns a copy of the position on the heap, because the data would otherwise be overwritten in a subsequent call.)

----------------------

## The Parser

The parser is responsible for transforming lexical tokens into S-expressions.  It is implemented as a recursive-descent parser, starting in `parsercore.c`, that repeatedly invokes `Lexer_Next()` (and friends) to consume tokens from the input stream.

### "Native" Syntax

Smile's "natural" grammar is formally defined as an LL(3) grammar in the file `/grammar.txt`.  It provides special precedence rules for operators that people generally agree on.  It does _not_ define meaning for those operators; it merely establishes precedence.  The precedence hierarchy, from lowest to highest, is as follows:

- STMT class:  `#syntax` _and_ `#macro`
- ASSIGN class:  `=` *and* `op=`
- OR class:  `x or y`
- AND class:  `x and y`
- NOT class: `not x`
- CMP class: `===`,`!==`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `~==`, `~!=`, `~<`, `~>`, `~<=`, `~>=`, `is`
- ADD class: `+` *and* `-` *(binary forms)*
- MUL class: `*` *and* `/` *(binary forms)*
- BINARY class:  `x anyname y`  *(handles all other binary  operators)*
- COLON class:  `x: y`
- RANGE class:  `x..y`
- PREFIX class:  `anyname x`  *(handles all unary operators except `not`)*
- NEW class:  `new base { ... }` *and* `new { ... }` and `{ ... }`
- CONS class:  `x ## y`
- DOT class:  `x . y`
- TERM class:
  - `(parentheses)`
  - ``quote`
  - `[call ...]`
  - `variable`
  - `"string"` and `'char'`
  - `#loanword` (*like* `#/regex/` *and* `#json`)
  - `|func| ...`
  - *numbers:* `123`

### Declaration Logic

In Smile, names have meaning based on context.  In particular, names have meaning based on how they are declared in the current scope.  In `x + y`, the parser only knows to treat `+` as an operator because it previously saw `x` and `y` being used or declared as variables.

The general rule, then, is that the parser must keep track of all implicit and explicit variable declarations in order to know how to recognize unary and binary operators.  Each scope contains a `ParseScope` object attached to it that contains a lookup table for the variables declared within it.  The following constructs may be used to "declare" that something is a variable:

- Explicit use of `var` or `const` or `auto`.  When you write `var x`, that tells the parser that `x` is definitely a variable and not available for use as a unary or binary operator.
- Function-call arguments.  When you write `|x| ...`, the parser knows that `x` is definitely a variable and not available for use as a unary or binary operator.
- Implicit declaration via assignment.  When you write `x = 5`, the parser knows that if `x` isn't a variable already, it should be.  It immediately adds a declaration of `x` to the current scope, and continues parsing.
- Explicit use of a `[$scope]` or `[$fn]` S-expression.  When you write `[$scope [x] ...]` or `[$fn [x] ...]`, the parser knows that `x` is definitely a variable and not available for use as a unary or binary operator.
- Implicit declaration via a `[$set]` expression.  When you write `[$set x 5]`, the parser follows the same rules as if you had written `x = 5`.

It is also important to note certain scenarios when variables are _not_ declared:

- Rvalues.  If you write `x = y`, that does not cause `y` to become declared.
- Operator-equals forms.  If you write `x += 5`, that does _not_ declare `x`, because for that operation to be meaningful, `x` must already have been declared somewhere before it.

### Custom Syntax Parsing

Because the language can be altered on the fly, a considerable amount of logic exists outside of the normal recursive-descent parser in the various `*syntax*.c` files for handling and applying `#syntax` declarations efficiently.  `#syntax` declarations support a full LL(1) grammar.  They are implemented as a lazily-constructed forest of copy-on-write trees (in `parsersyntaxtable.c`).  Each parsing scope can potentially have its own private grammar that is a derivative of an inherited grammar, so the custom-syntax parser goes out of its way to avoid building any more data structures than are necessary for a given scope, and tries hard to reuse existing data structures wherever possible.

At each level of the recursion in the parser, there may be a call to `applysyntax.c` to attempt to apply any syntax rules that may be appropriate for the current grammatical class.  Because `#syntax` uses LL(1) parsing, the only rule used to determine whether syntax is applied or "normal" grammar is applied is whether the next token matches the root of at least one syntax rule:  If it does, the parser begins attempting to greedily match the syntax rule(s).  The `Parser_ApplyCustomSyntax()` function can return one of these values to indicate the result of its parse:

- `NotMatchedAndNoTokensConsumed`.  No custom syntax rule matches this next token, so the "normal" grammar should continue to apply here.
- `PartialApplicationWithError`.  We got partway through parsing a custom syntax rule, but there was a syntax error midway through it:  For example, we got an `if` and its `EXPR`, but the subsequent `then` keyword was missing.  This cause the parser to engage its error recovery mode.
- `SuccessfullyParsed`. We got all the way through parsing a rule, performed all substitutions, and resulted in a fully-transformed S-expression.  The parser can skip any subsequent parsing for this syntax class.

### Raw Expression Parsing

In addition to the normal parsing logic, and to the custom syntax logic, there is special logic for parsing "raw expressions," i.e., expressions that have been `[$quote]`'d with a backtick: `

In raw-expression mode, the parser uses a simplified grammar, similar to that used by an ordinary Lisp parser.  The raw-expression parser recognizes lists and basic terms like symbols and numbers, and does not attempt to apply any syntax rules to anything it finds.

However, Smile raw expressions can be used like Lisp templates as well.  By using (parentheses) to embed normal syntax-evaluated expressions, you can turn an ordinary raw expression into a template.  The raw expression parser detects when (parentheses) (or {curly braces}) are used to embed normal expressions, and upon encountering them, will backtrack and transform the "simple" S-expression it has been constructing into a series of calls to `[$cons]` or `[$list]`, embedding the normally-parsed expressions in the middle of those as necessary.