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

#include <ossp/uuid++.hh>
#include <zapata/smtp/SMTP.h>

#ifndef CRLF
#define CRLF "\r\n"
#endif

zpt::SMTPPtr::SMTPPtr()
  : std::shared_ptr<zpt::SMTP>(new zpt::SMTP()) {}

zpt::SMTPPtr::~SMTPPtr() {}

zpt::SMTP::SMTP()
  : __port(0) {}

zpt::SMTP::~SMTP() {}

auto zpt::SMTP::credentials(std::string const& _user, std::string const& _passwd) -> void {
    this->__user.assign(_user);
    this->__passwd.assign(_passwd);
}

auto zpt::SMTP::user() -> std::string { return this->__user; }

auto zpt::SMTP::passwd() -> std::string { return this->__passwd; }

auto zpt::SMTP::connect(std::string const& _connection) -> void {
    std::lock_guard<std::mutex> _lock(this->__mtx);
    this->__connection.assign(_connection);
    this->__uri = zpt::uri::parse(_connection);

    this->__host.assign(this->__uri["domain"]->ok() ? std::string(this->__uri["domain"])
                                                    : "localhost");
    this->__port = this->__uri["port"]->ok() ? (unsigned int)this->__uri["port"] : 25;
    this->__type = zpt::split(std::string(this->__uri["scheme"]), "+");

    if (this->__uri["user"]->ok()) { this->__user.assign(std::string(this->__uri["user"])); }
    if (this->__uri["password"]->ok()) {
        this->__passwd.assign(std::string(this->__uri["password"]));
    }
}

auto zpt::SMTP::compose(zpt::json _e_mail) -> std::string {
    std::ostringstream _oss;
    _oss.str("");

    bool first = true;
    std::string _recipients;
    zpt::json _parsed_to = zpt::json::array();
    for (auto _r : _e_mail["To"]->array()) {
        zpt::json _parsed = zpt::email::parse(std::string(_r));
        _parsed_to << _parsed;
        if (!first) { _recipients += std::string(", "); }
        first = false;
        _recipients += (_parsed["name"]->ok()
                          ? zpt::quoted_printable::r_encode(std::string(_parsed["name"]), "utf-8") +
                              std::string(" <")
                          : std::string("")) +
                       std::string(_parsed["address"]) +
                       (_parsed["name"]->ok() ? std::string(">") : std::string(""));
    }

    zpt::json _from = zpt::email::parse(std::string(_e_mail["From"]));
    _oss << "Date: " << zpt::tostr(time(nullptr), "%a, %d %b %Y %X %Z") << CRLF << std::flush;
    _oss << "From: "
         << (_from["name"]->ok()
               ? zpt::quoted_printable::r_encode(std::string(_from["name"]), "utf-8") +
                   std::string(" <")
               : std::string(""))
         << std::string(_from["address"])
         << (_from["name"]->ok() ? std::string(">") : std::string("")) << CRLF << std::flush;
    if (_e_mail["Sender"]->ok()) {
        zpt::json _sender = zpt::email::parse(std::string(_e_mail["Sender"]));
        _oss << "Sender: " << std::string(_sender["address"]) << CRLF << std::flush;
    }
    else { _oss << "Sender: " << std::string(_from["address"]) << CRLF << std::flush; }
    _oss << "Subject: " << zpt::quoted_printable::r_encode(std::string(_e_mail["Subject"]), "utf-8")
         << CRLF << std::flush;
    if (_e_mail["Reply-To"]->ok()) {
        zpt::json _reply_to = zpt::email::parse(std::string(_e_mail["Reply-To"]));
        _oss << "Reply-To: "
             << (_reply_to["name"]->ok()
                   ? zpt::quoted_printable::r_encode(std::string(_reply_to["name"]), "utf-8") +
                       std::string(" <")
                   : std::string(""))
             << std::string(_reply_to["address"])
             << (_reply_to["name"]->ok() ? std::string(">") : std::string("")) << CRLF
             << std::flush;
    }
    _oss << "To: " << _recipients << CRLF << std::flush;

    std::string _boundary =
      std::string("============") + zpt::generate::r_uuid() + std::string("==");
    _oss << "Content-Type: multipart/alternative; boundary=\"" << _boundary << "\"" << CRLF
         << std::flush;

    for (auto _part : _e_mail["Body"]->object()) {
        _oss << CRLF << std::flush;
        _oss << "--" << _boundary << CRLF << std::flush;
        _oss << "Content-Type: " << _part.first << "; charset=\"utf-8\"" << CRLF << std::flush;
        _oss << "MIME-Version: 1.0" << CRLF << std::flush;
        _oss << "Content-Transfer-Encoding: 8bit" << CRLF << CRLF << std::flush;
        _oss << std::string(_part.second) << CRLF << std::flush;
    }
    _oss << "--" << _boundary << "--" << CRLF << std::flush;
    _oss << "." << CRLF << std::flush;

    _e_mail << "To" << _parsed_to;
    _e_mail << "From" << _from;

    return _oss.str();
}

