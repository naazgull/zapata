/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
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
	_out.assign("");

	while (in.readsome(&buff1[i++], 1))
		if (i == 3) {
			_out.push_back(encodeCharacterTable[(buff1[0] & 0xfc) >> 2]);
			_out.push_back(encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)]);
			_out.push_back(encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)]);
			_out.push_back(encodeCharacterTable[buff1[2] & 0x3f]);
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
			_out.push_back(encodeCharacterTable[(uint8_t) buff2[j]]);

		while (i++ < 3)
			_out.push_back('=');
	}
}

void zapata::base64_decode(string& _out) {
	istringstream in;
	ostringstream out;
	char buff1[4];
	char buff2[4];
	uint8_t i = 0, j;

	in.str(_out);
	_out.assign("");

	while (in.readsome(&buff2[i], 1) && buff2[i] != '=') {
		if (++i == 4) {
			for (i = 0; i != 4; i++)
				buff2[i] = decodeCharacterTable[(uint8_t) buff2[i]];

			_out.push_back((char) ((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4)));
			_out.push_back((char) (((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2)));
			_out.push_back((char) (((buff2[2] & 0x3) << 6) + buff2[3]));

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
			_out.push_back((char) buff1[j]);
	}
}

void zapata::base64_encode(istream& _in, ostream& _out) {
	char buff1[3];
	char buff2[4];
	uint8_t i = 0, j;

	while (_in.readsome(&buff1[i++], 1))
		if (i == 3) {
			_out << encodeCharacterTable[(buff1[0] & 0xfc) >> 2];
			_out << encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
			_out << encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
			_out << encodeCharacterTable[buff1[2] & 0x3f];
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
			_out << encodeCharacterTable[(uint8_t) buff2[j]];

		while (i++ < 3)
			_out << '=';
	}
}

void zapata::base64_decode(istream& _in, ostream& _out) {
	char buff1[4];
	char buff2[4];
	uint8_t i = 0, j;

	while (_in.readsome(&buff2[i], 1) && buff2[i] != '=') {
		if (++i == 4) {
			for (i = 0; i != 4; i++)
				buff2[i] = decodeCharacterTable[(uint8_t) buff2[i]];

			_out << (char) ((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
			_out << (char) (((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
			_out << (char) (((buff2[2] & 0x3) << 6) + buff2[3]);

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
			_out << (char) buff1[j];
	}
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
