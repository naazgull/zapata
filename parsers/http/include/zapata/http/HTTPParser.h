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

#pragma once

#include <zapata/http/HTTPObj.h>
#include <zapata/http/HTTPTokenizer.h>

namespace zpt {

/**
 * \brief HTTP parser class, wrapper for the bisonc++
 * (<http://bisoncpp.sourceforge.net/>) and
 * flexc++
 * (<http://flexcpp.sourceforge.net/>) generated parsers.
 *
 * The grammar files can be found at
 * <https://github.com/zapata/zapata-client-cpp/blob/master/src/parsers/HTTP.b>
 * and
 * <https://github.com/zapata/zapata-client-cpp/blob/master/src/parsers/HTTP.f>.
 * This class should no be used directly but through the zpt::HTTPReq or
 * zpt::HTTPRep **parse**
 * method or ** << **
 * operator, for your convenience.
 */
class HTTPParser : public HTTPTokenizer {
  public:
    /**
     * \brief Creates a new HTTPRep instance, pointing to a *null* object.
     *
     * @param _in  the std::istream where the textual representation of the HTTP
     * message is stream
     * for parsing
     * @param _out the std::ostream to where the transformed HTTP message may be
     * outputed. Just for
     * reference,
     * *this* parser will not output a transform byte buffer, instead it will
     * instantiate a
     * zpt::HTTOObj class
     * derivative.
     */
    HTTPParser(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    /**
     * \brief Destroys the current HTTPParser instance, freeing all allocated
     * memory.
     */
    virtual ~HTTPParser();

    /**
     * \brief Access method for passing in the zpt::zpt::http::request object to be populated
     * during parsing
     *
     * @param _root the zpt::http::request object to be populated during parsing
     */
    void switchRoots(zpt::http::basic_request& _root);
    /**
     * \brief Access method for passing in the zpt::http::reply object to be populated
     * during parsing
     *
     * @param _root the zpt::http::reply object to be populated during parsing
     */
    void switchRoots(zpt::http::basic_reply& _root);
    /**
     * \brief Write-access method for switching both input and output streams
     *
     * @param _in  the input stream to be used for parsing, from now on
     * @param _out the output stream to be used for transformation output, from
     * now on
     */
    void switchStreams(std::istream& _in = std::cin, std::ostream& _out = std::cout);
};
} // namespace zpt
