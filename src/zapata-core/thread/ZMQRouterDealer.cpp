/*
The MIT License (MIT)(

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
#include <zapata/thread/ZMQRouterDealer.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>

muzzley::ZMQRouterDealer::ZMQRouterDealer(string _host, uint _port) : __context(4), __zmq_router(__context, ZMQ_ROUTER), __zmq_dealer(__context, ZMQ_DEALER) {
	string _bind("tcp://");
	_bind.insert(_bind.length(), _host);
	_bind.insert(_bind.length(), ":");
	_bind.insert(_bind.length(), to_string(_port));
	this->__zmq_router.connect(_bind.data());

	string _bind("inproc://workers");
	this->__zmq_dealer.connect(_bind.data());

}

muzzley::ZMQRouterDealer::~ZMQRouterDealer() {
}

void muzzley::ZMQRouterDealer::loop() {
	zmq::proxy(this->__zmq_router, this->__zmq_dealer, NULL);
}
