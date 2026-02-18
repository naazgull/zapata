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
 * @file json.h
 * @brief Aggregate header for the zapata-parser-json module.
 *
 * Include this single header to access all JSON functionality:
 * - zpt::json - Dynamic JSON value type
 * - zpt::JSONObj - JSON object type
 * - zpt::JSONArr - JSON array type
 * - JSON parsing from strings and streams
 * - JSON serialization
 * - Path-based access (get_path, set_path)
 * - Lambda functions in JSON
 * - Regular expressions in JSON
 *
 * @par Example Usage
 * @code
 * #include <zapata/json.h>
 *
 * // Create JSON objects
 * zpt::json obj = { "name", "John", "age", 30 };
 * zpt::json arr = { zpt::array, 1, 2, 3 };
 *
 * // Access values
 * std::string name = obj["name"];
 * int age = obj["age"];
 *
 * // Parse JSON
 * zpt::json parsed;
 * parsed.load_from("{\"key\": \"value\"}");
 *
 * // Serialize
 * std::cout << obj << std::endl;
 * @endcode
 *
 * @see zpt::json
 * @see zpt::JSONObj
 * @see zpt::JSONArr
 */

#pragma once

#include <zapata/base/expect.h>
#include <zapata/exceptions/InterruptedException.h>
#include <zapata/exceptions/NoAttributeNameException.h>
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/json/JSONClass.h>
#include <zapata/json/JSONParser.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/html.h>
#include <zapata/text/manip.h>
