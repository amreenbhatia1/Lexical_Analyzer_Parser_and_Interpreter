# Lexical_Analyzer_Parser_and_Interpreter

A compiler design project that implements a Lexical Analyzer, CFG-based Parser, and Interpreter to tokenize, parse, and execute input code. Focused on syntax analysis, semantic interpretation, and hands-on compiler construction in C++.

## Overview

The interpreter targets **SADAL**, a small Ada-like procedural language. Source code is tokenized by the lexical analyzer, parsed with a recursive-descent parser that follows the language's context-free grammar, and executed directly as it is parsed.

## Features

- **Recursive-descent parser** with one-token lookahead (push-back) that mirrors the CFG rules
- **Declarations** of `integer`, `float`, `character`, `string`, and `boolean` variables, including multiple identifiers per declaration and optional initialization
- **Statements**: assignment (`:=`), `Put` / `PutLine` output, `Get` input, and `if` / `elsif` / `else` / `end if`
- **Expressions** with relational (`=`, `/=`, `<`, `>`), additive (`+`, `-`, `&` concatenation), multiplicative (`*`, `/`, `mod`), exponentiation (`**`), and unary (`+`, `-`, `not`) operators
- **String indexing and slicing**, e.g. `str(2)` and `str(1..4)`
- **Semantic checks**: undeclared or redefined variables, uninitialized use, illegal type assignments, illegal operand types, out-of-range indices, and division by zero
- **Error reporting** with line numbers

## Grammar (excerpt)

```
Prog       ::= PROCEDURE ProcName IS ProcBody
ProcBody   ::= DeclPart BEGIN StmtList END ProcName ;
DeclPart   ::= DeclStmt { DeclStmt }
DeclStmt   ::= IDENT { , IDENT } : Type [ := Expr ] ;
StmtList   ::= Stmt { Stmt }
Stmt       ::= AssignStmt | PrintStmts | GetStmt | IfStmt
Expr       ::= Relation
Relation   ::= SimpleExpr [ ( = | /= | < | > ) SimpleExpr ]
SimpleExpr ::= STerm { ( + | - | & ) STerm }
STerm      ::= Term { ( * | / | MOD ) Term }
Factor     ::= [ NOT | + | - ] Primary [ ** [ + | - ] Primary ]
Primary    ::= Name [ ( Range ) ] | ICONST | FCONST | SCONST | BCONST | CCONST | ( Expr )
```

## Project Structure

| File | Description |
| --- | --- |
| `Interpreter.cpp` | Parser and interpreter implementation |

The interpreter depends on the following companion files, which provide the lexer, token definitions, and value type:

- `lex.h` / `lex.cpp` - token types, `LexItem`, and `getNextToken()`
- `parserInterp.h` - parser/interpreter function declarations
- `val.h` / `val.cpp` - the `Value` class and its operators
- a driver program with `main()` that opens the source file and calls `Prog()`

## Build and Run

With all source files in the same directory:

```bash
g++ -std=c++17 -o interpreter *.cpp
./interpreter program.txt
```
