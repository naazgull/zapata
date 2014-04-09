#include <text/convert.h>

#include <unistd.h>
#include <iomanip>

using namespace std;
using namespace __gnu_cxx;

void zapata::html_entities_encode(wstring s, ostream& out, bool quote, bool tags) {
	ostringstream oss;
	for (size_t i = 0; i != s.length(); i++) {
		if (((unsigned char) s[i]) > 127) {
			oss << "&#" << dec << ((int) s.at(i)) << ";";
		}
		else if (s[i] == '"' && quote) {
			oss << "&quot;";
		}
		else if (s[i] == '<' && tags) {
			oss << "&lt;";
		}
		else if (s[i] == '>' && tags) {
			oss << "&gt;";
		}
		else if (s[i] == '&') {
			oss << "&amp;";
		}
		else {
			oss << ((char) s.at(i));
		}
	}
	oss << flush;
	out << oss.str();
}

void zapata::html_entities_encode(string& _out, bool quote, bool tags) {
	wchar_t* wc = zapata::utf8_to_wstring(_out);
	wstring ws(wc);
	ostringstream out;
	zapata::html_entities_encode(ws, out, quote, tags);
	delete[] wc;
	_out.assign(out.str());
}

void zapata::html_entities_decode(string& _out) {
	wostringstream oss;
	for (size_t i = 0; i != _out.length(); i++) {
		if (_out[i] == '&' && _out[i + 1] == '#') {
			stringstream ss;
			int j = i + 2;
			while (_out[j] != ';') {
				ss << _out[j];
				j++;
			}
			int c;
			ss >> c;
			oss << ((wchar_t) c);
			i = j;
		}
		else {
			oss << ((wchar_t) _out[i]);
		}
	}
	oss << flush;

	char* c = zapata::wstring_to_utf8(oss.str());
	string os(c);
	_out.assign(os);

	delete[] c;
}