auto zpt::SMTP::open() -> mailsmtp* {
    mailsmtp* _smtp = nullptr;

    expect((_smtp = mailsmtp_new(0, NULL)) != NULL, "could not create mailsmtp instance");

    try {
        if (this->__type[1] == zpt::json::string("ssl")) {
            expect(mailsmtp_ssl_connect(_smtp, this->__host.data(), this->__port) ==
                     MAILSMTP_NO_ERROR,
                   std::string("could not connect to ") + this->__connection);
        }
        else if (this->__type[1] == zpt::json::string("tls")) {
            expect(mailsmtp_socket_connect(_smtp, this->__host.data(), this->__port) ==
                     MAILSMTP_NO_ERROR,
                   std::string("could not connect to ") + this->__connection);
        }
        else {
            expect(mailsmtp_socket_connect(_smtp, this->__host.data(), this->__port) ==
                     MAILSMTP_NO_ERROR,
                   std::string("could not connect to ") + this->__connection);
        }

        if (this->__type[0] == zpt::json::string("smtp")) {
            expect(mailsmtp_helo(_smtp) == MAILSMTP_NO_ERROR,
                   std::string("could not introduce my self to ") + this->__connection +
                     std::string(" using SMTP"));
        }
        else if (this->__type[0] == zpt::json::string("esmtp")) {
            expect(mailesmtp_ehlo(_smtp) == MAILSMTP_NO_ERROR,
                   std::string("could not introduce my self to ") + this->__connection +
                     std::string(" using ESMTP"));
        }

        if (this->__type[0] == zpt::json::string("esmtp") &&
            this->__type[1] == zpt::json::string("tls")) {
            expect(mailsmtp_socket_starttls(_smtp) == MAILSMTP_NO_ERROR,
                   std::string("could not STARTTLS with ") + this->__connection);
            expect(mailesmtp_ehlo(_smtp) == MAILSMTP_NO_ERROR,
                   std::string("could not introduce my self to ") + this->__connection +
                     std::string(" using ESMTP"));
        }

        if (this->__user.length() != 0 && this->__type[0] == zpt::json::string("esmtp")) {
            expect(mailsmtp_auth(_smtp, this->__user.data(), this->__passwd.data()) ==
                     MAILSMTP_NO_ERROR,
                   std::string("could not authenticate propertly with ") + this->__connection);
        }
    }
    catch (zpt::failed_expectation const& _e) {
        this->close(_smtp);
        throw;
    }
    return _smtp;
}

auto zpt::SMTP::close(mailsmtp* _smtp) -> void {
    if (_smtp != nullptr) { mailsmtp_free(_smtp); }
}

auto zpt::SMTP::send(zpt::json _e_mail) -> void {
    mailsmtp* _smtp = this->open();

    try {
        if (this->__type[0] == zpt::json::string("esmtp")) {
            expect(mailesmtp_mail(_smtp,
                                  std::string(_e_mail["From"]["address"]).data(),
                                  1,
                                  "etPanSMTPZapataWrapper") == MAILSMTP_NO_ERROR,
                   std::string("could not send email through ") + this->__connection);
        }
        else {
            expect(mailsmtp_mail(_smtp, std::string(_e_mail["From"]["address"]).data()) ==
                     MAILSMTP_NO_ERROR,
                   std::string("could not send email through ") + this->__connection);
        }

        expect(_e_mail["To"]->is_array(), "'To' attribute must be a JSON array")
          std::string _mimed_mail = this->compose(_e_mail);
        zdbg(_mimed_mail);

        for (auto _r : _e_mail["To"]->array()) {
            if (this->__type[0] == zpt::json::string("esmtp")) {
                expect(mailesmtp_rcpt(_smtp,
                                      std::string(_r["address"]).data(),
                                      MAILSMTP_DSN_NOTIFY_FAILURE | MAILSMTP_DSN_NOTIFY_DELAY,
                                      nullptr) == MAILSMTP_NO_ERROR,
                       std::string("could not add recipient ") + std::string(_r["address"]));
            }
            else {
                expect(mailsmtp_rcpt(_smtp, std::string(_r["address"]).data()) == MAILSMTP_NO_ERROR,
                       std::string("could not add recipient ") + std::string(_r["address"]));
            }
        }

        expect(mailsmtp_data(_smtp) == MAILSMTP_NO_ERROR,
               std::string("could not send DATA command to ") + this->__connection);
        expect(mailsmtp_data_message(_smtp, _mimed_mail.data(), _mimed_mail.length()) ==
                 MAILSMTP_NO_ERROR,
               std::string("could not send email through ") + this->__connection);

        this->close(_smtp);
    }
    catch (zpt::failed_expectation const& _e) {
        this->close(_smtp);
        throw;
    }
}

extern "C" auto zpt_smtp() -> int { return 1; }
