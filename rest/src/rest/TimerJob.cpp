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

#include <zapata/rest/TimerJob.h>
#include <zapata/log/log.h>

zapata::TimerJob::TimerJob() : Job() {
	size_t _max = 64;
	this->__events = new epoll_event_t[ _max ];
	this->__epoll_fd = epoll_create1 (0);

	(* this)->loop([ this, _max ] (Job& _job) -> void {	
		for (; true; ) {
			int _rv = epoll_wait(this->__epoll_fd, this->__events, _max, -1);
			if (_rv > 0) {
				for (int _idx = 0; _idx != _rv; _idx++) {
					zapata::timer_data_t * _ptr = (zapata::timer_data_t *) this->__events[_idx].data.ptr;
					if ((this->__events[_idx].events & EPOLLERR) || (this->__events[_idx].events & EPOLLHUP) || (!(this->__events[_idx].events & EPOLLIN))) {
						if (_ptr != nullptr) {
							close(_ptr->__fd);
							delete (_ptr);
							this->__events[_idx].data.ptr = nullptr;
						}
					}
					else {
						uint64_t _expirations = 0;
						if (read(((zapata::timer_data_t *) this->__events[_idx].data.ptr)->__fd, & _expirations, sizeof(uint64_t)) > 0) {}

						if (_ptr != nullptr) {
							if (!_ptr->__callback(* this, _ptr->__data)) {
								close(_ptr->__fd);
								delete _ptr;
								this->__events[_idx].data.ptr = nullptr;
							}
						}
						else {
							close(_ptr->__fd);
						}
					}
				}
			}
		}
	});
}

zapata::TimerJob::~TimerJob() {
	delete [] this->__events;
}

void zapata::TimerJob::assign(long _tick_interval, zapata::JSONObj& _data, zapata::TimerJobLoopCallback _callback) {
	pthread_mutex_lock((* this)->__mtx);

	int _t_fd = timerfd_create(CLOCK_MONOTONIC, 0);
	epoll_event_t _event;
	_event.events = EPOLLIN | EPOLLPRI;
	_event.data.ptr = new timer_data_t();
	((zapata::timer_data_t *) _event.data.ptr)->__fd = _t_fd;
	((zapata::timer_data_t *) _event.data.ptr)->__data = _data;
	((zapata::timer_data_t *) _event.data.ptr)->__callback = _callback;
	epoll_ctl (this->__epoll_fd, EPOLL_CTL_ADD, _t_fd, & _event);
	::itimerspec _new;
	::itimerspec _old;
	bzero(& _new, sizeof(_new));
	bzero(& _old, sizeof(_old));

	_new.it_value.tv_sec = (_tick_interval / 1000);
	_new.it_value.tv_nsec = ((_tick_interval % 1000) * 1000000);
	_new.it_interval.tv_sec = (_tick_interval / 1000);
	_new.it_interval.tv_nsec = ((_tick_interval % 1000) * 1000000);
	timerfd_settime(_t_fd, 0, & _new, & _old);

	pthread_mutex_unlock((* this)->__mtx);
}
