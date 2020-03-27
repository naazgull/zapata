/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <stddef.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zapata/log/log.h>
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>

#include <crypto++/filters.h>
#include <crypto++/aes.h>
#include <crypto++/gcm.h>

void
zpt::ltrim(std::string& _in_out) {
    _in_out.erase(_in_out.begin(),
                  std::find_if(_in_out.begin(),
                               _in_out.end(),
                               std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void
zpt::rtrim(std::string& _in_out) {
    _in_out.erase(std::find_if(_in_out.rbegin(),
                               _in_out.rend(),
                               std::not1(std::ptr_fun<int, int>(std::isspace)))
                    .base(),
                  _in_out.end());
}

void
zpt::trim(std::string& _in_out) {
    zpt::ltrim(_in_out);
    zpt::rtrim(_in_out);
}

auto
zpt::replace(std::string& str, std::string find, std::string replace) -> void {
    if (str.length() == 0) {
        return;
    }

    size_t start = 0;

    while ((start = str.find(find, start)) != std::string::npos) {
        str.replace(start, find.size(), replace);
        start += replace.length();
    }
}

void
zpt::normalize_path(std::string& _in_out, bool _with_trailing) {
    if (_with_trailing) {
        if (_in_out[_in_out.length() - 1] != '/') {
            _in_out.insert(_in_out.length(), "/");
        }
    }
    else {
        if (_in_out[_in_out.length() - 1] == '/') {
            _in_out.erase(_in_out.length() - 1, 1);
        }
    }
}

void
zpt::cipher(std::string const& _in, std::string const& _key, std::string& _out) {
    unsigned int _ikey = _key.length(), iIn = _in.length(), x = 0;
    std::string _s_encrypted(_in);

    for (unsigned int i = 0; i < iIn; i++) {
        _s_encrypted[i] = _in[i] ^ (_key[x] & 10);
        if (++x == _ikey) {
            x = 0;
        }
    }
    _out.assign(_s_encrypted);
}

void
zpt::decipher(std::string const& _in, std::string const& _key, std::string& _out) {
    zpt::cipher(_in, _key, _out);
}

void
zpt::encrypt(std::string& _out, std::string const& _in, std::string const& _key) {
    Bytef* src = new Bytef[_in.length()];
    size_t destLen = (size_t)(_in.length() * 1.1 + 12);
    Bytef* dest = new Bytef[destLen];

    for (size_t i = 0; i != _in.length(); i++) {
        src[i] = (Bytef)_in[i];
    }

    compress(dest, &destLen, src, _in.length());

    std::ostringstream cos;
    for (size_t i = 0; i != destLen; i++) {
        cos << dest[i];
    }
    cos << std::flush;

    std::string _encrypted;
    zpt::cipher(cos.str(), _key, _encrypted);

    zpt::tostr(_out, _in.length());
    _out.insert(_out.length(), ".");
    zpt::base64::encode(_encrypted);

    delete[] src;
    delete[] dest;

    _out.assign(_encrypted);
}

void
zpt::decrypt(std::string& _out, std::string const& _in, std::string const& _key) {
    int _idx = _in.find('.');
    std::string _length(_in.substr(0, _idx));
    size_t _size = 0;
    zpt::fromstr(_length, &_size);
    if (_size == 0) {
        return;
    }

    std::string _encrypted = _in.substr(_idx + 1);
    zpt::base64::decode(_encrypted);

    std::string _decrypted;
    zpt::decipher(_encrypted, _key, _decrypted);

    Bytef* src = new Bytef[_decrypted.length()];
    size_t destLen = _size;
    Bytef* dest = new Bytef[destLen];

    for (size_t i = 0; i != _decrypted.length(); i++) {
        src[i] = (Bytef)_decrypted[i];
    }

    uncompress(dest, &destLen, src, _decrypted.length());

    std::ostringstream cos;
    for (size_t i = 0; i != destLen; i++) {
        cos << dest[i];
    }
    cos << std::flush;

    delete[] src;
    delete[] dest;

    _out.assign(cos.str());
}

void
zpt::prettify_header_name(std::string& name) {
    std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);

    std::stringstream iss;
    iss << name;

    char line[256];
    size_t pos = 0;
    while (iss.good()) {
        iss.getline(line, 256, '-');
        pos += iss.gcount();
        std::transform(name.begin() + pos, name.begin() + pos + 1, name.begin() + pos, ::toupper);
    }
}

std::string
zpt::r_ltrim(std::string const& _in_out) {
    std::string _return(_in_out.data());
    _return.erase(_return.begin(),
                  std::find_if(_return.begin(),
                               _return.end(),
                               std::not1(std::ptr_fun<int, int>(std::isspace))));
    return _return;
}

std::string
zpt::r_rtrim(std::string const& _in_out) {
    std::string _return(_in_out.data());
    _return.erase(std::find_if(_return.rbegin(),
                               _return.rend(),
                               std::not1(std::ptr_fun<int, int>(std::isspace)))
                    .base(),
                  _return.end());
    return _return;
}

std::string
zpt::r_trim(std::string const& _in_out) {
    std::string _return(_in_out.data());
    zpt::ltrim(_return);
    zpt::rtrim(_return);
    return _return;
}

std::string
zpt::r_replace(std::string str, std::string find, std::string replace) {
    std::string _return(str.data());
    try {
        if (_return.length() == 0) {
            return _return;
        }

        size_t start = 0;

        while ((start = _return.find(find, start)) != std::string::npos) {
            _return.replace(start, find.size(), replace);
            start += replace.length();
        }
    }
    catch (std::exception const& _e) {
        std::cout << (_e.what()) << std::endl << std::flush;
    }
    return _return;
}

std::string
zpt::r_normalize_path(std::string const& _in_out, bool _with_trailing) {
    std::string _return(_in_out.data());
    if (_with_trailing) {
        if (_return[_return.length() - 1] != '/') {
            _return.insert(_return.length(), "/");
        }
    }
    else {
        if (_return[_return.length() - 1] == '/') {
            _return.erase(_return.length() - 1, 1);
        }
    }
    return _return;
}

std::string
zpt::r_cipher(std::string const& _in, std::string const& _key) {
    std::string _out;
    unsigned int _ikey = _key.length(), iIn = _in.length(), x = 0;
    std::string _s_encrypted(_in);

    for (unsigned int i = 0; i < iIn; i++) {
        _s_encrypted[i] = _in[i] ^ (_key[x] & 10);
        if (++x == _ikey) {
            x = 0;
        }
    }
    _out.assign(_s_encrypted);
    return _out;
}

std::string
zpt::r_decipher(std::string const& _in, std::string const& _key) {
    return zpt::r_cipher(_in, _key);
}

std::string
zpt::r_encrypt(std::string const& _in, std::string const& _key) {
    std::string _out;
    Bytef* src = new Bytef[_in.length()];
    size_t destLen = (size_t)(_in.length() * 1.1 + 12);
    Bytef* dest = new Bytef[destLen];

    for (size_t i = 0; i != _in.length(); i++) {
        src[i] = (Bytef)_in[i];
    }

    compress(dest, &destLen, src, _in.length());

    std::ostringstream cos;
    for (size_t i = 0; i != destLen; i++) {
        cos << dest[i];
    }
    cos << std::flush;

    std::string _encrypted;
    zpt::cipher(cos.str(), _key, _encrypted);

    zpt::tostr(_out, _in.length());
    _out.insert(_out.length(), ".");
    zpt::base64::encode(_encrypted);

    delete[] src;
    delete[] dest;

    _out.assign(_encrypted);
    return _out;
}

std::string
zpt::r_decrypt(std::string const& _in, std::string const& _key) {
    std::string _out;
    int _idx = _in.find('.');
    std::string _length(_in.substr(0, _idx));
    size_t _size = 0;
    zpt::fromstr(_length, &_size);
    if (_size == 0) {
        return "";
    }

    std::string _encrypted = _in.substr(_idx + 1);
    zpt::base64::decode(_encrypted);

    std::string _decrypted;
    zpt::decipher(_encrypted, _key, _decrypted);

    Bytef* src = new Bytef[_decrypted.length()];
    size_t destLen = _size;
    Bytef* dest = new Bytef[destLen];

    for (size_t i = 0; i != _decrypted.length(); i++) {
        src[i] = (Bytef)_decrypted[i];
    }

    uncompress(dest, &destLen, src, _decrypted.length());

    std::ostringstream cos;
    for (size_t i = 0; i != destLen; i++) {
        cos << dest[i];
    }
    cos << std::flush;

    delete[] src;
    delete[] dest;

    _out.assign(cos.str());
    return _out;
}

std::string
zpt::r_prettify_header_name(std::string name) {
    std::string _return(name.data());
    std::transform(_return.begin(), _return.begin() + 1, _return.begin(), ::toupper);

    std::stringstream iss;
    iss << _return;

    char line[256];
    size_t pos = 0;
    while (iss.good()) {
        iss.getline(line, 256, '-');
        pos += iss.gcount();
        std::transform(
          _return.begin() + pos, _return.begin() + pos + 1, _return.begin() + pos, ::toupper);
    }
    return _return;
}

void
zpt::crypto::gcm::aes::encrypt(std::string& _out,
                               const std::string& _in,
                               const std::string& _key,
                               const std::string& _iv,
                               int _tag_size) {

    try {

        CryptoPP::GCM<CryptoPP::AES>::Encryption _enc;
        _enc.SetKeyWithIV((const byte*)_key.data(), _key.size(), (const byte*)_iv.data());

        _out.clear();

        CryptoPP::StringSource(_in,
                               true,
                               new CryptoPP::AuthenticatedEncryptionFilter(
                                 _enc, new CryptoPP::StringSink(_out), false, _tag_size));
    }
    catch (CryptoPP::InvalidArgument const& _exc) {

        zlog(std::string("zpt::cypto::aes::encrypt : Caught InvalidArgument: ") + _exc.what(),
             zpt::error);
    }
    catch (CryptoPP::Exception const& _exc) {

        zlog(std::string("zpt::cypto::aes::encrypt : Caught Exception: ") + _exc.what(),
             zpt::error);
    }
}

void
zpt::crypto::gcm::aes::decrypt(std::string& _out,
                               const std::string& _in,
                               const std::string& _key,
                               const std::string& _iv,
                               int _tag_size) {

    try {

        CryptoPP::GCM<CryptoPP::AES>::Decryption _dec;
        _dec.SetKeyWithIV((const byte*)_key.data(), _key.size(), (const byte*)_iv.data());

        _out.clear();

        CryptoPP::AuthenticatedDecryptionFilter _df(
          _dec,
          new CryptoPP::StringSink(_out),
          CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
          _tag_size);

        CryptoPP::StringSource _ss(_in, true, new CryptoPP::Redirector(_df));

        if (_df.GetLastResult()) {
            zlog("zpt::cypto::aes::decrypt : Error to recover the encrypted text", zpt::error);
        }
    }
    catch (CryptoPP::HashVerificationFilter::HashVerificationFailed const& _e) {

        zlog(std::string("zpt::cypto::aes::decrypt : HashVerificationFailed:") + _e.what(),
             zpt::error);
    }
    catch (CryptoPP::InvalidArgument const& _e) {

        zlog(std::string("zpt::cypto::aes::decrypt : Caught InvalidArgument:") + _e.what(),
             zpt::error);
    }
    catch (CryptoPP::Exception const& _e) {

        zlog(std::string("zpt::cypto::aes::decrypt : Caught Exception:") + _e.what(), zpt::error);
    }
}

auto
zpt::crypto::gcm::aes::r_encrypt(const std::string& _in,
                                 const std::string& _key,
                                 const std::string& _iv,
                                 int _tag_size) -> std::string {

    std::string _out;

    zpt::crypto::gcm::aes::encrypt(_out, _in, _key, _iv, _tag_size);

    return _out;
}

auto
zpt::crypto::gcm::aes::r_decrypt(const std::string& _in,
                                 const std::string& _key,
                                 const std::string& _iv,
                                 int _tag_size) -> std::string {

    std::string _out;

    zpt::crypto::gcm::aes::decrypt(_out, _in, _key, _iv, _tag_size);

    return _out;
}
