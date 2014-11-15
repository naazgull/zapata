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

#include <zapata/thread/JobServer.h>

#include <zapata/exceptions/InterruptedException.h>

zapata::JobServer::JobServer(size_t _max) : __next(0), __max_idx(-1) {
	this->max(_max);
	this->__next = 0;
}

zapata::JobServer::~JobServer() {
}

void zapata::JobServer::start() {
	try {
		for (; true; ) {
			if (this->max() != 0 && this->__next == this->max()) {
				this->__next = 0;
			}

			this->wait();
			this->notify();

			this->__next++;
		}
	}
	catch (zapata::InterruptedException& e) {
		return;
	}
}

size_t zapata::JobServer::max() {
	return this->__max_idx;
}

void zapata::JobServer::max(size_t _max) {
	this->__max_idx = _max;
}

size_t zapata::JobServer::next() {
	return this->__next;
}

