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

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int _argc, char* _argv[]) {
    int _max_threads = 1000;
    int _n_elements = 1000;
    zpt::lf::queue<std::string*> _list{_max_threads, _n_elements};
    std::vector<std::thread> _threads;
    std::atomic<int> _count{0};

    for (int _i = 0; _i != _max_threads; ++_i) {
        _threads.emplace_back(
            [_n_elements, &_list, &_count](int _n_thread) {
                if (_n_thread % 2 == 0)
                    for (int _k = 0; _k != _n_elements; ++_k)
                        _list.push(new string(std::to_string(_n_thread * _n_elements + _k)));
                else {
                    for (int _k = 0, _n_tries = 0; _k != _n_elements && (_n_tries != _n_elements); ++_n_tries)
                        try {
                            std::string* _value = _list.pop();
                            if (_value != nullptr) {
                                delete _value;
                                ++_k;
                                _count.fetch_add(1);
                            } else
                                std::this_thread::sleep_for(
                                    std::chrono::duration<int, std::milli>{10});
                        } catch (std::out_of_range&) {
                        }
                }
            },
            _i);
    }

    for (int _i = 0; _i != _max_threads; ++_i)
        _threads[_i].join();

    std::cout << "Processed " << _count << " elements" << std::endl << std::flush;
    std::cout << _list << std::endl << std::flush;

    return 0;
}
