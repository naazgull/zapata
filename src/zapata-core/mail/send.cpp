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

#include <zapata/mail/manip.h>

bool zapata::sendmail(string _to, string _from, string _subject, string _message, string _replyto) {
	bool _retval = false;
	FILE *_mailpipe = popen("/usr/sbin/sendmail -t", "w");
	if (_mailpipe != nullptr) {
		fprintf(_mailpipe, "To: %s\n", _to.data());
		fprintf(_mailpipe, "From: %s\n", _from.data());
		if (_replyto.length() != 0) {
			fprintf(_mailpipe, "Reply-To: %s\n", _replyto.data());
		}
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
