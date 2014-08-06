/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <text/html.h>

#include <unistd.h>
#include <iomanip>
#include <text/manip.h>

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

void zapata::content_boundary(string& _in, string& _out) {
	size_t _idx = _in.find("boundary=");
	if (_idx != string::npos) {
		_out.assign(_in.substr(_idx + 9));
	}
}

void zapata::fromformurlencoded(string& _in, JSONObj& _out) {
	stringstream _ss(_in);
	string _item;
	size_t _separator = 0;

	while (std::getline(_ss, _item, '&')) {
		_separator = _item.find("=");
		if (_separator != string::npos) {
			string _pname(_item.substr(0, _separator));
			string _pvalue(_item.substr(_separator + 1));
			zapata::url_decode(_pname);
			zapata::url_decode(_pvalue);

			if (!!_out[_pname.data()]) {
				if (((JSONElement) _out[_pname]).type() == zapata::JSArray) {
					_out[_pname] << _pvalue;
				}
				else {
					string _cvalue = (string)  _out[_pname];
					_out->erase(_out->find(_pname));

					JSONArr _arr;
					_arr << _cvalue << _pvalue;
					_out << _pname << _arr;
				}
			}
			else {
				_out << _pname << _pvalue;
			}
		}
	}
}

void zapata::fromformdata(string& _in, string _boundary, string _tmp_path, JSONObj& _out) {
	zapata::trim(_boundary);
	zapata::trim(_in);
	size_t _separator = 0;
	size_t _last = _in.find(_boundary)  + _boundary.length() + 2;

	while ((_separator = _in.find(_boundary, _last)) != string::npos) {
		string _part = _in.substr(_last, _separator - _last + _boundary.length());
		string _body;

		zapata::trim(_part);

		size_t _header_body_sep = _part.find("\r\n\r\n");
		string _head;
		size_t _body_len = 0;
		if (_header_body_sep != string::npos) {
			_head.assign(_part.substr(0, _header_body_sep));
			_body_len = _part.find(_boundary) - _header_body_sep - 8;
			_body.assign(_part.substr(_header_body_sep + 4, _body_len).data(), _body_len);
		}
		else {
			continue;
		}

		istringstream _iss(_head, ios_base::in);
		string _line;
		string _pname;
		bool _is_base64 = false;
		while (_iss.good()) {
			std::getline(_iss, _line);
			size_t _header_name_sep = _line.find(":");
			if (_header_name_sep != string::npos) {
				string _n = _line.substr(0, _header_name_sep);
				::transform(_n.begin(), _n.end(), _n.begin(), ::tolower);
				if (_n == "content-enconding") {
					if (_line.find("Base64") != string::npos) {
						_is_base64 = true;
					}
				}
				else if (_n == "content-disposition") {
					if (_line.find("filename=") != string::npos) {
						size_t _name_sep = _line.find("name=") + 5;
						_pname.assign(_line.substr(_name_sep, _line.find(";", _name_sep) - _name_sep));
						zapata::replace(_pname, "\"", " ");
						zapata::trim(_pname);

						string _filename = _line.substr(_line.find("filename=") + 9);
						zapata::replace(_filename, "\"", " ");
						zapata::trim(_filename);

						_out << _pname << _filename;

						if (_is_base64) {
							zapata::base64_decode(_body);
							_body_len = _body.length();
						}

						string _tmp_file(_tmp_path);
						zapata::tostr(_tmp_file, time(NULL));
						_tmp_file.insert(_tmp_file.length(), _filename);

						ofstream ofs;
						ofs.open(_tmp_file.data());
						ofs.write(_body.c_str(), _body_len);
						ofs.flush();
						ofs.close();

						_pname.insert(_pname.length(), "_tmp");
						_out << _pname << _tmp_file;
					}
					else {
						_pname.assign(_line.substr(_line.find("name=") + 5));
						zapata::replace(_pname, "\"", " ");
						zapata::trim(_pname);

						_out << _pname << _body;
					}
					break;
				}
			}
		}

		_last = _separator + _boundary.length() + 2;
	}
}

