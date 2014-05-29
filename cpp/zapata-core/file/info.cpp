#include <file/manip.h>

#include <magic.h>

void zapata::get_mime(string _in, string& _out) {
	magic_t myt = magic_open(MAGIC_CONTINUE | MAGIC_ERROR | MAGIC_MIME);
	magic_load(myt, NULL);
	_out.insert(_out.length(), magic_file(myt, _in.data()));
	magic_close(myt);
}

bool zapata::path_exists(string _in) {
	struct stat  _buffer;
	return ( stat(_in.data(), &_buffer) == 0);
}
