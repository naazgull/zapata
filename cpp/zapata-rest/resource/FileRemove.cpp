#include <resource/FileRemove.h>

#include <stdio.h>

zapata::FileRemove::FileRemove() : zapata::RESTController("^/file/remove$") {
}

zapata::FileRemove::~FileRemove() {
}

void zapata::FileRemove::post(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	string _body = _req->body();
	assertz(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412);

	assertz(_req->header("Content-Type").find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406);

	zapata::JSONObj _params;
	zapata::fromstr(_body, _params);

	assertz(!!_params["file_url"], "The 'file_url' parameter must be provided.", zapata::HTTP412);

	string _url((string) _params["file_url"]);
	string _url_root(this->configuration()["zapata"]["rest"]["uploads"]["upload_url"]);
	string _path_root(this->configuration()["zapata"]["rest"]["uploads"]["upload_path"]);

	zapata::replace(_url, _url_root, _path_root);

	assertz(zapata::path_exists(_url), "Couldn't find the designated file", zapata::HTTP404);

	_url.assign(_url.substr(0, _url.rfind("/")));

	string _cmd("rm -rf ");
	_cmd.insert(_cmd.length(), _url);
	_cmd.insert(_cmd.length(), " > /dev/null");
	assertz(system(_cmd.data()) == 0, "There was an error removing the uploaded file.", zapata::HTTP500);

	_rep->status(zapata::HTTP200);
}
