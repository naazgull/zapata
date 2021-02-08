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

#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <chrono>

#include <zapata/uri.h>

int
main(int argc, char* argv[]) {
    try {
        std::cout << zpt::uri::parse("file:./users.json") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("file:./users.json")) << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("{(.*)}:/{(.*)}") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("{(.*)}:/{(.*)}")) << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("/home/pf/{tmp:/(.*)/}/a.txt") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("/home/pf/{tmp:/(.*)/}/a.txt"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("./pf/{tmp:/(.*)/}/a.txt") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("./pf/{tmp:/(.*)/}/a.txt")) << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("../pf/{tmp:/(.*)/}/a.txt") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("../pf/{tmp:/(.*)/}/a.txt")) << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("file+json:/home/pf/{tmp:/(.*)/}/a.j") << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("file+json:/home/pf/{tmp:/(.*)/}/a.j"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("file+json:./pf/{tmp:/(.*)/}/a.j") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("file+json:./pf/{tmp:/(.*)/}/a.j"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("file+json:../pf/{tmp:/(.*)/}/a.j") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("file+json:../pf/{tmp:/(.*)/}/a.j"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("file+json:/home/pf/{tmp:/(.*)/}/a.j#some_point_in_doc")
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(
                       zpt::uri::parse("file+json:/home/pf/{tmp:/(.*)/}/a.j#some_point_in_doc"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("tcp+json+ssl:/home/pf/{tmp:/(.*)/}/a.j#some_point_in_doc")
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(
                       zpt::uri::parse("tcp+json+ssl:/home/pf/{tmp:/(.*)/}/a.j#some_point_in_doc"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("tcp+json+ssl:/home/pf/../{tmp:/(.*)/}/a.j#some_point_in_doc")
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse(
                       "tcp+json+ssl:/home/pf/../{tmp:/(.*)/}/a.j#some_point_in_doc"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("{/http|ftp/}://api:8081/2.0/users/me?a=2&b=3&c=") << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(
                       zpt::uri::parse("{/http|ftp/}://api:8081/2.0/users/me?a=2&b=3&c="))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse(
                       "{/http|ftp/}://api:8081/2.0/users/me?a=2&b=3&c=#some_point_in_doc")
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse(
                       "{/http|ftp/}://api:8081/2.0/users/me?a=2&b=3&c=#some_point_in_doc"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("?a=2&b=3&c=1&d=lower(Strong)") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("?a=2&b=3&c=1&d=lower(Strong)"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("http://pf@na.zgul.me/api/2.0/users/me?a=2&b=3&c=")
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(
                       zpt::uri::parse("http://pf@na.zgul.me/api/2.0/users/me?a=2&b=3&c="))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse(
                       "http://pf@na.zgul.me/api/2.0/users/me?a=2&b=3&c=&d=ge(integer(0))")
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse(
                       "http://pf@na.zgul.me/api/2.0/users/me?a=2&b=3&c=&d=ge(integer(0))"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("ftp://pf@na.zgul.me/files/movies") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("ftp://pf@na.zgul.me/files/movies"))
                  << std::endl
                  << std::endl
                  << std::flush;
        std::cout << zpt::uri::parse("#some_point_in_doc") << std::endl << std::flush;
        std::cout << zpt::uri::to_string(zpt::uri::parse("#some_point_in_doc")) << std::endl
                  << std::endl
                  << std::flush;
    }
    catch (zpt::SyntaxErrorException const& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    return 0;
}
