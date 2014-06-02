#include <mail/manip.h>

bool zapata::sendmail(string _to, string _from, string _subject, string _message, string _replyto) {
	bool _retval = false;
	FILE *_mailpipe = popen("/usr/sbin/sendmail -t", "w");
	if (_mailpipe != NULL) {
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
		pclose(_mailpipe);
		_retval = true;
	}
	return _retval;
}
