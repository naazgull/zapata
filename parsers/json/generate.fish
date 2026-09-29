#
# Regenerate the checked-in bison/re2c parser artifacts for the JSON parser.
#
# Produces (committed to the source tree):
#   include/zapata/json/JSONParser.bison.h   bison header  (from src/JSON.y)
#   src/JSONParser.bison.cpp                 bison parser  (from src/JSON.y)
#   src/JSONLexer.re2c.cpp                   re2c lexer    (from src/JSON.re)
#
# Requires: bison (3.8.x) and re2c (4.x) on PATH.
# Run from anywhere:  fish parsers/json/generate.fish

set -l here (dirname (status filename))

bison -Wall -Werror=conflicts-sr -Werror=conflicts-rr \
      --defines="$here/include/zapata/json/JSONParser.bison.h" \
      -o "$here/src/JSONParser.bison.cpp" \
      "$here/src/JSON.y"
or exit 1

re2c -o "$here/src/JSONLexer.re2c.cpp" "$here/src/JSON.re"
or exit 1

# bison emits the parser impl's self-include as the bare basename
# (#include "JSONParser.bison.h"); retarget it at the checked-in header.
sed -i.bak 's|#include "JSONParser.bison.h"|#include <zapata/json/JSONParser.bison.h>|' \
    "$here/src/JSONParser.bison.cpp"
rm -f "$here/src/JSONParser.bison.cpp.bak"

printf 'Generated JSON parser artifacts:\n'
printf '  %s\n' "$here/include/zapata/json/JSONParser.bison.h" \
                   "$here/src/JSONParser.bison.cpp" \
                   "$here/src/JSONLexer.re2c.cpp"
