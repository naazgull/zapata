#pragma once

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {
	bool sendmail(string _to, string _from, string _subject, string _message, string _replyto = "");
}
