/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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

// Generated by Flexc++ V1.08.00 on Sun, 29 Jun 2014 10:16:29 +0100

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

// $insert class_ih
#include <zapata/parsers/JSONLexerimpl.h>

// $insert namespace-open
namespace zapata
{

    // s_ranges__: use (unsigned) characters as index to obtain
    //           that character's range-number.
    //           The range for EOF is defined in a constant in the
    //           class header file
size_t const JSONLexerBase::s_ranges__[] =
{
     0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
     5, 5, 5, 5, 5, 5, 5, 6, 7, 8, 9, 9, 9, 9,10,11,11,11,11,12,13,14,15,16,16,
    16,16,16,16,16,16,16,16,17,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,
    18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,19,20,21,22,22,22,23,24,24,
    25,26,27,28,28,29,30,30,31,32,33,34,34,34,35,36,37,38,39,39,39,39,39,40,41,
    42,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,
    43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,
    43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,
    43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,
    43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,
    43,43,43,43,43,44,
};

    // s_dfa__ contains the rows of *all* DFAs ordered by start state.
    // The enum class StartCondition__ is defined in the baseclass header
    // INITIAL is always 0.
    // Each entry defines the row to transit to if the column's
    // character range was sensed. Row numbers are relative to the
    // used DFA and d_dfaBase__ is set to the first row of the subset to use.
    // The row's final two values are begin and end indices in
    // s_rfc__[] (rule, flags and count), defining the state's rule details
int const JSONLexerBase::s_dfa__[][48] =
{
    // INITIAL
    {-1, 1, 1,-1, 1,-1, 1,-1, 2,-1, 3,-1, 4, 5,-1,-1, 5, 6,-1, 7,
         -1, 8,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,10,-1,-1,-1,11,12,-1,
         13,-1,14,-1,-1,-1,   0, 0},  // 0
    {-1, 1, 1,-1, 1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   0, 1},  // 1
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   1, 2},  // 2
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   2, 3},  // 3
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   3, 4},  // 4
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   4, 5},  // 5
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   5, 6},  // 6
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   6, 7},  // 7
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   7, 8},  // 8
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   8, 8},  // 9
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,16,-1,
         -1,-1,-1,-1,-1,-1,   8, 8},  // 10
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,17,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   8, 8},  // 11
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,18,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   8, 8},  // 12
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   8, 9},  // 13
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,   9,10},  // 14
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,19,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 15
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,20,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 16
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,21,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 17
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 18
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,23,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 19
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,24,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 20
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,25,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 21
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 22
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,27,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,10},  // 23
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  10,11},  // 24
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  11,12},  // 25
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,28,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  12,12},  // 26
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  12,13},  // 27
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,29,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  13,13},  // 28
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,30,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  13,13},  // 29
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,31,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  13,13},  // 30
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,32,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  13,13},  // 31
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  13,14},  // 32
    // number
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1, 0,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  14,15},  // 0
    // escaped
    {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1,
          1, 1, 1, 1,-1,-1,  15,15},  // 0
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  15,16},  // 1
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  16,17},  // 2
    // string_single
    {-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1,-1,-1,  17,17},  // 0
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  17,18},  // 1
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  18,19},  // 2
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  19,20},  // 3
    // string
    {-1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1,-1,-1,  20,20},  // 0
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  20,21},  // 1
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  21,22},  // 2
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  22,23},  // 3
    // unicode
    {-1, 1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1,-1,  23,23},  // 0
    {-1, 2,-1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2,-1,  23,23},  // 1
    {-1, 3,-1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3, 3,-1,  23,23},  // 2
    {-1, 4,-1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
          4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
          4, 4, 4, 4, 4,-1,  23,23},  // 3
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         -1,-1,-1,-1,-1,-1,  23,24},  // 4
};

    // The first value is the rule index
    // The second value is the FLAG: see the scannerbase.h file
    // 1: Final     4: Count        11: Final/BOL,Inc     
    // 2: Inc.      5: Final,Count  13: Final/BOL,Count
    // 3: Final,Inc 9: Final/BOL    
    // The third value is the LOP count value (valid for Count flags)
