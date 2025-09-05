grammar Calculator;

// Parser rules
expr: expr '+' term | term;
term: term '*' factor | factor;
factor: NUM | '(' expr ')';

// Lexer rules  
NUM: [0-9]+;
WS: [ \t\r\n]+ -> skip;
