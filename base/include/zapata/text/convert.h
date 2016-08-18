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

#pragma once

#include <string.h>
#include <stdio.h>
#include <string>
#include <wchar.h>
#include <wctype.h>
#include <stdint.h>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <memory.h>
#include <algorithm>
#include <fstream>
#include <zapata/text/manip.h>
#include <cmath>

#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

using namespace std;
using namespace __gnu_cxx;

namespace zpt {

	void tostr(std::string& s, int i);
	void tostr(std::string& s, bool i);
	void tostr(std::string&, int, std::ios_base& (&)(std::ios_base&));
#ifdef __LP64__
	void tostr(std::string& s, unsigned int i);
#endif
	void tostr(std::string& s, size_t i);
	void tostr(std::string& s, long i);
	void tostr(std::string& s, long long i);
	void tostr(std::string& s, float i);
	void tostr(std::string& s, double i);
	void tostr(std::string& s, char i);
	void tostr(std::string& s, time_t i, const char* f);

	std::string tostr(int i);
	std::string tostr(bool i);
	std::string tostr(int, std::ios_base& (&)(std::ios_base&));
#ifdef __LP64__
	std::string tostr(unsigned int i);
#endif
	std::string tostr(size_t i);
	std::string tostr(long i);
	std::string tostr(long long i);
	std::string tostr(float i);
	std::string tostr(double i);
	std::string tostr(char i);
	std::string tostr(time_t i, const char* f);

	void fromstr(std::string s, int* i);
#ifdef __LP64__
	void fromstr(std::string s, unsigned int* i);
#endif
	void fromstr(std::string s, size_t* i);
	void fromstr(std::string s, long* i);
	void fromstr(std::string s, long long* i);
	void fromstr(std::string s, float* i);
	void fromstr(std::string s, double* i);
	void fromstr(std::string s, char* i);
	void fromstr(std::string s, bool* i);
	void fromstr(std::string s, time_t* i, const char* f);

	const char encodeCharacterTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const char decodeCharacterTable[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	const char encodeCharacterTableUrl[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	const char decodeCharacterTableUrl[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	const wstring iso = L"\u00e1\u00e0\u00e2\u00e3\u00e4\u00e9\u00e8\u00ea\u1ebd\u00eb\u00ed\u00ec\u00ee\u0129\u00ef\u00f3\u00f2\u00f4\u00f5\u00f6\u00fa\u00f9\u00fb\u0169\u00fc\u00e7\u00c1\u00c0\u00c2\u00c3\u00c4\u00c9\u00c8\u00ca\u1ebc\u00cb\u00cd\u00cc\u00ce\u0128\u00cf\u00d3\u00d2\u00d4\u00d5\u00d6\u00da\u00d9\u00db\u0168\u00dc\u00c7";
	const wstring plain = L"aaaaaeeeeeiiiiiooooouuuuucAAAAAEEEEEIIIIIOOOOOUUUUUC";

	time_t timezone_offset();

	namespace base64 {
		void encode(string& _out);
		void decode(string& _out);
		void encode(istream& _in, ostream& _out);
		void decode(istream& _in, ostream& _out);
		void url_encode(string& _out);
		void url_decode(string& _out);
		
	}

	typedef unsigned char uchar;

	namespace utf8 {
		char* wstring_to_utf8(wstring ws);
		wchar_t* utf8_to_wstring(string s);
		int length(string s);
		void encode(wstring s, string& _out, bool quote = true);
		void encode(string& _out, bool quote = true);
		void decode(string& _out);
		
	}

	namespace unicode {
		void escape(std::string& _out);
	}
	
	namespace url {
		void encode(wstring s, ostream& out);
		void encode(string& out);
		void decode(string& out);
	}

	namespace ascii {
		void encode(string& out, bool quote = true);
	}

	namespace hash {
		template<class type>
		void MD5(const type& input, type& hash) {
			MD5_CTX context;
			MD5_Init(&context);
			MD5_Update(&context, &input[0], input.size());

			hash.resize(128/8);
			MD5_Final((unsigned char*)&hash[0], &context);
		}
		template<class type>
		type MD5(const type& input) {
			type hash;
			MD5(input, hash);
			return hash;
		}

		template<class type>
		void SHA1(const type& input, type& hash) {
			SHA_CTX context;
			SHA1_Init(&context);
			SHA1_Update(&context, &input[0], input.size());

			hash.resize(160/8);
			SHA1_Final((unsigned char*)&hash[0], &context);
		}
		template<class type>
		type SHA1(const type& input) {
			type hash;
			SHA1(input, hash);
			return hash;
		}

		template<class type>
		void SHA256(const type& input, type& hash) {
			SHA256_CTX context;
			SHA256_Init(&context);
			SHA256_Update(&context, &input[0], input.size());

			hash.resize(256/8);
			SHA256_Final((unsigned char*)&hash[0], &context);
		}
		template<class type>
		type SHA256(const type& input) {
			type hash;
			SHA256(input, hash);
			return hash;
		}

		template<class type>
		void SHA512(const type& input, type& hash) {
			SHA512_CTX context;
			SHA512_Init(&context);
			SHA512_Update(&context, &input[0], input.size());

			hash.resize(512/8);
			SHA512_Final((unsigned char*)&hash[0], &context);
		}
		template<class type>
		type SHA512(const type& input) {
			type hash;
			SHA512(input, hash);
			return hash;
		}		
	}

	void generate_key(string& _out);
	std::string generate_key();
	void generate_hash(string& _out);

}
