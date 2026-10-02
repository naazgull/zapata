#
# Regenerate the checked-in bison/re2c parser artifacts for the HTTP parser.
#
# Produces (committed to the source tree):
#   include/zapata/http/HTTPParser.bison.h   bison header  (from src/HTTP.y)
#   src/HTTPParser.bison.cpp                 bison parser  (from src/HTTP.y)
#   src/HTTPLexer.re2c.cpp                   re2c lexer    (from src/HTTP.re)
#
# Requires: bison (3.8.x) and re2c (4.x) on PATH.
# Run from anywhere:  fish parsers/http/generate.fish

set -l here (dirname (status filename))

bison -Wall -Werror=conflicts-sr -Werror=conflicts-rr \
      --defines="$here/include/zapata/http/HTTPParser.bison.h" \
      -o "$here/src/HTTPParser.bison.cpp" \
      "$here/src/HTTP.y"
or exit 1

re2c -o "$here/src/HTTPLexer.re2c.cpp" "$here/src/HTTP.re"
or exit 1

# bison emits the parser impl's self-include as the bare basename
# (#include "HTTPParser.bison.h"); retarget it at the checked-in header.
sed -i.bak 's|#include "HTTPParser.bison.h"|#include <zapata/http/HTTPParser.bison.h>|' \
    "$here/src/HTTPParser.bison.cpp"
rm -f "$here/src/HTTPParser.bison.cpp.bak"

printf 'Generated HTTP parser artifacts:\n'
printf '  %s\n' "$here/include/zapata/http/HTTPParser.bison.h" \
                   "$here/src/HTTPParser.bison.cpp" \
                   "$here/src/HTTPLexer.re2c.cpp"
