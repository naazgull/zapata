#pragma once

#include <resource/Login.h>

namespace zapata {
	
	class UserLogin: public Login {
		public:
			UserLogin();
			virtual ~UserLogin();

			virtual bool authenticate(string _id, string _secret, string& _out_code);

	};

}
