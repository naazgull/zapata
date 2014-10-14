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
#include <zapata/parsers/json.h>

namespace zapata {
	pthread_key_t __configuration_key;
}

zapata::Job::Job(string _key_file_path) : __idx(-1), __max_idx(-1), __sem(-1), __skey(_key_file_path) {
	this->__mtx = new pthread_mutex_t();
	this->__thr = new pthread_t();
	pthread_mutexattr_init(&this->__attr);
	pthread_mutex_init(this->__mtx, &this->__attr);

	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	zapata::fromfile(_key_file, this->__configuration);
}

zapata::Job::~Job(){
	pthread_mutexattr_destroy(&this->__attr);
	pthread_mutex_destroy(this->__mtx);
	delete this->__mtx;
	delete this->__thr;
	pthread_exit(NULL);
}

void* zapata::Job::start(void* _thread) {
	Job* _running = static_cast<Job*>(_thread);
	pthread_setspecific(zapata::__configuration_key, &_running->__configuration);
	_running->run();
	return NULL;
}

void zapata::Job::start() {
	key_t key = ftok(this->__skey.data(), this->__max_idx);
	this->__sem = semget(key, this->__max_idx, IPC_CREAT | 0777);
	if (this->__sem == 0) {
	}
	pthread_create(this->__thr, 0, &Job::start, this);
}

void zapata::Job::assign() {
	struct sembuf ops[1] = { { (short unsigned int) this->__idx, 1 } };
	semop(this->__sem, ops, 1);
}

void zapata::Job::wait() {
	struct sembuf ops[1] = { { (short unsigned int) this->__idx, -1 } };
	semop(this->__sem, ops, 1);
}

void zapata::Job::wait(int seconds) {
	sleep(seconds);
}

size_t zapata::Job::idx() {
	return this->__idx;
}

size_t zapata::Job::max() {
	return this->__max_idx;
}

semid_t zapata::Job::semid() {
	return this->__sem;
}

zapata::JSONObj& zapata::Job::configuration() {
	return this->__configuration;
}

void zapata::Job::idx(size_t _idx) {
	this->__idx = _idx;
}

void zapata::Job::max(size_t _max) {
	this->__max_idx = _max;
}