size_t const JSONLexerBase::s_rfc__[][3] =
{
//     R  F  C
     { 0, 1, 0},  // 0
     {12, 1, 0},  // 1
     {13, 1, 0},  // 2
     {10, 1, 0},  // 3
     {11, 1, 0},  // 4
     { 9, 1, 0},  // 5
     { 7, 1, 0},  // 6
     { 8, 1, 0},  // 7
     { 5, 1, 0},  // 8
     { 6, 1, 0},  // 9
     { 3, 1, 0},  // 10
     { 1, 1, 0},  // 11
     { 2, 1, 0},  // 12
     { 4, 1, 0},  // 13
     {14, 1, 0},  // 14
     {22, 1, 0},  // 15
     {21, 1, 0},  // 16
     {20, 1, 0},  // 17
     {18, 1, 0},  // 18
     {19, 1, 0},  // 19
     {17, 1, 0},  // 20
     {15, 1, 0},  // 21
     {16, 1, 0},  // 22
     {23, 1, 0},  // 23
};

int const (*JSONLexerBase::s_dfaBase__[])[48] =
{
    s_dfa__ + 0,
    s_dfa__ + 33,
    s_dfa__ + 34,
    s_dfa__ + 37,
    s_dfa__ + 41,
    s_dfa__ + 45,
};

size_t JSONLexerBase::s_istreamNr = 0;

// $insert inputImplementation
JSONLexerBase::Input::Input()
:
    d_in(0),
    d_lineNr(1)
{}

JSONLexerBase::Input::Input(std::istream *iStream, size_t lineNr)
:
    d_in(iStream),
    d_lineNr(lineNr)
{}

size_t JSONLexerBase::Input::get()
{
    switch (size_t ch = next())         // get the next input char
    {
        case '\n':
            ++d_lineNr;
        // FALLING THROUGH

        default:
        return ch;
    }
}

size_t JSONLexerBase::Input::next()
{
    size_t ch;

    if (d_deque.empty())                    // deque empty: next char fm d_in
    {
        if (d_in == 0)
            return AT_EOF;
        ch = d_in->get();
        return *d_in ? ch : static_cast<size_t>(AT_EOF);
    }

    ch = d_deque.front();
    d_deque.pop_front();

    return ch;
}

void JSONLexerBase::Input::reRead(size_t ch)
{
    if (ch < 0x100)
    {
        if (ch == '\n')
            --d_lineNr;
        d_deque.push_front(ch);
    }
}

void JSONLexerBase::Input::reRead(std::string const &str, size_t fm)
{
    for (size_t idx = str.size(); idx-- > fm; )
        reRead(str[idx]);
}

JSONLexerBase::JSONLexerBase(std::istream &in, std::ostream &out)
:
    d_filename("-"),
    d_startCondition(StartCondition__::INITIAL),
    d_state(0),
    d_out(new std::ostream(out.rdbuf())),
    d_sawEOF(false),
    d_atBOL(true),
    d_tailCount(24, std::numeric_limits<size_t>::max()),
// $insert interactiveInit
    d_in(0),
    d_input(new std::istream(in.rdbuf())),
    d_dfaBase__(s_dfa__)
{}

void JSONLexerBase::switchStream__(std::istream &in, size_t lineNr)
{
    d_input.close();
    d_state = 0;
    d_input = Input(new std::istream(in.rdbuf()), lineNr);
    d_sawEOF = false;
    d_atBOL = true;
}


JSONLexerBase::JSONLexerBase(std::string const &infilename, std::string const &outfilename)
:
    d_filename(infilename),
    d_startCondition(StartCondition__::INITIAL),
    d_state(0),
    d_out(outfilename == "-"    ? new std::ostream(std::cout.rdbuf()) :
          outfilename == ""     ? new std::ostream(std::cerr.rdbuf()) :
                                  new std::ofstream(outfilename)),
    d_sawEOF(false),
    d_atBOL(true),
    d_tailCount(24, std::numeric_limits<size_t>::max()),
    d_input(new std::ifstream(infilename)),
    d_dfaBase__(s_dfa__)
{}

void JSONLexerBase::switchStreams(std::istream &in, std::ostream &out)
{
    switchStream__(in, 1);
    switchOstream(out);
}


