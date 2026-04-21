# EZ-Sharp-Compiler
In this project, we designed and implemented a compiler for a custom programming language named "EZ-Sharp" using the C language. The compiler performs lexical, syntactic, and semantic analysis and generates suitable 3TAC code, similar to high-level Assembly code.
Throughout this project, we learned how programming languages are interpreted, compiled, and converted into executable source code.
The work was drafted and completed individually.  However, our work was combined using our separate renditions of the compiler at the end to improve its optimality, correctness, and robustness.

## EZ-Sharp Features

EZ-Sharp is a simplified version of a general-purpose programming language. 
The grammar ensures the user can formulate for loops and if-else statements, define functions, and perform basic arithmetic and numeric comparisons and operations.

The EZ-Sharp grammar is attached below, for reference.

```txt
<program> ::= <fdecls> <declarations> <statement_seq>.
<fdecls> ::= <fdec>; | <fdecls> <fdec>; | ε
<fdec> ::= def <type> <fname> ( <params> ) <declarations> <statement_seq> fed
<params> ::= <type> <var> | <type> <var> , <params> | ε
<fname> ::= <id>
<declarations> ::= <decl>; | <declarations> <decl>; | ε
<decl> := <type> <varlist>
<type> := int | double
<varlist> ::= <var>, <varlist> | <var>
<statement_seq> ::= <statement> | <statement>; <statement_seq>
<statement> ::= <var> = <expr> |
				if <bexpr> then <statement_seq> fi |
				if <bexpr> then <statement_seq> else <statement_seq> fi |
				while <bexpr> do <statement_seq> od |
				print <expr> |
				return <expr> | ε
<expr> ::= <expr> + <term> | <expr> - <term> | <term>
<term> ::= <term> * <factor> | <term> / <factor> | <term> % <factor> |
<factor>
<factor> ::= <var> | <number> | (<expr>) | <fname>(<exprseq>)
<exprseq> ::= <expr>, <exprseq> | <expr> | ε
<bexpr> ::= <bexpr> or <bterm> | <bterm>
<bterm> ::= <bterm> and <bfactor> | <bfactor>
<bfactor> ::= (<bexpr>) | not <bfactor> | (<expr> <comp> <expr>)
<comp> ::= < | > | == | <= | >= | <>
<var> ::= <id> | <id>[<expr>]
<letter> ::= a | b | c | ... | z
<digit> ::= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0
<id> ::= <letter> | <id><letter> | <id><digit>
<number> ::= <integer> | <double>
```

## Why choose C over Python?

We are both more experienced with Python, but preferred C for the following reasons:

1. Performance: C is a low-level language that produces highly optimized machine code. We wanted to choose a language that compiled closer to assembly for efficiency, to eliminate unnecessary abstractions and keep the core line-to-line assembly comparison.
2. Minimal Runtime Overhead: C does not require a runtime environment like the interpreted Python, ensuring that the compiled code can run without additional dependencies.
3. System-Level Access: C allows an intimate and direct interaction with system components, such as file systems and process management, which is useful for implementing compiler optimizations and debugging tools.
4. Fine-Grained Memory Control: C provides direct memory management, which makes it easier to implement data structures like symbol tables and abstract syntax trees (ASTs) efficiently.
5. Standing on the shoulders of Giants: Most traditional compilers, including GCC and Clang, were historically written in C or C++.
6. Lightweight and Predictable Execution: Unlike Python, C does not have garbage collection or dynamic typing, ensuring predictable performance and a more "from scratch" project.

### LL(1) Form Grammar

```txt
PROG → FNS DECLS STMTS .
FNS → FN; FNSC 
FNS → ε
FNSC → FN; FNSC 
FNSC → ε
FN → def TYPE FNAME ( PARAMS ) DECLS STMTS fed  
PARAMS → TYPE VAR PARAMSC 
PARAMS → ε
PARAMSC → , TYPE VAR PARAMSC  
PARAMSC → ε
FNAME → ID
DECLS → DECL; DECLSC 
DECLS → ε
DECLSC → DECL; DECLSC 
DECLSC → ε
DECL → TYPE VARS
TYPE → int 
TYPE → double
VARS → VAR VARSC
VARSC → , VARS 
VARSC → ε
STMTS → STMT STMTSC
STMTSC → ; STMTS 
STMTSC → ε
STMT → VAR = EXPR
STMT → if BEXPR then STMTS STMTC 
STMT →  while BEXPR do STMTS od 
STMT → print EXPR
STMT → return EXPR 
STMT →  ε
STMTC → fi 
STMTC → else STMTS fi
EXPR → TERM EXPRC
EXPRC → + TERM EXPRC 
EXPRC → - TERM EXPRC 
EXPRC → ε
TERM → FACTOR TERMC
TERMC → * FACTOR TERMC 
TERMC → / FACTOR TERMC 
TERMC → % FACTOR TERMC 
TERMC → ε
FACTOR→ ID FACTORC 
FACTOR→ NUMBER 
FACTOR→ ( EXPR )
FACTORC → VARC 
FACTORC → ( EXPRS )
EXPRS → EXPR EXPRSC 
EXPRS → ε
EXPRSC → , EXPRS 
EXPRSC → ε
BEXPR → BTERM BEXPRC
BEXPRC → or BTERM BEXPRC 
BEXPRC → ε
BTERM → BFACTOR BTERMC
BTERMC → and BFACTOR BTERMC  
BTERMC → ε
BFACTOR → not bfactor
BFACTOR → (expr comp expr)
COMP → < 
COMP → > 
COMP → == 
COMP → <= 
COMP → >= 
COMP → <>
VAR → ID VARC
VARC → [ EXPR ] 
VARC → ε
```

