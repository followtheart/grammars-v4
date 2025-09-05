grammar SimpleJSON;

json: value;

value: STRING 
     | NUMBER 
     | object 
     | array 
     | 'true' 
     | 'false' 
     | 'null';

object: '{' '}'
      | '{' members '}';

members: pair
       | members ',' pair;

pair: STRING ':' value;

array: '[' ']'
     | '[' elements ']';

elements: value
        | elements ',' value;

STRING: '"' (~["])* '"';
NUMBER: '-'? ('0' | [1-9][0-9]*) ('.' [0-9]+)?;

WS: [ \t\r\n]+ -> skip;
