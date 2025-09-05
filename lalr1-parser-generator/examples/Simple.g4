grammar Simple;

expr: NUM;

NUM: [0-9]+;
WS: [ \t\r\n]+ -> skip;
