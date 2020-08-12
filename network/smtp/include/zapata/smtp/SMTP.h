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

#include <functional>
#include <iostream>
#include <libetpan/libetpan.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <zapata/json.h>

namespace zpt {

class SMTP;
class SMTPPtr;

class SMTPPtr : public std::shared_ptr<zpt::SMTP> {
  public:
    SMTPPtr();
    virtual ~SMTPPtr();
};

namespace smtp {
typedef zpt::SMTPPtr broker;
}

class SMTP {
  public:
    SMTP();
    virtual ~SMTP();

    virtual auto credentials(std::string const& _user, std::string const& _passwd) -> void;

    virtual auto user() -> std::string;
    virtual auto passwd() -> std::string;

    virtual auto connect(std::string const& _connection) -> void;
    virtual auto send(zpt::json _e_mail) -> void;

  private:
    std::string __connection;
    zpt::json __uri;
    std::string __user;
    std::string __passwd;
    std::string __host;
    uint __port;
    zpt::json __type;
    std::mutex __mtx;

    auto open() -> mailsmtp*;
    auto close(mailsmtp* _smtp) -> void;
    auto compose(zpt::json _e_mail) -> std::string;
};
} // namespace zpt
