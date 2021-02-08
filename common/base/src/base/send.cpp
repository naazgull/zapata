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

#include <zapata/mail/manip.h>

auto
zpt::sendmail(std::string const& _to,
              std::string _from,
              std::string _subject,
              std::string _message,
              std::string _replyto) -> bool {
    bool _retval{ false };
    FILE* _mailpipe = popen("/usr/sbin/sendmail -t", "w");
    if (_mailpipe != nullptr) {
        fprintf(_mailpipe, "To: %s\n", _to.data());
        fprintf(_mailpipe, "From: %s\n", _from.data());
        if (_replyto.length() != 0) { fprintf(_mailpipe, "Reply-To: %s\n", _replyto.data()); }
        fprintf(_mailpipe, "Subject: %s\n", _subject.data());
        fprintf(_mailpipe, "Mime-Version: 1.0\n");
        fprintf(_mailpipe, "Content-Type: text/html; charset=utf-8\n\n");
        fwrite(_message.data(), 1, _message.length(), _mailpipe);
        fwrite(".\n", 1, 2, _mailpipe);
        fflush(_mailpipe);
        pclose(_mailpipe);
        _retval = true;
    }
    return _retval;
}
