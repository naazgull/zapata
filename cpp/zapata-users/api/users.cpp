/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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

#include <api/RESTPool.h>
#include <api/codes_rest.h>
#include <api/codes_users.h>
#include <http/requester.h>
#include <json/JSONObj.h>
#include <mongo/client/dbclient.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/connpool.h>
#include <mongo/client/dbclientcursor.h>
#include <mongo/client/dbclientinterface.h>
#include <base/smart_ptr.h>
#include <db/convert_mongo.h>
#include <text/convert.h>
#include <text/manip.h>
#include <http/params.h>
#include <ctime>
#include <memory>

namespace zapata {

	// AUTHENTICATION & TOKEN GENERATION UTILITY FUNCTIONS

	bool authenticate(string _id, string _secret, string& _out_code, zapata::JSONObj& _config) {
		bool _exists = false;

		mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
		string _collection((string) _config["zapata"]["mongodb"]["db"]);
		_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

		unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "password" << _secret));
		_exists = _ptr->more();
		_ptr->decouple();
		 (*_conn)->killCursor(_ptr->getCursorId());
		 delete _ptr.release();

		_conn->done();
		delete _conn;

		if (_exists) {
			zapata::generate_hash(_out_code);
		}

		return _exists;
	}

	bool usrtoken(string _id, string _secret, string _code, zapata::JSONObj& _out, zapata::JSONObj& _config) {
		string _out_token;
		string _cur_date;
		zapata::tostr(_cur_date, time(NULL));
		_out_token.insert(_out_token.length(), _cur_date);
		_out_token.insert(_out_token.length(), "|");
		_out_token.insert(_out_token.length(), _id);
		_out_token.insert(_out_token.length(), "|");

		mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
		string _collection((string) _config["zapata"]["mongodb"]["db"]);
		_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

		unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "password" << _secret));
		if(_ptr->more()) {
			mongo::BSONObj _bo = _ptr->next();
			zapata::frommongo(_bo, _out);
			_out_token.insert(_out_token.length(), _bo.getStringField("scopes"));
			_out_token.insert(_out_token.length(), "|");
			_out_token.insert(_out_token.length(), _bo.getStringField("_id"));
			_out_token.insert(_out_token.length(), "|");
			string _expiration;
			zapata::tostr(_expiration, time(NULL) + 3600 * 24 * 60);
			_out_token.insert(_out_token.length(), _expiration);
		}
		_ptr->decouple();
		 (*_conn)->killCursor(_ptr->getCursorId());
		 delete _ptr.release();

		_conn->done();
		delete _conn;

		string _enc_token;
		zapata::encrypt(_enc_token, _out_token, (string) _config["zapata"]["auth"]["signing_key"]);

		_out >> "password" >> "confirmation_password";
		_out << "access_token" << _enc_token;

		return true;
	}

	bool apptoken(string _id, string _secret, string _code, zapata::JSONObj& _out, zapata::JSONObj& _config) {
		string _out_token;
		string _cur_date;
		zapata::tostr(_cur_date, time(NULL));
		_out_token.insert(_out_token.length(), _cur_date);
		_out_token.insert(_out_token.length(), "|");
		_out_token.insert(_out_token.length(), _id);
		_out_token.insert(_out_token.length(), "|");

		mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
		string _collection((string) _config["zapata"]["mongodb"]["db"]);
		_collection.insert(_collection.length(), _config["zapata_users"]["mongodb"]["collection"]);

		unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "password" << _secret));
		if(_ptr->more()) {
			mongo::BSONObj _bo = _ptr->next();
			zapata::frommongo(_bo, _out);
			_out_token.insert(_out_token.length(), _bo.getStringField("scopes"));
			_out_token.insert(_out_token.length(), "|");
			_out_token.insert(_out_token.length(), _bo.getStringField("_id"));
			_out_token.insert(_out_token.length(), "|");
			string _expiration;
			zapata::tostr(_expiration, time(NULL) + 3600 * 24 * 60);
			_out_token.insert(_out_token.length(), _expiration);
		}
		_ptr->decouple();
		 (*_conn)->killCursor(_ptr->getCursorId());
		 delete _ptr.release();

		_conn->done();
		delete _conn;

		string _enc_token;
		zapata::encrypt(_enc_token, _out_token, (string) _config["zapata"]["auth"]["signing_key"]);

		_out >> "password" >> "confirmation_password";
		_out << "access_token" << _enc_token;

		return true;
	}

}

