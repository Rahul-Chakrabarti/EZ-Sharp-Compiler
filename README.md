# EZ-Sharp-Compiler
In this project we designed and implemented a compiler for a custom programming language named "EZ-Sharp" using the C language. The compiler performs lexical, syntactic and semantic analysis and finished with generate suitable 3TAC code, similar to Assembly code.
Through this project, we learned how programming languages are intepreted, compiled and converted into executable source code.
The work was drafted and completed individually, and put together using both of our renditions of the compiler at the end, for improved optimality, correctness and robustness of the compiler.

## EZ-Sharp Features

EZ-Sharp is a simplified version of a general purpose programming language. 
The grammar ensures the user can formulate for loops and if else statements, define functions and perform basic arithmetic and numeric comparisons and operations.

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

We are both more experienced with Python but perferred C for the following reasons:

1. Performance: C is a low-level language that produces highly optimized machine code, we wanted to choose a language that compiled closer to assembly for efficency, and eliminate unnecessary abstractions away from core line-to-line assembly comparison.
2. Minimal Runtime Overhead: C does not require a runtime environment like the interpreted Python, ensuring that the compiled code can run without additional dependencies.
3. System-Level Access: C allows a intimate and direct interaction with system components, such as file systems and process management, which is useful for implementing compiler optimizations and debugging tools.
4. Fine-Grained Memory Control: C provides direct memory management, which gave ease for implementing data structures like symbol tables and abstract syntax trees (ASTs) efficiently.
5. Standing on the shoulders of Giants: Most traditional compilers, including GCC and Clang, were historically written in C or C++.
6. Lightweight and Predictable Execution: Unlike Python, C does not have garbage collection or dynamic typing, ensuring predictable performance and a more "from scratch" project.