### First & Follow Sets 

|  | First | Follow |
| --- | --- | --- |
| PROG | { ".", ";", def, int, double, if, while, print, return, ID } | { $ } |
| FNS | { def, $\epsilon$ } | { ".", ";", int, double, if, while, print, return, ID } |
| FNSC | { def, $\epsilon$ ) | { ".", ";", int, double, if, while, print, return, ID } |
| FN | { def } | { ";" } |
| PARAMS | { int, double, $\epsilon$  ) | {”)” }  |
| PARAMSC | { ",",  $\epsilon$  } | { “)” } |
| FNAME | { ID  } | { “(”  } |
| DECLS | { int, double, $\epsilon$  } | { ".", ";", fed, if, while, print, return, ID } |
| DECLSC | { int, double, $\epsilon$ } | { ".", ";", fed, if, while, print, return, ID } |
| DECL | { int, double } | { ";" } |
| TYPE | { int, double } | { ID } |
| VARS | { ID } | { ";" } |
| VARSC | { ",",  $\epsilon$ } | { ";" } |
| STMTS | { ";", if, while, print, return, ID, $\epsilon$ } | { ".", fed, od, fi, else } |
| STMTSC | { ";", $\epsilon$ } | { ".", fed, od, fi, else } |
| STMT | { if, while, print, return, ID, $\epsilon$ } | { ".", ";", fed, od, fi, else } |
| STMTC | { fi, else } | { ".", ";", fed, od, fi, else } |
| EXPR | { “(”,  ID, NUMBER } | { ".", ";", “)”, fed, ",", od, fi, else,  <, >, ==, ≤, ≥, <>, “]”  } |
| EXPRC | { “+”, “-”,  $\epsilon$ } | { ".", ";", “)”, fed, ",", od, fi, else,  <, >, ==, ≤, ≥, <>, “]”  } |
| TERM | { “(”, ID, NUMBER } | { ".", ";", ), fed, ",", od, fi, else, +, -,  <, >, ==, ≤, ≥, <>, “]” } |
| TERMC | { “*”, “/”, “%”, $\epsilon$  } | { ".", ";", ), fed, ",", od, fi, else, +, -,  <, >, ==, ≤, ≥, <>, “]” } |
| FACTOR | { “(”, ID, NUMBER } | { ".", ";", ), fed, ",", od, fi, else, +, -, *, /, %, <, >, ==, ≤, ≥, <>, “]” } |
| EXPRS | { “(”, ID, NUMBER, $\epsilon$ } | { “)” } |
| EXPRSC | { "," , $\epsilon$  } | { “)” } |
| BEXPR | { “(”, not } | { then, do } |
| BEXPRC | { or, $\epsilon$  } | { then, do, or, and } |
| BTERM | { “(”, not } | { then, do, or, and } |
| BTERMC | { and,  $\epsilon$  } | { then, do, or, and } |
| BFACTOR | { “(”, not } | { then, do, or, and } |
| BFACTORC | { or, and,  $\epsilon$  } | { then, do, or, and } |
| COMP | { “<”, “>”, “==”, “≤”, “≥”, “<>” } | { “(”, ID, NUMBER } |
| VAR | { ID } | { “;”,  “)”,  “,”, “=”} |
| VARC | { “[”,  $\epsilon$  } | { ".", ";", “)”, fed, ",", =, od, fi, else, +, -, *, /, %, <, >, ==, ≤, ≥, <>, “]” } |

References:
Compilers: Principles, Techniques, and Tools, A. V. Aho, R. Sethi and J. D. Ullman, Addison-Wesley, 2nd ed., 2006
