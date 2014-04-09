#include <text/convert.h>

#include <unistd.h>
#include <iomanip>

using namespace std;
using namespace __gnu_cxx;

void zapata::url_encode(string& _out) {
	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const unsigned char * pSrc = (const unsigned char *) _out.c_str();
	const int SRC_LEN = _out.length();
	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	unsigned char * pEnd = pStart;
	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	for (; pSrc < SRC_END; ++pSrc) {
		if ((*pSrc > 127) || (*pSrc == '%') || (*pSrc == ' ') || (*pSrc == '&') || (*pSrc == '+') || (*pSrc == '?') || (*pSrc == '#') || (*pSrc == '=') || (*pSrc == '/') || (*pSrc == ':')) {
			*pEnd++ = '%';
			*pEnd++ = DEC2HEX[*pSrc >> 4];
			*pEnd++ = DEC2HEX[*pSrc & 0x0F];
		}
		else {
			*pEnd++ = *pSrc;
		}
	}

	std::string sResult((char *) pStart, (char *) pEnd);
	delete[] pStart;
	_out.assign(sResult);
}

void zapata::url_decode(string& _out) {
	zapata::replace(_out, "+", "%20");

	const unsigned char * pSrc = (const unsigned char *) _out.c_str();
	const int SRC_LEN = _out.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC) {
		if (*pSrc == '%') {
			stringstream ss;
			ss << *(pSrc + 1) << *(pSrc + 2);
			int c;
			ss >> hex >> c;
			*pEnd++ = (char) c;
			pSrc += 3;
			continue;
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END) {
		*pEnd++ = *pSrc++;
	}
	std::string sResult(pStart, pEnd);
	delete[] pStart;
	_out.assign(sResult);
}
