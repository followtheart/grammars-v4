# LALR(1) Parser Generator and Lexical Analyzer

A complete LALR(1) parser generator and lexical analyzer implementation in C++ for the grammars-v4 project.

## Features

- **Lexical Analyzer**: Tokenizes input text based on configurable token specifications using regex patterns
- **Grammar Parser**: Reads and parses context-free grammar specifications
- **LALR(1) Table Generation**: Generates LALR(1) parsing tables from grammar specifications
- **LR(0) Automaton Construction**: Builds LR(0) item sets and state transitions
- **FIRST/FOLLOW Set Computation**: Computes FIRST and FOLLOW sets for grammar analysis
- **Conflict Detection**: Detects and reports shift/reduce and reduce/reduce conflicts
- **Parse Tree Construction**: Builds parse trees from successful parses

## Components

### Core Components
- `token.h/.cpp` - Token definitions and utilities
- `lexer.h/.cpp` - Regex-based lexical analyzer
- `symbol.h/.cpp` - Grammar symbol management and symbol table
- `grammar.h/.cpp` - Context-free grammar with FIRST/FOLLOW computation
- `lr_items.h/.cpp` - LR(0)/LR(1) items and automaton construction
- `parse_table.h/.cpp` - LALR(1) parsing table generation
- `parser.h/.cpp` - LALR(1) parser engine (work in progress)

### Utilities
- `lalr1_tool` - Command-line tool for demonstrating parser generation
- `test_basic` - Tests for core components
- `test_safe` - Safe tests for LALR(1) table generation
- `examples/` - Example grammars and test cases

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Command-line Tool
```bash
# Run demonstration with built-in calculator grammar
./lalr1_tool --demo

# Show help
./lalr1_tool --help
```

### Programmatic Usage
```cpp
#include "grammar.h"
#include "parse_table.h"

// Create grammar
Grammar grammar;
auto E = grammar.symbol_table().get_nonterminal("E");
auto T = grammar.symbol_table().get_nonterminal("T");
auto plus = grammar.symbol_table().get_terminal("+", TokenType::PLUS);
auto num = grammar.symbol_table().get_terminal("num", TokenType::NUMBER);

// Add productions: E -> E + T | T, T -> num
grammar.add_production(E, {E, plus, T});
grammar.add_production(E, {T});
grammar.add_production(T, {num});

grammar.set_start_symbol(E);
grammar.augment();

// Generate LALR(1) table
LALR1Generator generator(grammar);
auto table = generator.generate_table();

// Check for conflicts
if (!table->has_conflicts()) {
    std::cout << "Grammar is LALR(1)!\n";
    table->print_table();
}
```

## Examples

### Simple Calculator Grammar
```
E -> E + T | T
T -> num

Tokens:
num: [0-9]+
+: \+
```

This generates a conflict-free LALR(1) parser for simple addition expressions.

### Generated LALR(1) Table
```
 State | ACTION       $       +     num | GOTO       E       T
------------------------------------------------------------
     0 |           err     err      s3 |           1       2
     1 |           acc      s4     err |
     2 |            r2      r2     err |
     3 |            r3      r3     err |
     4 |           err     err      s3 |                   5
     5 |            r1      r1     err |
```

- `s3` = shift and go to state 3
- `r1` = reduce by production 1
- `acc` = accept input
- `err` = error

## Testing

Run the test suite:
```bash
# Test core components
./test_basic

# Test LALR(1) table generation (safe)
./test_safe

# Comprehensive test (includes parser - may have issues)
./test_lalr1
```

## Current Status

✅ **Working Components:**
- Lexical analysis with regex-based tokenization
- Grammar representation and symbol management
- FIRST and FOLLOW set computation
- LR(0) automaton construction
- LALR(1) parsing table generation
- Conflict detection and reporting

⚠️ **Known Issues:**
- Parser engine has segmentation fault issues (work in progress)
- Limited grammar format support (programmatic only)
- No error recovery in parser

## Architecture

The parser generator follows the classic compiler construction approach:

1. **Lexical Analysis**: Input text → Tokens
2. **Grammar Analysis**: Context-free grammar → FIRST/FOLLOW sets
3. **Automaton Construction**: Grammar → LR(0) states and transitions
4. **Table Generation**: LR(0) automaton → LALR(1) parsing table
5. **Parsing**: Tokens + Table → Parse tree

## Grammar Properties

The generator can handle any context-free grammar that is LALR(1), including:
- Left-recursive grammars
- Ambiguous grammars (with conflicts reported)
- Epsilon productions
- Complex operator precedence

It reports conflicts when grammars are not LALR(1):
- Shift/reduce conflicts
- Reduce/reduce conflicts

## License

BSD License - Same as the parent grammars-v4 project.