void JSONLexerBase::switchOstream(std::ostream &out)
{
    *d_out << std::flush;
    d_out.reset(new std::ostream(out.rdbuf()));
}

// $insert debugFunctions
void JSONLexerBase::setDebug(bool onOff)
{}

bool JSONLexerBase::debug() const
{
    return false;
}

void JSONLexerBase::redo(size_t nChars)
{
    size_t from = nChars >= length() ? 0 : length() - nChars;
    d_input.reRead(d_matched, from);
    d_matched.resize(from);
}

void JSONLexerBase::switchOstream(std::string const &outfilename)
{
    *d_out << std::flush;
    d_out.reset(
            outfilename == "-"    ? new std::ostream(std::cout.rdbuf()) :
            outfilename == ""     ? new std::ostream(std::cerr.rdbuf()) :
                                    new std::ofstream(outfilename));
}


void JSONLexerBase::switchIstream(std::string const &infilename)
{
    d_input.close();
    d_filename = infilename;
    d_input = Input(new std::ifstream(infilename));
    d_sawEOF = false;
    d_atBOL = true;
}

void JSONLexerBase::switchStreams(std::string const &infilename,
                           std::string const &outfilename)
{
    switchOstream(outfilename);
    switchIstream(infilename);
}

void JSONLexerBase::pushStream(std::istream  &istr)
{
    std::istream *streamPtr = new std::istream(istr.rdbuf());
    p_pushStream("(istream)", streamPtr);
}

void JSONLexerBase::pushStream(std::string const &name)
{
    std::istream *streamPtr = new std::ifstream(name);
    if (!*streamPtr)
    {
        delete streamPtr;
        throw std::runtime_error("Cannot read " + name);
    }
    p_pushStream(name, streamPtr);
}


void JSONLexerBase::p_pushStream(std::string const &name, std::istream *streamPtr)
{
    if (d_streamStack.size() == s_maxSizeofStreamStack__)
    {
        delete streamPtr;
        throw std::length_error("Max stream stack size exceeded");
    }

    d_streamStack.push_back(StreamStruct{d_filename, d_input});
    d_filename = name;
    d_input = Input(streamPtr);
    d_sawEOF = false;
    d_atBOL = true;
}


bool JSONLexerBase::popStream()
{
    d_input.close();

    if (d_streamStack.empty())
        return false;

    StreamStruct &top = d_streamStack.back();

    d_input =   top.pushedInput;
    d_filename = top.pushedName;
    d_streamStack.pop_back();
    d_sawEOF = false;

    return true;
}

JSONLexerBase::ActionType__ JSONLexerBase::actionType__(size_t range)
{
    d_nextState = d_dfaBase__[d_state][range];

    if (d_nextState != -1)                  // transition is possible
        return ActionType__::CONTINUE;

    if (atFinalState())                     // FINAL state reached
        return ActionType__::MATCH;

    if (d_matched.size())
        return ActionType__::ECHO_FIRST;    // no match, echo the 1st char

    return range != s_rangeOfEOF__ ? 
                ActionType__::ECHO_CH 
            : 
                ActionType__::RETURN;
}

void JSONLexerBase::accept(size_t nChars)          // old name: less
{
    if (nChars < d_matched.size())
    {
        d_input.reRead(d_matched, nChars);
        d_matched.resize(nChars);
    }
}

  // The size of d_matched is determined:
  //    The current state is a known final state (as determined by 
  // inspectRFCs__(), just prior to calling matched__). 
  //    The contents of d_matched are reduced to d_final's size or (if set)
  // to the LOP-rule's tail size.
void JSONLexerBase::determineMatchedSize(FinData const &final)
{
    size_t length = final.matchLen;
    if (final.tailCount != std::numeric_limits<size_t>::max())
        length -= final.tailCount;

    d_input.reRead(d_matched, length);      // reread the tail section
    d_matched.resize(length);               // return what's left
}

  // At this point a rule has been matched.  The next character is not part of
  // the matched rule and is sent back to the input.  The final match length
  // is determined, the index of the matched rule is determined, and then
  // d_atBOL is updated. Finally the rule index is returned.
