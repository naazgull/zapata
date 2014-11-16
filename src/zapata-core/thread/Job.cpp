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

#include <zapata/thread/Job.h>
#include <fstream>

zapata::Job::Job() : shared_ptr<zapata::JobRef>(new JobRef()) {
}

zapata::Job::Job(Job& _rhs) : shared_ptr<zapata::JobRef>(_rhs) {
}

zapata::Job::Job(JobRef* _target) : shared_ptr<zapata::JobRef>(_target) {
}

zapata::Job::Job(JobLoopCallback _callback) : shared_ptr<zapata::JobRef>(new JobRef(_callback)) {
}

zapata::Job::~Job(){
}

zapata::JobRef::JobRef() {
	this->__mtx = new pthread_mutex_t();
	this->__thr = new pthread_t();
	pthread_mutexattr_init(&this->__attr);
	pthread_mutex_init(this->__mtx, &this->__attr);
}

zapata::JobRef::JobRef(JobLoopCallback _callback) {
	this->__mtx = new pthread_mutex_t();
	this->__thr = new pthread_t();
	pthread_mutexattr_init(&this->__attr);
	pthread_mutex_init(this->__mtx, &this->__attr);

	this->__callback = _callback;
}

zapata::JobRef::JobRef(JobRef& _rhs) {
	this->__mtx = _rhs.__mtx;
	this->__thr = _rhs.__thr;
	this->__callback = _rhs.__callback;
}

zapata::JobRef::~JobRef(){
}

void* zapata::JobRef::start(void* _thread) {
	JobRef* _running = static_cast<JobRef*>(_thread);
	Job _self(_running);
	_self->__callback(_self);
	_self->exit();
	return nullptr;
}

void zapata::JobRef::start() {
	JobRef* _new = new JobRef(* this);
	pthread_create(_new->__thr, 0, &JobRef::start, _new);
}

void zapata::JobRef::exit() {
	pthread_mutexattr_destroy(&this->__attr);
	pthread_mutex_destroy(this->__mtx);
	delete this->__mtx;
	delete this->__thr;
	pthread_exit(nullptr);
}

void zapata::JobRef::loop(JobLoopCallback _callback) {
	this->__callback = _callback;
}
