#include <thread/Job.h>

zapata::Job::Job(string _key_file_path) : __idx(-1), __max_idx(-1), __sem(-1), __skey(_key_file_path) {
	this->__mtx = new pthread_mutex_t();
	this->__thr = new pthread_t();
	pthread_mutexattr_init(&this->__attr);
	pthread_mutex_init(this->__mtx, &this->__attr);
}

zapata::Job::~Job(){
	pthread_mutexattr_destroy(&this->__attr);
	pthread_mutex_destroy(this->__mtx);
	delete this->__mtx;
	delete this->__thr;
	pthread_exit(NULL);
}

void* zapata::Job::start(void* thread) {
	Job* running = static_cast<Job*>(thread);
	running->run();
	return NULL;
}

void zapata::Job::start() {
	key_t key = ftok(this->__skey.data(), this->__idx);
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

int zapata::Job::semid() {
	return this->__sem;
}

void zapata::Job::idx(size_t _idx) {
	this->__idx = _idx;
}

void zapata::Job::max(size_t _max) {
	this->__max_idx = _max;
}