extern "C" void populate(zapata::RESTPool& _pool) {

	//-------------------------------------------------------------------------------------------------------------------------------------------------
	// USER MANAGEMENT: COLLECTION & DOCUMENT SUPPORT

	/*
	 *  definition of handlers for the '/users' collection
	 *  registered as a Collection
	 */
	{
		_pool.on("^/users$",
			//get
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				zapata::JSONObj _params;
				zapata::fromparams(_req, _params, zapata::RESTfulCollection, true);

				zapata::JSONObj _out;
				try {
					zapata::torestcollection(_conn, _collection, _params, _out);
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}
				_conn->done();
				delete _conn;

				assertz(((size_t) _out["size"]) != 0, "The requested resource is empty", zapata::HTTP204, zapata::ERRResourceIsEmpty);

				string _text;
				zapata::tostr(_text, _out);
				_rep->status(zapata::HTTP200);
				_rep->body(_text);
				_rep << "Cache-Control" << "no-store" << "Pragma" << "no-cache" << "Content-Type" << "application/json" << "Content-Length" << _text.length();
			},
			//put
			NULL,
			//post
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				string _body = _req->body();
				assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

				string _content_type = _req->header("Content-Type");
				assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

				zapata::JSONObj _record;
				zapata::fromstr(_body, _record);
				if (!_record["id"] && !!_record["email"]) {
					_record << "id" << (string) _record["email"];
				}
				if (!_record["email"] && !!_record["id"]) {
					string _email((string) _record["id"]);
					_email.insert(_email.length(), "@");
					_email.insert(_email.length(), (string) _config["zapata"] ["rest"]["bind_address"]);
					_record << "email" << _email;
				}

				assertz(!!_record["fullname"], "The 'name' field is mandatory", zapata::HTTP412, zapata::ERRNameMandatory);
				assertz(!!_record["id"], "The 'id' field is mandatory", zapata::HTTP412, zapata::ERRIDMandatory);
				assertz(!!_record["email"], "The 'email' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
				assertz(!!_record["password"], "The 'password' field is mandatory", zapata::HTTP412, zapata::ERRPasswordMandatory);
				assertz(!!_record["confirmation_password"], "The 'confirmation_password' field is mandatory", zapata::HTTP412, zapata::ERRConfirmationMandatory);
				assertz(((string ) _record["confirmation_password"]) == ((string ) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				string __id;
				bool _exists = true;
				try {

					unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << (string) _record["id"]));
					_exists = _ptr->more();
					_ptr->decouple();
					 (*_conn)->killCursor(_ptr->getCursorId());
					 delete _ptr.release();

					 if (!_exists) {
						bool _error = true;
						do {
							__id.assign("/users/");
							zapata::generate_key(__id);
							_record << "_id" << __id;

							mongo::BSONObjBuilder _bo;
							zapata::tomongo(_record, _bo);
							(*_conn)->insert(_collection, _bo.obj());

							_error = (*_conn)->getLastError().length() != 0;
						}
						while(_error);
					 }
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}

				_conn->done();
				delete _conn;

				 assertz(!_exists, "Already exists a user identified by that ID", zapata::HTTP412, zapata::ERRUserAlreadyExists);

				zapata::JSONObj _rep_body;
				_rep_body << "href" << __id;
				string _text;
				zapata::tostr(_text, _rep_body);
				_rep->status(zapata::HTTP201);
				_rep << "Location" << __id << "Content-Length" << (long) _text.length();
				_rep->body(_text);
			},
			//delete
			NULL,
			//head
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				zapata::JSONObj _params;
				zapata::fromparams(_req, _params, zapata::RESTfulCollection);

				zapata::JSONObj _out;
				try {
					zapata::torestcollection(_conn, _collection, _params, _out);
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}
				_conn->done();
				delete _conn;

				assertz(((size_t) _out["size"]) != 0, "The requested resource is empty", zapata::HTTP204, zapata::ERRResourceIsEmpty);

				string _text;
				zapata::tostr(_text, _out);
				_rep->status(zapata::HTTP200);
				_rep << "Cache-Control" << "no-store" << "Pragma" << "no-cache" << "Content-Type" << "application/json" << "Content-Length" << _text.length();
			},
			//trace
			NULL,
			//options
			NULL,
			//patch
			NULL,
			//connect
			NULL, zapata::RESTfulCollection);
	}

	/*
	 *  definition of handlers for the '/users/{id]' document
	 *  registered as a Document
	 */
	{
		_pool.on("^/users/([^/]+)$",
			//get
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				zapata::JSONObj _params;
				zapata::fromparams(_req, _params, zapata::RESTfulDocument);

				zapata::JSONObj _out;
				try {
					zapata::torestdocument(_conn, _collection, _params, _out);
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}
				_conn->done();
				delete _conn;

				assertz(_out->size() != 0, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

				string _text;
				zapata::tostr(_text, _out);
				_rep->status(zapata::HTTP200);
				_rep->body(_text);
				_rep << "Cache-Control" << "no-store" << "Pragma" << "no-cache" << "Content-Type" << "application/json" << "Content-Length" << _text.length();
			},
			//put
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				string _body = _req->body();
				assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

				string _content_type = _req->header("Content-Type");
				assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

				zapata::JSONObj _record;
				zapata::fromstr(_body, _record);

				assertz(!!_record["fullname"], "The 'name' field is mandatory", zapata::HTTP412, zapata::ERRNameMandatory);
				assertz(!!_record["id"], "The 'id' field is mandatory", zapata::HTTP412, zapata::ERRIDMandatory);
				assertz(!!_record["email"], "The 'email' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
				assertz(!!_record["password"], "The 'password' field is mandatory", zapata::HTTP412, zapata::ERRPasswordMandatory);
				assertz(!!_record["confirmation_password"], "The 'confirmation_password' field is mandatory", zapata::HTTP412, zapata::ERRConfirmationMandatory);
				assertz(((string ) _record["confirmation_password"]) == ((string ) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				string __id(_req->url());

				bool _exists = false;
				try {
					mongo::BSONObjBuilder _id_bo;
					_id_bo.append("_id", __id);

					mongo::BSONObjBuilder _bo;
					zapata::tomongo(_record, _bo);

					(*_conn)->update(_collection, _id_bo.obj(),  _bo.obj());

					_exists = (*_conn)->getLastError().length() == 0;
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}

				_conn->done();
				delete _conn;

				 assertz(_exists, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

				zapata::JSONObj _rep_body;
				_rep_body << "href" << __id;
				string _text;
				zapata::tostr(_text, _rep_body);
				_rep->status(zapata::HTTP200);
				_rep << "Cache-Control" << "no-store" << "Pragma" << "no-cache" << "Location" << __id << "Content-Length" << (long) _text.length();
				_rep->body(_text);
			},
			//post
			NULL,
			//delete
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				zapata::JSONObj _params;
				zapata::fromparams(_req, _params, zapata::RESTfulDocument);

				zapata::JSONObj _out;
				try {
					zapata::torestdocument(_conn, _collection, _params, _out);
					(*_conn)->remove(_collection, QUERY("_id" << _req->url()), true);
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}
				_conn->done();
				delete _conn;

				string _text;
				zapata::tostr(_text, _out);
				_rep->status(zapata::HTTP200);
				_rep->body(_text);
				_rep << "Content-Type" << "application/json" << "Content-Length" << _text.length();
			},
			//head
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				zapata::JSONObj _params;
				zapata::fromparams(_req, _params, zapata::RESTfulDocument);

				zapata::JSONObj _out;
				try {
					zapata::torestdocument(_conn, _collection, _params, _out);
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}
				_conn->done();
				delete _conn;

				assertz(_out->size() != 0, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

				string _text;
				zapata::tostr(_text, _out);
				_rep->status(zapata::HTTP200);
				_rep << "Content-Type" << "application/json" << "Content-Length" << _text.length();
			},
			//trace
			NULL,
			//options
			NULL,
			//patch
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				string _body = _req->body();
				assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

				string _content_type = _req->header("Content-Type");
				assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

				zapata::JSONObj _record;
				zapata::fromstr(_body, _record);

				assertz(((string ) _record["confirmation_password"]) == ((string ) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

				mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) _config["zapata"]["mongodb"]["address"]);
				string _collection((string) _config["zapata"]["mongodb"]["db"]);
				_collection.insert(_collection.length(), "." + ((string) _config["zapata_users"]["mongodb"]["collection"]));

				string __id(_req->url());

				bool _exists = false;
				try {
					mongo::BSONObjBuilder _id_bo;
					_id_bo.append("_id", __id);

					mongo::BSONObjBuilder _bo;
					zapata::tomongo(_record, _bo);

					(*_conn)->update(_collection, _id_bo.obj(), BSON("$set" << _bo.obj()));

					_exists = (*_conn)->getLastError().length() == 0;
				}
				catch (mongo::exception& _e) {
					_conn->done();
					delete _conn;
					assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
				}

				_conn->done();
				delete _conn;

				assertz(_exists, "The requested user was not found", zapata::HTTP404, zapata::ERRUserNotFound);

				zapata::JSONObj _rep_body;
				_rep_body << "href" << __id;
				string _text;
				zapata::tostr(_text, _rep_body);
				_rep->status(zapata::HTTP200);
				_rep << "Cache-Control" << "no-store" << "Pragma" << "no-cache" << "Location" << __id << "Content-Length" << (long) _text.length();
				_rep->body(_text);
			},
			//connect
			NULL, zapata::RESTfulCollection);
	}

	//-------------------------------------------------------------------------------------------------------------------------------------------------
	// LOGIN AND AUTHENTICATION

	/*
	 *  collect OAuth2.0 server authentication code (for token exchange)
	 *  registered as a Resource
	 */
	{
		vector<zapata::HTTPMethod> _ets = { zapata::HTTPGet, zapata::HTTPPost };
		_pool.on(_ets, "^/auth/collect", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
			assertz(_req->param("code").length() != 0, "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
			assertz(_req->param("state").length() != 0, "Parameter 'state' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

			string _code(_req->param("code"));
			string _state(_req->param("state"));
			zapata::base64url_decode(_state);

			size_t _idx = _state.find("||");
			string _redirect_uri(_state.substr(_idx + 2));
			string _client_code(_state.substr(0, _idx));

			assertz(!!_config["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
			zapata::JSONObj _client_config(_config["zapata"]["auth"]["clients"][_client_code]);

			string _client_id(_client_config["client_id"]);
			string _client_secret(_client_config["client_secret"]);
			string _type(_client_config["type"]);

			assertz(!!_config["zapata"]["auth"]["endpoints"][_type], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
			zapata::JSONObj _endpoint_config(_config["zapata"]["auth"]["endpoints"][_type]);

			string _auth_endpoint(_endpoint_config["token"]);
			if (_auth_endpoint.find("?") != string::npos) {
				_auth_endpoint.insert(_auth_endpoint.length(), "&");
			}
			else {
				_auth_endpoint.insert(_auth_endpoint.length(), "?");
			}

			_auth_endpoint.insert(_auth_endpoint.length(), "client_id=");
			_auth_endpoint.insert(_auth_endpoint.length(), _client_id);

			_auth_endpoint.insert(_auth_endpoint.length(), "&client_secret=");
			_auth_endpoint.insert(_auth_endpoint.length(), _client_secret);

			_auth_endpoint.insert(_auth_endpoint.length(), "&code=");
			_auth_endpoint.insert(_auth_endpoint.length(), _code);

			zapata::HTTPReq _token_req;
			zapata::HTTPRep _token_rep;
			_token_req->url(_auth_endpoint);
			zapata::init(_token_req);
			_pool.trigger(_token_req, _token_rep, true);

			string _access_token;
			if (_redirect_uri.find("?") != string::npos) {
				_redirect_uri.insert(_redirect_uri.length(), "&");
			}
			else {
				_redirect_uri.insert(_redirect_uri.length(), "?");
			}
			_redirect_uri.insert(_redirect_uri.length(), "access_token=");
			_redirect_uri.insert(_redirect_uri.length(), _access_token);

			_rep->status(_req->method() == zapata::HTTPGet ? zapata::HTTP307 : zapata::HTTP303);
			_req << "Location" << _redirect_uri;
		}, zapata::RESTfulResource);
	}

	/*
	 *  connect to an OAuth2.0 server
	 *  registered as a Resource
	 */
	{
		vector<zapata::HTTPMethod> _ets = { zapata::HTTPGet, zapata::HTTPPost };
		_pool.on(_ets, "^/auth/connect", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
			assertz(_req->param("client_code").length() != 0, "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

			string _grant_type("code");
			string _client_code(_req->param("client_code"));
			string _redirect_uri(_req->param("redirect_uri"));

			assertz(!!_config["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
			zapata::JSONObj _client_config(_config["zapata"]["auth"]["clients"][_client_code]);

			string _client_id(_client_config["client_id"]);
			string _client_secret(_client_config["client_secret"]);
			string _type(_client_config["type"]);

			assertz(!!_config["zapata"]["auth"]["endpoints"][_type], "No such endpoint type found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
			zapata::JSONObj _endpoint_config(_config["zapata"]["auth"]["endpoints"][_type]);

			string _auth_endpoint(_endpoint_config["authorization"]);
			if (_auth_endpoint.find("?") != string::npos) {
				_auth_endpoint.insert(_auth_endpoint.length(), "&");
			}
			else {
				_auth_endpoint.insert(_auth_endpoint.length(), "?");
			}

			_auth_endpoint.insert(_auth_endpoint.length(), "client_id=");
			_auth_endpoint.insert(_auth_endpoint.length(), _client_id);

			_auth_endpoint.insert(_auth_endpoint.length(), "&client_secret=");
			_auth_endpoint.insert(_auth_endpoint.length(), _client_secret);

			_auth_endpoint.insert(_auth_endpoint.length(), "&grant_type=");
			_auth_endpoint.insert(_auth_endpoint.length(), _grant_type);

			string _self_uri(_config["zapata"]["rest"]["rest"]["bind_url"]);
			_self_uri.insert(_self_uri.length(), "/auth/collect");
			zapata::url_encode(_self_uri);
			_auth_endpoint.insert(_auth_endpoint.length(), "&redirect_uri=");
			_auth_endpoint.insert(_auth_endpoint.length(), _self_uri);

			if (_redirect_uri.length() != 0) {
				_auth_endpoint.insert(_auth_endpoint.length(), "&state=");
				_redirect_uri.insert(0, "||");
				_redirect_uri.insert(0, _client_code);
				zapata::base64url_encode(_redirect_uri);
				_auth_endpoint.insert(_auth_endpoint.length(), _redirect_uri);
			}

			_rep->status(_req->method() == zapata::HTTPGet ? zapata::HTTP307 : zapata::HTTP303);
			_req << "Location" << _auth_endpoint;
		}, zapata::RESTfulResource);
	}

	/*
	 *  exchange the OAuth2.0 server authentication code by an authentication token
	 *  registered as a Resource
	 */
	{
		_pool.on("^/auth/token",
			//get
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				string _grant_type(_req->param("grant_type"));
				assertz(_req->param("grant_type").length() != 0, "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_req->param("client_id").length() != 0, "Parameter 'client_id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_req->param("client_secret").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_req->param("code").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_grant_type == string("user_code") || _req->param("redirect_uri").length() != 0, "Parameter 'redirect_uri' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

				zapata::JSONObj _token;
				bool _has_token = (_grant_type == string("user_code") && zapata::usrtoken(_req->param("client_id"), _req->param("client_secret"), _req->param("code"), _token, _config)) || (_grant_type == string("autorization_code") && zapata::apptoken(_req->param("client_id"), _req->param("client_secret"), _req->param("code"), _token, _config));

				string _redirect_uri(_req->param("redirect_uri"));
				if (_redirect_uri.length() != 0) {
					if (_redirect_uri.find("?") != string::npos) {
						_redirect_uri.insert(_redirect_uri.length(), "&");
					}
					else {
						_redirect_uri.insert(_redirect_uri.length(), "?");
					}

					if (!_has_token) {
						_redirect_uri.insert(_redirect_uri.length(), "error=unauthorized code");
					}
					else {
						_redirect_uri.insert(_redirect_uri.length(), "access_token=");
						_redirect_uri.insert(_redirect_uri.length(), _token["access_token"]);
					}
					_rep->status(zapata::HTTP307);
					_req << "Location" << _redirect_uri;
				}
				else {
					assertz(_has_token, "Unauthorized code", zapata::HTTP401, zapata::ERRGeneric);

					string _body;
					zapata::tostr(_body, _token);
					_rep->status(zapata::HTTP200);
					_rep << "Content-Type" << "application/json" << "Content-Length" << (size_t) _body.length();
					_rep->body(_body);
				}
			},
			//put
			NULL,
			//post
			[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
				string _body = _req->body();
				assertz(_body.length() != 0, "Body ENTITY must be provided", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

				bool _is_json = _req->header("Content-Type").find("application/json") != string::npos;
				bool _is_form_encoded = _req->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos;
				assertz(_is_json || _is_form_encoded, "Body ENTITY must be provided either in JSON or Form URL encoded format", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

				zapata::JSONObj _params;
				if (_is_json) {
					zapata::fromstr(_body, _params);
				}
				else {
					zapata::fromformurlencoded(_body, _params);
				}

				string _grant_type((string) _params["grant_type"]);
				assertz(!!_params["grant_type"], "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(!!_params["client_id"], "Parameter 'client_id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(!!_params["client_secret"], "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(!!_params["code"], "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_grant_type == string("user_code") || !!_params["redirect_uri"], "Parameter 'redirect_uri' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

				zapata::JSONObj _token;

				bool _has_token = (_grant_type == string("user_code") && zapata::usrtoken((string) _params["client_id"], (string) _params["client_secret"], (string) _params["code"], _token, _config)) || (_grant_type == string("autorization_code") && zapata::apptoken((string) _params["client_id"], (string) _params["client_secret"], (string) _params["code"], _token, _config));

				string _redirect_uri(_req->param("redirect_uri"));
				if (_redirect_uri.length() != 0) {
					if (_redirect_uri.find("?") != string::npos) {
						_redirect_uri.insert(_redirect_uri.length(), "&");
					}
					else {
						_redirect_uri.insert(_redirect_uri.length(), "?");
					}

					if (!_has_token) {
						_redirect_uri.insert(_redirect_uri.length(), "error=unauthorized code");
					}
					else {
						_redirect_uri.insert(_redirect_uri.length(), "access_token=");
						_redirect_uri.insert(_redirect_uri.length(), _token["access_token"]);
					}
					_rep->status(zapata::HTTP303);
					_req << "Location" << _redirect_uri;
				}
				else {
					assertz(_has_token, "Unauthorized code", zapata::HTTP401, zapata::ERRGeneric);

					string _body;
					zapata::tostr(_body, _token);
					_rep->status(zapata::HTTP200);
					_rep << "Content-Type" << "application/json" << "Content-Length" << (size_t) _body.length();
					_rep->body(_body);
				}
			},
			//delete
			NULL,
			//head
			NULL,
			//trace
			NULL,
			//options
			NULL,
			//patch
			NULL,
			//connect
			NULL, zapata::RESTfulResource);
	}

	/*
	 *  login to this backend and return an OAuth2.0 server authentication token
	 *  registered as a Contoller
	 */
	{
		_pool.on(zapata::HTTPPost, "^/auth/login", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
			string _body = _req->body();
			assertz(_body.length() != 0, "Body ENTITY must be provided", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

			bool _is_json = _req->header("Content-Type").find("application/json") != string::npos;
			bool _is_form_encoded = _req->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos;
			assertz(_is_json || _is_form_encoded, "Body ENTITY must be provided either in JSON or Form URL encoded format", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);


			zapata::JSONObj _params;
			if (_is_json) {
				zapata::fromstr(_body, _params);
			}
			else {
				zapata::fromformurlencoded(_body, _params);
			}

			assertz(!!_params["id"], "Parameter 'id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
			assertz(!!_params["secret"], "Parameter 'secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
			assertz(!!_params["client_code"], "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

			string _client_code(_params["client_code"]);
			assertz(!!_config["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
			zapata::JSONObj _client_config(_config["zapata"]["auth"]["clients"][_client_code]);

			string _type(_client_config["type"]);
			assertz(!!_config["zapata"]["auth"]["endpoints"][_type], "No such endpoint type found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
			zapata::JSONObj _endpoint_config(_config["zapata"]["auth"]["endpoints"][_type]);

			string _code;
			if (zapata::authenticate((string) _params["id"], (string) _params["secret"], _code, _config)) {
				string _redirect_uri(_req->param("redirect_uri"));

				string _redirect(_endpoint_config["token"]);
				if (_redirect_uri.length() != 0) {
					_redirect.insert(_redirect.length(), "?code=");
					_redirect.insert(_redirect.length(), _code);

					zapata::url_encode(_redirect_uri);
					_redirect.insert(_redirect.length(), "redirect_uri=");
					_redirect.insert(_redirect.length(), _redirect_uri);

					_rep->status(zapata::HTTP303);
					_req << "Location" << _redirect;
				}
				else {
					string _token_body_s;
					zapata::JSONObj _token_body;
					_token_body << "grant_type" << "user_code" << "client_id" << (string) _params["id"] << "client_secret" << (string) _params["secret"] << "code" << _code;
					zapata::tostr(_token_body_s, _token_body);

					//_redirect.insert(0, _config["zapata"]["rest"]["bind_url"]);

					zapata::HTTPReq _token_req;
					_token_req << "Content-Length" << (long) _token_body_s.length() << "Content-Type" << "application/json";
					_token_req->method(zapata::HTTPPost);
					_token_req->body(_token_body_s);

					zapata::HTTPRep _token_rep;
					_pool.trigger(_redirect, _token_req, _token_rep);

					string _token = _token_rep->body();

					_rep->status(zapata::HTTP200);
					_rep << "Content-Type" << "application/json" << "Content-Length" << (long) _token.length();
					_rep->body(_token);
				}
			}
		}, zapata::RESTfulController);
	}
}
