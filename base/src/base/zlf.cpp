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

#include <signal.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <sys/types.h>
#include <sys/ipc.h>
#include <zapata/lf/queue.h>
#include <zapata/log/log.h>
#include <zapata/text/manip.h>
#include <zapata/exceptions/NoMoreElementsException.h>

constexpr int MAX_THREADS = 100;
constexpr int N_ELEMENTS = 10000;

// #define QUEUE_USE_STRING
// #define FIRST_PUSH_THEN_POP

std::atomic<int> _pushed{ 0 };
std::atomic<int> _poped{ 0 };
#ifdef QUEUE_USE_STRING
zpt::lf::queue<std::string*> _list{ MAX_THREADS, N_ELEMENTS };
#else
zpt::lf::queue<int> _list{ MAX_THREADS, N_ELEMENTS };
#endif

auto
pause(int _signal) -> void {
    std::cout << _list.to_string() << std::endl << std::flush;
    std::cout << "#pushed -> " << _pushed.load() << std::endl << std::flush;
    std::cout << "#poped -> " << _poped.load() << std::endl << std::flush;
}

auto
main(int _argc, char* _argv[]) -> int {
    std::vector<std::thread> _threads;

    std::signal(SIGINT, pause);

#ifdef FIRST_PUSH_THEN_POP
    for (int _i = 0; _i != MAX_THREADS / 2; ++_i) {
#else
    for (int _i = 0; _i != MAX_THREADS; ++_i) {
#endif
        _threads.emplace_back(
          [](int _n_thread) {
#ifdef FIRST_PUSH_THEN_POP
              if (_n_thread < MAX_THREADS / 2) {
#else
              if (_n_thread % 2 == 0) {
#endif
                  for (int _k = 0; _k != N_ELEMENTS; ++_k) {
#ifdef QUEUE_USE_STRING
                      _list.push(new std::string(std::to_string(_n_thread * N_ELEMENTS + _k)));
#else
                      _list.push(_n_thread * N_ELEMENTS + _k + 1);
#endif
                      ++_pushed;
                  }
              }
#ifndef FIRST_PUSH_THEN_POP
              else {
                  for (int _k = 0; _k != N_ELEMENTS;) {
                      try {
#ifdef QUEUE_USE_STRING
                          std::string* _value = _list.pop();
                          delete _value;
#else
                          int _value = _list.pop();
#endif
                          ++_k;
                          ++_poped;
                      }
                      catch (zpt::NoMoreElementsException& e) {
                          std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 10 });
                      }
                  }
              }
#endif
          },
          _i);
    }

#ifdef FIRST_PUSH_THEN_POP
    std::cout << "wainting for pushes to finish... " << std::flush;
    std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 5000 });
    std::cout << "done!" << std::endl << std::flush;
    for (int _i = MAX_THREADS / 2; _i != MAX_THREADS; ++_i) {
        _threads.emplace_back(
          [](int _n_thread) {
              for (int _k = 0; _k != N_ELEMENTS;) {
                  try {
#ifdef QUEUE_USE_STRING
                      std::string* _value = _list.pop();
                      delete _value;
#else
                      int _value = _list.pop();
#endif
                      ++_k;
                      ++_poped;
                  }
                  catch (zpt::NoMoreElementsException& e) {
                      std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 10 });
                  }
              }
          },
          _i);
    }
#endif

    for (int _i = 0; _i != MAX_THREADS; ++_i)
        _threads[_i].join();

    std::cout << "Processed " << _poped << " elements" << std::endl << std::flush;
    std::cout << _list.to_string() << std::endl << std::flush;

    return 0;
}
