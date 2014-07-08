#pragma once

#include <resource/ExchangeToken.h>

namespace zapata {
	
	class UserExchangeToken: public ExchangeToken {
		public:
			UserExchangeToken();
			virtual ~UserExchangeToken();

			virtual bool usrtoken(string _id, string _secret, string _code, zapata::JSONObj& _out_token);
			virtual bool apptoken(string _id, string _secret, string _code, zapata::JSONObj& _out_token);

	};

}
