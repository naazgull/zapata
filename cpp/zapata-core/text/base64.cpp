#include <text/convert.h>

#include <unistd.h>
#include <iomanip>

using namespace std;
using namespace __gnu_cxx;

void zapata::base64_encode(string& _out) {
	istringstream in;
	ostringstream out;
	char buff1[3];
	char buff2[4];
	uint8_t i = 0, j;

	in.str(_out);

	while (in.readsome(&buff1[i++], 1))
		if (i == 3) {
			out << encodeCharacterTable[(buff1[0] & 0xfc) >> 2];
			out << encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
			out << encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
			out << encodeCharacterTable[buff1[2] & 0x3f];
			i = 0;
		}

	if (--i) {
		for (j = i; j < 3; j++)
			buff1[j] = '\0';

		buff2[0] = (buff1[0] & 0xfc) >> 2;
		buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
		buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
		buff2[3] = buff1[2] & 0x3f;

		for (j = 0; j < (i + 1); j++)
			out << encodeCharacterTable[(uint8_t) buff2[j]];

		while (i++ < 3)
			out << '=';
	}
	out << flush;
	_out.assign(out.str());
}

void zapata::base64_decode(string& _out) {
	istringstream in;
	ostringstream out;
	char buff1[4];
	char buff2[4];
	uint8_t i = 0, j;

	in.str(_out);

	while (in.readsome(&buff2[i], 1) && buff2[i] != '=') {
		if (++i == 4) {
			for (i = 0; i != 4; i++)
				buff2[i] = decodeCharacterTable[(uint8_t) buff2[i]];

			out << (char) ((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
			out << (char) (((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
			out << (char) (((buff2[2] & 0x3) << 6) + buff2[3]);

			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			buff2[j] = '\0';
		for (j = 0; j < 4; j++)
			buff2[j] = decodeCharacterTable[(uint8_t) buff2[j]];

		buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
		buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
		buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];

		for (j = 0; j < (i - 1); j++)
			out << (char) buff1[j];
	}
	out << flush;
	_out.assign(out.str());
}

void zapata::base64url_encode(string& _out) {
	istringstream in;
	ostringstream out;
	char buff1[4];
	char buff2[4];
	uint8_t i = 0, j;

	in.str(_out);

	while (in.readsome(&buff1[i++], 1))
		if (i == 3) {
			out << encodeCharacterTableUrl[(buff1[0] & 0xfc) >> 2];
			out << encodeCharacterTableUrl[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
			out << encodeCharacterTableUrl[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
			out << encodeCharacterTableUrl[buff1[2] & 0x3f];
			i = 0;
		}

	if (--i) {
		for (j = i; j < 3; j++)
			buff1[j] = '\0';

		buff2[0] = (buff1[0] & 0xfc) >> 2;
		buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
		buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
		buff2[3] = buff1[2] & 0x3f;

		for (j = 0; j < (i + 1); j++)
			out << encodeCharacterTableUrl[(uint8_t) buff2[j]];

//		while (i++ < 3)
//			out << '=';
	}
	out << flush;
	_out.assign(out.str());
}

void zapata::base64url_decode(string& _out) {
	istringstream in;
	ostringstream out;
	char buff1[4];
	char buff2[4];
	uint8_t i = 0, j;

	while (in.readsome(&buff2[i], 1) && buff2[i] != '=') {
		if (++i == 4) {
			for (i = 0; i != 4; i++)
				buff2[i] = decodeCharacterTableUrl[(uint8_t) buff2[i]];

			out << (char) ((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
			out << (char) (((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
			out << (char) (((buff2[2] & 0x3) << 6) + buff2[3]);

			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			buff2[j] = '\0';
		for (j = 0; j < 4; j++)
			buff2[j] = decodeCharacterTableUrl[(uint8_t) buff2[j]];

		buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
		buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
		buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];

		for (j = 0; j < (i - 1); j++)
			out << (char) buff1[j];
	}
	out << flush;
	_out.assign(out.str());
}
