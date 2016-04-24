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
#include <zapata/rest.h>
#include <zapata/mongodb.h>
#include <zapata/users/codes_users.h>
#include <ctime>
#include <memory>

namespace zapata {

	namespace users {

		void collection(zapata::RESTPoolPtr& _pool) {
			zapata::RESTHandler _handler_set[9] = {
				//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);
					zapata::mongodb::CollectionPtr _mongo((zapata::mongodb::Collection *) _pool->get_kb("mongodb.users").get());
					zapata::JSONPtr _payload = _mongo->query("users", _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->body(_text);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_put,
				//post
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONPtr _record = zapata::fromstr(_body);
					if (!_record["id"]->ok() && _record["email"]->ok()) {
						_record << "id" << (string) _record["email"];
					}
					if (!_record["email"]->ok() && _record["id"]->ok()) {
						string _email((string) _record["id"]);
						_email.insert(_email.length(), "@");
						_email.insert(_email.length(), (string) _config["rest"]["bind_address"]);
						_record << "email" << _email;
					}

					assertz(_record["fullname"]->ok(), "The 'name' field is mandatory", zapata::HTTP412, zapata::ERRNameMandatory);
					assertz(_record["id"]->ok(), "The 'id' field is mandatory", zapata::HTTP412, zapata::ERRIDMandatory);
					assertz(_record["email"]->ok(), "The 'email' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					assertz(_record["password"]->ok(), "The 'password' field is mandatory", zapata::HTTP412, zapata::ERRPasswordMandatory);
					assertz(_record["confirmation_password"]->ok(), "The 'confirmation_password' field is mandatory", zapata::HTTP412, zapata::ERRConfirmationMandatory);
					assertz(((string ) _record["confirmation_password"]) == ((string ) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

					zapata::mongodb::CollectionPtr _mongo((zapata::mongodb::Collection *) _pool->get_kb("mongodb.users").get());
					zapata::JSONPtr _payload = _mongo->insert("users", _req->url(), _record);
					string _text = (string) _payload;
					_rep->status(zapata::HTTP201);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Location", (string) _payload["href"]);
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_delete,
				//head
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);
					zapata::mongodb::CollectionPtr _mongo((zapata::mongodb::Collection *) _pool->get_kb("mongodb.users").get());
					zapata::JSONPtr _payload = _mongo->query("users", _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_trace,
				no_options, 
				no_patch,
				no_connect
			};
			_pool->on("^/users$", _handler_set);
		}

		void document(zapata::RESTPoolPtr& _pool) {
			zapata::RESTHandler _handler_set[9] = {
			//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);
					zapata::mongodb::CollectionPtr _mongo((zapata::mongodb::Collection *) _pool->get_kb("mongodb.users").get());
					zapata::JSONPtr _payload = _mongo->query("users", _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->body(_text);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
			//put
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONPtr _record = zapata::fromstr(_body);

					assertz(_record["fullname"]->ok(), "The 'name' field is mandatory", zapata::HTTP412, zapata::ERRNameMandatory);
					assertz(_record["id"]->ok(), "The 'id' field is mandatory", zapata::HTTP412, zapata::ERRIDMandatory);
					assertz(_record["email"]->ok(), "The 'email' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					assertz(_record["password"]->ok(), "The 'password' field is mandatory", zapata::HTTP412, zapata::ERRPasswordMandatory);
					assertz(_record["confirmation_password"]->ok(), "The 'confirmation_password' field is mandatory", zapata::HTTP412, zapata::ERRConfirmationMandatory);
					assertz(((string ) _record["confirmation_password"]) == ((string ) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::replace_document(_config, (string) _config["users"]["mongodb"]["collection"], _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//post
				no_post,
			//delete
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::delete_document(_config, (string) _config["users"]["mongodb"]["collection"], _params);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//head
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					zapata::JSONPtr _payload = zapata::mongodb::get_document(_config, (string) _config["users"]["mongodb"]["collection"], _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_trace, 
				no_options, 
			//patch
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONPtr _record = zapata::fromstr(_body);

					assertz(!_record["id"]->ok(), "The 'id' field is mandatory", zapata::HTTP412, zapata::ERRIDMandatory);
					assertz((!_record["password"]->ok() && !_record["confirmation_password"]->ok()) || (_record["password"]->ok() && _record["confirmation_password"]->ok()), "The 'confirmation_password' field is mandatory when the field 'password' is provided", zapata::HTTP412, zapata::ERRConfirmationMandatory);
					assertz(((string) _record["confirmation_password"]) == ((string) _record["password"]), "The 'password' and 'confirmation_password' fields don't match", zapata::HTTP412, zapata::ERRPasswordConfirmationDontMatch);

					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::patch_document(_config, (string) _config["users"]["mongodb"]["collection"], _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_connect
			};
			_pool->on("^/users/([^/]+)$", _handler_set);
		}

		void collect(zapata::RESTPoolPtr& _pool) {
			vector<zapata::HTTPMethod> _ets = { zapata::HTTPGet, zapata::HTTPPost };
			_pool->on(_ets, "^/auth/collect", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
				assertz(_req->param("code").length() != 0, "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_req->param("state").length() != 0, "Parameter 'state' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

				string _code(_req->param("code"));
				string _state(_req->param("state"));
				zapata::base64::url_decode(_state);

				size_t _idx = _state.find("||");
				string _redirect_uri(_state.substr(_idx + 2));
				string _client_code(_state.substr(0, _idx));

				assertz(_config["auth"]["clients"][_client_code]->ok(), "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
				zapata::JSONObj _client_config(_config["auth"]["clients"][_client_code]);

				string _client_id(_client_config["client_id"]);
				string _client_secret(_client_config["client_secret"]);
				string _type(_client_config["type"]);

				assertz(_config["auth"]["endpoints"][_type]->ok(), "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
				zapata::JSONObj _endpoint_config(_config["auth"]["endpoints"][_type]);

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
				_pool->trigger(_token_req, _token_rep, true);

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
				_rep->header("Location", _redirect_uri);
			});
		}
	
		void connect(zapata::RESTPoolPtr& _pool) {
			vector<zapata::HTTPMethod> _ets = { zapata::HTTPGet, zapata::HTTPPost };
			_pool->on(_ets, "^/auth/connect", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
				assertz(_req->param("client_code").length() != 0, "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

				string _grant_type("code");
				string _client_code(_req->param("client_code"));
				string _redirect_uri(_req->param("redirect_uri"));

				assertz(_config["auth"]["clients"][_client_code]->ok(), "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
				zapata::JSONObj _client_config(_config["auth"]["clients"][_client_code]);

				string _client_id(_client_config["client_id"]);
				string _client_secret(_client_config["client_secret"]);
				string _type(_client_config["type"]);

				assertz(_config["auth"]["endpoints"][_type]->ok(), "No such endpoint type found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
				zapata::JSONObj _endpoint_config(_config["auth"]["endpoints"][_type]);

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

				string _self_uri(_config["rest"]["rest"]["bind_url"]);
				_self_uri.insert(_self_uri.length(), "/auth/collect");
				zapata::url::encode(_self_uri);
				_auth_endpoint.insert(_auth_endpoint.length(), "&redirect_uri=");
				_auth_endpoint.insert(_auth_endpoint.length(), _self_uri);

				if (_redirect_uri.length() != 0) {
					_auth_endpoint.insert(_auth_endpoint.length(), "&state=");
					_redirect_uri.insert(0, "||");
					_redirect_uri.insert(0, _client_code);
					zapata::base64::url_encode(_redirect_uri);
					_auth_endpoint.insert(_auth_endpoint.length(), _redirect_uri);
				}

				_rep->status(_req->method() == zapata::HTTPGet ? zapata::HTTP307 : zapata::HTTP303);
				_rep->header("Location", _auth_endpoint);
			});			
		}

		void token(zapata::RESTPoolPtr& _pool) {
			zapata::RESTHandler _handler_set[9] = {
			//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _grant_type(_req->param("grant_type"));
					assertz(_req->param("grant_type").length() != 0, "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_req->param("client_id").length() != 0, "Parameter 'client_id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_req->param("client_secret").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_req->param("code").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_grant_type == string("user_code") || _req->param("redirect_uri").length() != 0, "Parameter 'redirect_uri' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

					zapata::JSONObj _token;
					bool _has_token = (_grant_type == string("user_code") && zapata::auth::usrtoken(_req->param("client_id"), _req->param("client_secret"), _req->param("code"), _token, _config)) || (_grant_type == string("autorization_code") && zapata::auth::apptoken(_req->param("client_id"), _req->param("client_secret"), _req->param("code"), _token, _config));

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
						_rep->header("Location", _redirect_uri);
					}
					else {
						assertz(_has_token, "Unauthorized code", zapata::HTTP401, zapata::ERRGeneric);

						string _body;
						zapata::tostr(_body, _token);
						_rep->status(zapata::HTTP200);
						_rep->header("Content-Type", "application/json"); 
						_rep->header("Content-Length", std::to_string(_body.length()));
						_rep->body(_body);
					}
				},
				no_put,
			//post
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body ENTITY must be provided", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					bool _is_json = _req->header("Content-Type").find("application/json") != string::npos;
					bool _is_form_encoded = _req->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos;
					assertz(_is_json || _is_form_encoded, "Body ENTITY must be provided either in JSON or Form URL encoded format", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONObj _params;
					if (_is_json) {
						_params = (zapata::JSONObj&) zapata::fromstr(_body);
					}
					else {
						zapata::fromformurlencoded(_body, _params);
					}

					string _grant_type((string) _params["grant_type"]);
					assertz(_params["grant_type"]->ok(), "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_params["client_id"]->ok(), "Parameter 'client_id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_params["client_secret"]->ok(), "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_params["code"]->ok(), "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
					assertz(_grant_type == string("user_code") || !!_params["redirect_uri"], "Parameter 'redirect_uri' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

					zapata::JSONObj _token;

					bool _has_token = (_grant_type == string("user_code") && zapata::auth::usrtoken((string) _params["client_id"], (string) _params["client_secret"], (string) _params["code"], _token, _config)) || (_grant_type == string("autorization_code") && zapata::auth::apptoken((string) _params["client_id"], (string) _params["client_secret"], (string) _params["code"], _token, _config));

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
						_rep->header("Location", _redirect_uri);
					}
					else {
						assertz(_has_token, "Unauthorized code", zapata::HTTP401, zapata::ERRGeneric);

						string _body;
						zapata::tostr(_body, _token);
						_rep->status(zapata::HTTP200);
						_rep->header("Content-Type", "application/json"); 
						_rep->header("Content-Length", std::to_string(_body.length()));
						_rep->header("Content-Type", "application/json"); 
						_rep->body(_body);
					}
				},
				no_delete, 
				no_head, 
				no_trace, 
				no_options, 
				no_patch, 
				no_connect
			};
			_pool->on("^/auth/token", _handler_set);			
		}

		void login(zapata::RESTPoolPtr& _pool) {
			_pool->on(zapata::HTTPPost, "^/auth/login", [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
				string _body = _req->body();			
				assertz(_body.length() != 0, "Body ENTITY must be provided", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

				bool _is_json = _req->header("Content-Type").find("application/json") != string::npos;
				bool _is_form_encoded = _req->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos;
				assertz(_is_json || _is_form_encoded, "Body ENTITY must be provided either in JSON or Form URL encoded format", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);


				zapata::JSONObj _params;
				if (_is_json) {
					_params = (zapata::JSONObj&) zapata::fromstr(_body);
				}
				else {
					zapata::fromformurlencoded(_body, _params);
				}

				assertz(_params["id"]->ok(), "Parameter 'id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_params["secret"]->ok(), "Parameter 'secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
				assertz(_params["client_code"]->ok(), "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

				string _client_code(_params["client_code"]);
				assertz(_config["auth"]["clients"][_client_code]->ok(), "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
				zapata::JSONObj _client_config(_config["auth"]["clients"][_client_code]);

				string _type(_client_config["type"]);
				assertz(_config["auth"]["endpoints"][_type]->ok(), "No such endpoint type found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
				zapata::JSONObj _endpoint_config(_config["auth"]["endpoints"][_type]);

				string _code;
				if (zapata::auth::authenticate((string) _params["id"], (string) _params["secret"], _code, _config)) {
					string _redirect_uri(_req->param("redirect_uri"));

					string _redirect(_endpoint_config["token"]);
					if (_redirect_uri.length() != 0) {
						_redirect.insert(_redirect.length(), "?code=");
						_redirect.insert(_redirect.length(), _code);

						zapata::url::encode(_redirect_uri);
						_redirect.insert(_redirect.length(), "redirect_uri=");
						_redirect.insert(_redirect.length(), _redirect_uri);

						_rep->status(zapata::HTTP303);
						_rep->header("Location", _redirect);
					}
					else {
						string _token_body_s;
						zapata::JSONObj _token_body;
						_token_body << "grant_type" << "user_code" << "client_id" << (string) _params["id"] << "client_secret" << (string) _params["secret"] << "code" << _code;
						zapata::tostr(_token_body_s, _token_body);

						//_redirect.insert(0, _config["rest"]["bind_url"]);

						zapata::HTTPReq _token_req;
						_token_req->header("Content-Type", "application/json"); 
						_token_req->header("Content-Length", std::to_string(_token_body_s.length()));
						_token_req->method(zapata::HTTPPost);
						_token_req->body(_token_body_s);

						zapata::HTTPRep _token_rep;
						_pool->trigger(_redirect, _token_req, _token_rep);

						string _token = _token_rep->body();

						_rep->status(zapata::HTTP200);
						_rep->header("Content-Type", "application/json"); 
						_rep->header("Content-Length", std::to_string(_token.length()));
						_rep->body(_token);
					}
				}
			});
		}
	}

	namespace groups {

		void collection(zapata::RESTPoolPtr& _pool) {	
			zapata::RESTHandler _handler_set[9] = {
			//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);

					zapata::JSONPtr _payload = zapata::mongodb::get_collection(_config, (string) _config["users"]["mongodb"]["collection"], _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->body(_text);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_put,
			//post
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					zapata::JSONPtr _record = zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONPtr _payload = zapata::mongodb::create_document(_config, (string) _config["users"]["mongodb"]["collection"], _record);
					string _text = (string) _payload;
					_rep->status(zapata::HTTP201);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Location", (string) _payload["href"]);
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//delete
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);

					string _text = (string) zapata::mongodb::delete_from_collection(_config, (string) _config["users"]["mongodb"]["collection"], _params);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//head
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);

					zapata::JSONPtr _payload = zapata::mongodb::get_collection(_config, (string) _config["users"]["mongodb"]["collection"], _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_trace, 
				no_options, 
			//patch
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONPtr _record = zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulCollection, true);

					string _text = (string) zapata::mongodb::patch_from_collection(_config, (string) _config["users"]["mongodb"]["collection"], _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_connect
			};
			_pool->on("^/groups$", _handler_set);
		}

		void document(zapata::RESTPoolPtr& _pool) {	
			zapata::RESTHandler _handler_set[9] = {
			//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					zapata::JSONPtr _payload = zapata::mongodb::get_document(_config, (string) _config["users"]["mongodb"]["collection"], _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->body(_text);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
			//put
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONPtr _record = zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::replace_document(_config, (string) _config["users"]["mongodb"]["collection"], _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_post,
			//delete
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::delete_document(_config, (string) _config["users"]["mongodb"]["collection"], _params);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//head
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					zapata::JSONPtr _payload = zapata::mongodb::get_document(_config, (string) _config["users"]["mongodb"]["collection"], _params);

					string _text = (string) _payload;
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_trace, 
				no_options, 
			//patch
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONPtr _record = zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONPtr _params = zapata::fromparams(_req, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::patch_document(_config, (string) _config["users"]["mongodb"]["collection"], _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_connect
			};
			_pool->on("^/groups/([^/]+)$", _handler_set);
		}
	}
}

extern "C" void restify(zapata::RESTPoolPtr& _pool) {
	zapata::KBPtr _kb(new zapata::mongodb::Collection(_pool->options()["users"]->obj()));
	_pool->add_kb("mongodb.users", _kb);

	zapata::users::collection(_pool);
	zapata::users::document(_pool);
	zapata::users::collect(_pool);
	zapata::users::connect(_pool);
	zapata::users::token(_pool);
	zapata::users::login(_pool);

	zapata::groups::collection(_pool);
	zapata::groups::document(_pool);
}

extern "C" int zapata_users() {
	return 1;
}