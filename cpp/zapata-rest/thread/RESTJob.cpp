#include <thread/RESTJob.h>

#include <zapata/net.h>
#include <zapata/http.h>
#include <exceptions/SyntaxErrorException.h>

zapata::RESTJob::RESTJob(string _key_file_path) : Job(_key_file_path), __cur_fd(-1) {
}

zapata::RESTJob::~RESTJob() {
}

void zapata::RESTJob::run() {
	for (; true; ) {
		this->wait();
		if (this->__cur_fd != -1) {
			socketstream _cs(this->__cur_fd);

			HTTPRep _rep;
			HTTPReq _req;
			for (; true; ) {
				try  {
					zapata::fromstream(_cs, _req);
					{
						ostringstream _text;
						_text << _req << flush;
						zapata::log(_text.str(), zapata::debug);
					}

					this->__pool->process(_req, _rep);

					{
						ostringstream _text;
						_text << _rep << flush;
						zapata::log(_text.str(), zapata::debug);
					}

					_cs << _rep << flush;
					break;
				}
				catch(zapata::SyntaxErrorException& e) {
					zapata::log(e.what(), zapata::error);
				}
				catch(zapata::ClosedException& e) {
					zapata::log(e.what(), zapata::error);
					break;
				}
			}
			this->__cur_fd = -1;
		}

	}
}

void zapata::RESTJob::assign(int _cs_fd) {
	this->__cur_fd = _cs_fd;
}

zapata::RESTPool& zapata::RESTJob::pool() {
	return *this->__pool;
}

void zapata::RESTJob::pool(RESTPool* _pool) {
	this->__pool = _pool;
}
