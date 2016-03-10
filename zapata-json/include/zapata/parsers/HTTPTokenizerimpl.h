/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

// Generated by Bisonc++ V4.04.01 on Sun, 23 Mar 2014 13:54:15 +0000

    // Include this file in the sources of the class HTTPTokenizer.
#pragma once

// $insert class.h
#include <zapata/parsers/HTTPTokenizer.h>
#include <zapata/exceptions/SyntaxErrorException.h>

// $insert namespace-open
namespace zapata
{

inline void HTTPTokenizer::error(char const *msg)
{
    //std::cerr << msg << '\n';
   throw zapata::SyntaxErrorException(string(msg));

}

// $insert lex
inline int HTTPTokenizer::lex()
{
    return d_scanner.lex();
}

inline void HTTPTokenizer::print()
{
    print__();           // displays tokens if --print was specified
}

inline void HTTPTokenizer::exceptionHandler__(std::exception const &exc)
{
    throw;              // re-implement to handle exceptions thrown by actions
}

// $insert namespace-close
}

    // Add here includes that are only required for the compilation
    // of HTTPTokenizer's sources.


// $insert namespace-use
    // UN-comment the next using-declaration if you want to use
    // symbols from the namespace zapata without specifying zapata::
//using namespace zapata;

    // UN-comment the next using-declaration if you want to use
    // int HTTPTokenizer's sources symbols from the namespace std without
    // specifying std::

//using namespace std;
