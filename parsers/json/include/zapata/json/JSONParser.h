/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file JSONParser.h
 * @brief JSON parser implementation for stream-based parsing.
 *
 * Provides a lexer/tokenizer-based JSON parser that reads from input streams
 * and produces `zpt::json` objects. This is the low-level parsing interface;
 * most users should use `zpt::json::load_from()` instead.
 *
 * @see zpt::json::load_from()
 * @see zpt::JSONTokenizer
 */

#pragma once

#include <mutex>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONTokenizer.h>

namespace zpt {

/**
 * @brief Stream-based JSON parser using lexer/tokenizer pattern.
 *
 * Extends JSONTokenizer to provide a complete JSON parsing solution.
 * The parser reads from an input stream and produces structured JSON data.
 *
 * @par Example
 * @code
 * std::istringstream input(R"({"name": "John", "age": 30})");
 * zpt::json result;
 * zpt::JSONParser parser(input);
 * parser.switchRoots(result);
 * parser.parse();
 * // result now contains the parsed JSON
 * @endcode
 *
 * @note For most use cases, prefer `zpt::json::load_from()` which wraps
 *       this parser with a simpler interface.
 */
class JSONParser : public JSONTokenizer {
  public:
    /**
     * @brief Constructs a parser with input and output streams.
     * @param _in Input stream to read JSON from (default: stdin).
     * @param _out Output stream for error messages (default: stdout).
     */
    JSONParser(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~JSONParser();

    /**
     * @brief Sets the root JSON object to populate during parsing.
     * @param _root Reference to the JSON object to fill with parsed data.
     */
    void switchRoots(zpt::json& _root);

    /**
     * @brief Changes the input and output streams.
     * @param _in New input stream.
     * @param _out New output stream.
     */
    void switchStreams(std::istream& _in = std::cin, std::ostream& _out = std::cout);
};
} // namespace zpt
