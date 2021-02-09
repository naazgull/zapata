/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
