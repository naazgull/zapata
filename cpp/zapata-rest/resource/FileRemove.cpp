/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <api/codes_rest.h>
#include <base/assert.h>
#include <exceptions/AssertionException.h>
#include <file/manip.h>
#include <http/HTTPObj.h>
#include <json/JSONObj.h>
#include <parsers/json.h>
#include <resource/FileRemove.h>
#include <text/manip.h>
#include <cstdlib>
#include <string>

zapata::FileRemove::FileRemove() : zapata::RESTController("^/file/remove$") {
}

zapata::FileRemove::~FileRemove() {
}

void zapata::FileRemove::post(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	string _body = _req->body();
	assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

	assertz(_req->header("Content-Type").find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

	zapata::JSONObj _params;
	zapata::fromstr(_body, _params);

	assertz(!!_params["file_url"], "The 'file_url' parameter must be provided.", zapata::HTTP412, zapata::ERRRequiredField);

	string _url((string) _params["file_url"]);
	string _url_root(this->configuration()["zapata"]["rest"]["uploads"]["upload_url"]);
	string _path_root(this->configuration()["zapata"]["rest"]["uploads"]["upload_path"]);

	zapata::replace(_url, _url_root, _path_root);

	assertz(zapata::path_exists(_url), "Couldn't find the designated file", zapata::HTTP404, zapata::ERRFileNotFound);

	_url.assign(_url.substr(0, _url.rfind("/")));

	string _cmd("rm -rf ");
	_cmd.insert(_cmd.length(), _url);
	_cmd.insert(_cmd.length(), " > /dev/null");
	assertz(system(_cmd.data()) == 0, "There was an error removing the uploaded file.", zapata::HTTP500, zapata::ERRFilePermissions);

	_rep->status(zapata::HTTP200);
}
