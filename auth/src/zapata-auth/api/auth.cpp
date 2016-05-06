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
#include <zapata/api/RESTEmitter.h>
#include <zapata/api/codes_rest.h>
#include <zapata/api/codes_users.h>
#include <zapata/http/requester.h>
#include <zapata/json/JSONObj.h>
#include <mongo/client/dbclient.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/connpool.h>
#include <mongo/client/dbclientcursor.h>
#include <mongo/client/dbclientinterface.h>
#include <zapata/db/convert_mongo.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zapata/http/params.h>
#include <ctime>
#include <memory>

namespace zapata {

	namespace auth {
		void register(zapata::RESTEmitter& _pool, zapata::AuthAgent * _auth_agent) {
			vector<zapata::ev::Performative> _ets = { zapata::ev::Get, zapata::ev::Post };
			_pool->on(_ets, "^/oauth/connect$",
				[] (zapata::ev::Performative _performative, std::string _resource, zapata::JSONPtr _payload, zapata::EventEmitterPtr _events) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONObj _record = (zapata::JSONObj&) zapata::fromstr(_body);
					if (!_record["id"]->ok() && _record["email"]->ok()) {
						_record << "id" << (string) _record["email"];
					}
					if (!_record["email"]->ok() && _record["id"]->ok()) {
						string _email((string) _record["id"]);
						_email.insert(_email.length(), "@");
						_email.insert(_email.length(), (string) _config["zapata"] ["rest"]["bind_address"]);
						_record << "email" << _email;
					}

					assertz(_record["fullname"]->ok(), "The 'name' field is mandatory", zapata::HTTP412, zapata::ERRNameMandatory);
					assertz(_record["id"]->ok(), "The 'id' field is mandatory", zapata::HTTP412, zapata::ERRIDMandatory);
					assertz(_record["email"]->ok(), "The 'email' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					assertz(_record["password"]->ok(), "The 'password' field is mandatory", zapata::HTTP412, zapata::ERRPasswordMandatory);
					assertz(_record["confirmation_password"]->ok(), "The 'confirmation_password' field is mandatory", zapata::HTTP412, zapata::ERRConfirmationMandatory);
					assertz(((string ) _record["confirmation_password"]) == ((string ) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

					zapata::JSONPtr _req_body = zapata::mongodb::create_document(_config, (string) _config["users"]["mongodb"]["collection"], _record);
					string _text = (string) _req_body;
					_rep->status(zapata::HTTP201);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Location", (string) _req_body["href"]);
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				}
			);
		}
	}
}

extern "C" void populate(zapata::EventEmitterPtr& _pool) {
	zapata::auth::register(_pool);
}

extern "C" int zapata_auth() {
	return 1;
}