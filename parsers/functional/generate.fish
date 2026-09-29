#
# Regenerate the checked-in bison/re2c parser artifacts for the Functional parser.
#
# Produces (committed to the source tree):
#   include/zapata/functional/FunctionalParser.bison.h   bison header  (from src/Functional.y)
#   src/FunctionalParser.bison.cpp                       bison parser  (from src/Functional.y)
#   src/FunctionalLexer.re2c.cpp                         re2c lexer    (from src/Functional.re)
#
# Requires: bison (3.8.x) and re2c (4.x) on PATH.
# Run from anywhere:  fish parsers/functional/generate.fish

set -l here (dirname (status filename))

bison -Wall -Werror=conflicts-sr -Werror=conflicts-rr \
      --defines="$here/include/zapata/functional/FunctionalParser.bison.h" \
      -o "$here/src/FunctionalParser.bison.cpp" \
      "$here/src/Functional.y"
or exit 1

re2c -o "$here/src/FunctionalLexer.re2c.cpp" "$here/src/Functional.re"
or exit 1

# bison emits the parser impl's self-include as the bare basename
# (#include "FunctionalParser.bison.h"); retarget it at the checked-in header.
sed -i.bak 's|#include "FunctionalParser.bison.h"|#include <zapata/functional/FunctionalParser.bison.h>|' \
    "$here/src/FunctionalParser.bison.cpp"
rm -f "$here/src/FunctionalParser.bison.cpp.bak"

printf 'Generated Functional parser artifacts:\n'
printf '  %s\n' "$here/include/zapata/functional/FunctionalParser.bison.h" \
                   "$here/src/FunctionalParser.bison.cpp" \
                   "$here/src/FunctionalLexer.re2c.cpp"