size_t JSONLexerBase::matched__(size_t ch)
{
    d_input.reRead(ch);

    if (!d_atBOL)
        d_final.atBOL.rule = std::numeric_limits<size_t>::max();

    FinData &final = d_final.atBOL.rule == std::numeric_limits<size_t>::max() ? 
                            d_final.notAtBOL
                        :
                            d_final.atBOL;

    determineMatchedSize(final);

    d_atBOL = *d_matched.rbegin() == '\n';


    return final.rule;
}

size_t JSONLexerBase::getRange__(int ch)       // using int to prevent casts
{
    if (ch != AT_EOF)
        d_sawEOF = false;

    return ch == AT_EOF ? static_cast<size_t>(s_rangeOfEOF__) : s_ranges__[ch];
}

  // At this point d_nextState contains the next state and continuation is
  // possible. The just read char. is appended to d_match, and LOP counts
  // are updated.
void JSONLexerBase::continue__(int ch)
{
    d_state = d_nextState;

    if (ch != AT_EOF)
        d_matched += ch;
}

void JSONLexerBase::echoCh__(size_t ch)
{
    *d_out << static_cast<char>(ch);
    d_atBOL = ch == '\n';
}


   // At this point there is no continuation. The last character is
   // pushed back into the input stream as well as all but the first char. in
   // the buffer. The first char. in the buffer is echoed to stderr. 
   // If there isn't any 1st char yet then the current char doesn't fit any
   // rules and that char is then echoed
void JSONLexerBase::echoFirst__(size_t ch)
{
    d_input.reRead(ch);
    d_input.reRead(d_matched, 1);
    echoCh__(d_matched[0]);
}

    // Inspect all s_rfc__ elements associated with the current state
    //  If the s_rfc__ element has its COUNT flag set then set the 
    // d_tailCount[rule] value to the element's tailCount value, if it has its 
    // INCREMENT flag set then increment d_tailCount[rule]
    //  If neither was set set the d_tailCount[rule] to numeric_limits<size_t>::max()
    // 
    // If the s_rfc__ element has its FINAL flag set then store the rule number
    // in d_final.second. If it has its FINAL + BOL flags set then store the
    // rule number in d_final.first
void JSONLexerBase::inspectRFCs__()
{
    for 
    (
        size_t begin = d_dfaBase__[d_state][s_finacIdx__], 
                 end = d_dfaBase__[d_state][s_finacIdx__ + 1];
            begin != end;
                ++begin
    )
    {
        size_t const *rfc = s_rfc__[begin];
        size_t flag = rfc[FLAGS];
        size_t rule = rfc[RULE];

        if (flag & INCREMENT)
            ++d_tailCount[rule];
        else 
            d_tailCount[rule] = (flag & COUNT) ? rfc[ACCCOUNT] : std::numeric_limits<size_t>::max();

        if (flag & FINAL)
        {
            FinData &final = (flag & BOL) ? d_final.atBOL : d_final.notAtBOL;
            final = FinData { rule, d_matched.size(), d_tailCount[rule] };
        }
    }
}

void JSONLexerBase::reset__()
{
    d_final = Final { {std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max() }, 
                      {std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max() } };
    d_state = 0;
    d_return = true;

    if (!d_more)
        d_matched.clear();

    d_more = false;
}

