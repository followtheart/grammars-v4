# LALR(1) Parser Generator and Lexical Analyzer

A complete LALR(1) parser generator and lexical analyzer implementation in C++ for the grammars-v4 project.

## Features

- **Lexical Analyzer**: Tokenizes input text based on configurable token specifications
- **Grammar Parser**: Reads and parses grammar specifications in a custom format
- **LALR(1) Table Generation**: Generates LALR(1) parsing tables from grammar specifications
- **LALR(1) Parser Engine**: Parses input using generated LALR(1) tables
- **Example Grammars**: Includes examples like calculator expressions

## Components

### Core Components
- `lexer.h/.cpp` - Lexical analyzer implementation
- `grammar.h/.cpp` - Grammar representation and parsing
- `lalr1_generator.h/.cpp` - LALR(1) table generation
- `parser.h/.cpp` - LALR(1) parser engine
- `token.h` - Token definitions and types

### Utilities
- `grammar_reader.h/.cpp` - Grammar specification file reader
- `parse_table.h/.cpp` - Parsing table representation
- `symbol.h/.cpp` - Grammar symbol management

### Examples
- `examples/` - Example grammars and test cases
- `tests/` - Unit tests and integration tests

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic Example
```cpp
#include "lalr1_parser.h"

// Create parser from grammar file
LALR1Parser parser("grammar.txt");

// Parse input
auto result = parser.parse("2 + 3 * 4");
```

### Grammar Format
The parser uses a simplified grammar specification format:
```
# Grammar rules (BNF-like syntax)
S -> E
E -> E + T | T
T -> T * F | F
F -> ( E ) | id | num

# Token specifications
id: [a-zA-Z][a-zA-Z0-9]*
num: [0-9]+
+: \+
*: \*
(: \(
): \)
```

## Testing

Run the test suite:
```bash
./build/test_lalr1
```

## Examples

See the `examples/` directory for complete examples including:
- Calculator expressions
- Simple programming language constructs
- Arithmetic expressions with precedence

## License

BSD License - Same as the parent grammars-v4 project.