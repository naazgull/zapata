/*
The MIT License (MIT)

Copyright (c) 2014 ${author} <${email}>

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
#include <${project_name}/api.h>

#include <zapata/core.h>
#include <zapata/net.h>
#include <zapata/http.h>
#include <zapata/rest.h>
#include <zapata/mongodb.h>

namespace ${project_name} {

	namespace ${collection_name} {

		void collection(zapata::RESTPool& _pool) {	
			_pool.on("^/${collection_name}$",
			//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulCollection, true);

					zapata::JSONPtr _req_body = zapata::mongodb::get_collection(_config, ${mongo_collection_name}, _params);

					string _text = (string) _req_body;
					_rep->status(zapata::HTTP200);
					_rep->body(_text);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_put,
			//post
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					zapata::JSONObj _record = (zapata::JSONObj&) zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONPtr _req_body = zapata::mongodb::create_document(_config, ${mongo_collection_name}, _record);
					string _text = (string) _req_body;
					_rep->status(zapata::HTTP201);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Location", (string) _req_body["href"]);
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//delete
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulCollection, true);

					string _text = (string) zapata::mongodb::delete_from_collection(_config, ${mongo_collection_name}, _params);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//head
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulCollection, true);

					zapata::JSONPtr _req_body = zapata::mongodb::get_collection(_config, ${mongo_collection_name}, _params);

					string _text = (string) _req_body;
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_trace, 
				no_options, 
			//patch
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONObj _record = (zapata::JSONObj&) zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulCollection, true);

					string _text = (string) zapata::mongodb::patch_from_collection(_config, ${mongo_collection_name}, _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_connect
			);
		}

		void document(zapata::RESTPool& _pool) {	
			_pool.on("^/${collection_name}/([^/]+)$",
			//get
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulDocument);

					zapata::JSONPtr _req_body = zapata::mongodb::get_document(_config, ${mongo_collection_name}, _params);

					string _text = (string) _req_body;
					_rep->status(zapata::HTTP200);
					_rep->body(_text);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
			//put
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONObj _record = (zapata::JSONObj&) zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::replace_document(_config, ${mongo_collection_name}, _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_post,
			//delete
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::delete_document(_config, ${mongo_collection_name}, _params);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
			//head
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulDocument);

					zapata::JSONPtr _req_body = zapata::mongodb::get_document(_config, ${mongo_collection_name}, _params);

					string _text = (string) _req_body;
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Type", "application/json"); 
					_rep->header("Content-Length", std::to_string(_text.length()));
				},
				no_trace, 
				no_options, 
			//patch
				[] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr& _options, zapata::RESTPool& _pool) -> void {
					string _body = _req->body();
					assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

					string _content_type = _req->header("Content-Type");
					assertz(_content_type.find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

					zapata::JSONObj _record = (zapata::JSONObj&) zapata::fromstr(_body);

					/**
					 * Put your field validations here, using 'assertz'
					 *
					 * assertz(_record[${field}]->ok(), "The '${field}' field is mandatory", zapata::HTTP412, zapata::ERREmailMandatory);
					 */

					zapata::JSONObj _params;
					zapata::fromparams(_req, _params, zapata::RESTfulDocument);

					string _text = (string) zapata::mongodb::patch_document(_config, ${mongo_collection_name}, _params, _record);
					_rep->status(zapata::HTTP200);
					_rep->header("Cache-Control", "no-store");
					_rep->header("Pragma", "no-cache");
					_rep->header("Content-Length", std::to_string(_text.length()));
					_rep->header("Content-Type", "application/json"); 
					_rep->body(_text);
				},
				no_connect
			);
		}
	}
}