int JSONLexer::executeAction__(size_t ruleIdx)
try
{
    switch (ruleIdx)
    {
        // $insert actions
        case 1:
        {
            return 258;

        }
        break;
        case 2:
        {
            return 258;

        }
        break;
        case 3:
        {
            return 261;

        }
        break;
        case 4:
        {
            return 261;

        }
        break;
        case 5:
        {
            return 262;

        }
        break;
        case 6:
        {
            return 263;

        }
        break;
        case 7:
        {
            return 264;

        }
        break;
        case 8:
        {
            return 265;

        }
        break;
        case 9:
        {
            return 267;

        }
        break;
        case 10:
        {
            return 266;

        }
        break;
        case 11:
        {
            {
    more();
    begin(StartCondition__::number);
}

        }
        break;
        case 12:
        {
            {
    begin(StartCondition__::string);
}

        }
        break;
        case 13:
        {
            {
    begin(StartCondition__::string_single);
}

        }
        break;
        case 14:
        {
            {
    begin(StartCondition__::INITIAL);
    if (matched().find(".") != std::string::npos) {
        return 260;
    }
    else {
        return 259;
    }
}

        }
        break;
        case 15:
        {
            {
    
    std::string _out(matched());
    _out.erase(_out.length() - 1, 1);
    setMatched(_out);
    begin(StartCondition__::INITIAL);
    return 257;
}

        }
        break;
        case 16:
        {
            {
    std::string _out(matched());
    _out.erase(_out.length() - 1, 1);
    setMatched(_out);
    more();
    begin(StartCondition__::escaped);
}

        }
        break;
        case 17:
        {
            {
    more();
}

        }
        break;
        case 18:
        {
            {
    
    std::string _out(matched());
    _out.erase(_out.length() - 1, 1);
    setMatched(_out);
    begin(StartCondition__::INITIAL);
    return 257;
}

        }
        break;
        case 19:
        {
            {
    std::string _out(matched());
    _out.erase(_out.length() - 1, 1);
    setMatched(_out);
    more();
    begin(StartCondition__::escaped);
}

        }
        break;
        case 20:
        {
            {
    more();
}

        }
        break;
        case 21:
        {
            {
    std::string _out(matched());
    _out.erase(_out.length() - 1, 1);
    setMatched(_out);
    more();
    begin(StartCondition__::unicode);
}

        }
        break;
        case 22:
        {
            {
    more();
    begin(StartCondition__::string);
}

        }
        break;
        case 23:
        {
            {
    std::string _out(matched());
    
    std::stringstream ss;
    ss << _out[_out.length() - 4] << _out[_out.length() - 3] << _out[_out.length() - 2] << _out[_out.length() - 1];
    int c;
    ss >> std::hex >> c;
    
    wchar_t w = (wchar_t) c;
    std::string dest("");
    
    if (w <= 0x7f) {
        dest.insert(dest.begin(), w);
    }
    else if (w <= 0x7ff) {
        dest.insert(dest.end(), 0xc0| ((w >> 6) & 0x1f));
        dest.insert(dest.end(), 0x80| (w & 0x3f));
    }
    else if (w <= 0xffff) {
        dest.insert(dest.end(), 0xe0| ((w >> 12) & 0x0f));
        dest.insert(dest.end(), 0x80| ((w >> 6) & 0x3f));
        dest.insert(dest.end(), 0x80| (w & 0x3f));
    }
    else if (w <= 0x10ffff) {
        dest.insert(dest.end(), 0xf0| ((w >> 18) & 0x07));
        dest.insert(dest.end(), 0x80| ((w >> 12) & 0x3f));
        dest.insert(dest.end(), 0x80| ((w >> 6) & 0x3f));
        dest.insert(dest.end(), 0x80| (w & 0x3f));
    }
    else {
        dest.insert(dest.end(), '?');
    }
    
    _out.assign(_out.substr(0, _out.length() - 4));
    _out.insert(_out.length(), dest);
    setMatched(_out);
    more();
    
    begin(StartCondition__::string);
}

        }
        break;
    }
    noReturn__();
    return 0;
}
catch (Leave__ value)
{
    return static_cast<int>(value);
}

int JSONLexer::lex__()
{
    reset__();
    preCode();

    while (true)
    {
        size_t ch = get__();                // fetch next char
        size_t range = getRange__(ch);      // determine the range

        inspectRFCs__();                    // update d_tailCount values

        switch (actionType__(range))        // determine the action
        {
            case ActionType__::CONTINUE:
                continue__(ch);
            continue;

            case ActionType__::MATCH:
            {
                d_token__ = executeAction__(matched__(ch));
                if (return__())
                {
                    print();
                    postCode(PostEnum__::RETURN);
                    return d_token__;
                }
                break;
            }

            case ActionType__::ECHO_FIRST:
                echoFirst__(ch);
            break;

            case ActionType__::ECHO_CH:
                echoCh__(ch);
            break;

            case ActionType__::RETURN:
                if (!popStream())
                {
                     postCode(PostEnum__::END);
                     return 0;
                }
                postCode(PostEnum__::POP);
             continue;
        } // switch

        postCode(PostEnum__::WIP);

        reset__();
        preCode();
    } // while
}

void JSONLexerBase::print__() const
{
}


// $insert namespace-close
}
