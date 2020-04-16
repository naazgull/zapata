/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
        // std::cout << zpt::pretty(zpt::uri::parse("{(.*)}:/{(.*)}")) << std::endl << std::flush;
        // std::cout << zpt::pretty(zpt::uri::parse("/home/pf/{tmp:/(.*)/}/a.txt")) << std::endl
        //           << std::flush;
        // std::cout << zpt::uri::parse("{/http|ftp/}://api/2.0/users/me?a=2&b=3&c=") << std::endl
        //           << std::flush;
        std::cout << zpt::uri::parse("?a=2&b=3&c=1&d=lower(Strong)") << std::endl << std::flush;
        // std::cout << zpt::uri::parse("http://pf@na.zgul.me/api/2.0/users/me?a=2&b=3&c=")
        //           << std::endl
        //           << std::flush;
        // std::cout << zpt::uri::parse("ftp://pf:pf@na.zgul.me/files/movies") << std::endl
        //           << std::flush;
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
