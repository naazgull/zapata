#include <text/convert.h>

#include <unistd.h>
#include <iomanip>

using namespace std;
using namespace __gnu_cxx;

void zapata::ascii_encode(string& _out, bool quote) {
	wchar_t* wc = zapata::utf8_to_wstring(_out);
	wstring ws(wc);

	for (size_t i = 0; i != iso.length(); i++) {
		std::replace(ws.begin(), ws.end(), iso[i], ascii[i]);
	}

	ostringstream _oss;
	delete[] wc;
	for (size_t i = 0; i != ws.length(); i++) {
		if (((int) ws[i]) <= 127) {
			_oss << ((char) ws[i]) << flush;
		}
		else {
			_oss << " " << flush;
		}
	}
	_out.assign(_oss.str());
}

void zapata::generate_key(string& _out) {
	static string _charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	string _randompass;
	_randompass.resize(45);

	srand(time(NULL));
	for (int i = 0; i < 45; i++) {
		_randompass[i] = _charset[rand() % _charset.length()];
	};
	_out.insert(_out.length(), _randompass);
}
