#
# Regenerate the checked-in bison/re2c parser artifacts for the URI parser.
#
# Produces (committed to the source tree):
#   include/zapata/uri/URIParser.bison.h   bison header  (from src/URI.y)
#   src/URIParser.bison.cpp                bison parser  (from src/URI.y)
#   src/URILexer.re2c.cpp                  re2c lexer    (from src/URI.re)
#
# Requires: bison (3.8.x) and re2c (4.x) on PATH.
# Run from anywhere:  fish parsers/uri/generate.fish

set -l here (dirname (status filename))

bison -Wall -Werror=conflicts-sr -Werror=conflicts-rr \
      --defines="$here/include/zapata/uri/URIParser.bison.h" \
      -o "$here/src/URIParser.bison.cpp" \
      "$here/src/URI.y"
or exit 1

re2c -o "$here/src/URILexer.re2c.cpp" "$here/src/URI.re"
or exit 1

# bison emits the parser impl's self-include as the bare basename
# (#include "URIParser.bison.h"); retarget it at the checked-in header.
sed -i.bak 's|#include "URIParser.bison.h"|#include <zapata/uri/URIParser.bison.h>|' \
    "$here/src/URIParser.bison.cpp"
rm -f "$here/src/URIParser.bison.cpp.bak"

printf 'Generated URI parser artifacts:\n'
printf '  %s\n' "$here/include/zapata/uri/URIParser.bison.h" \
                  "$here/src/URIParser.bison.cpp" \
                  "$here/src/URILexer.re2c.cpp"
