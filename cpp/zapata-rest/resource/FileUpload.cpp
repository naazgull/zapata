#include <resource/FileUpload.h>

zapata::FileUpload::FileUpload() : zapata::RESTController("^/file/upload$") {
}

zapata::FileUpload::~FileUpload() {
}

void zapata::FileUpload::post(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	string _body = _req->body();
	assert(_body.length() != 0, "Body entity must be provided.", zapata::HTTP412);

	assert(_req->header("Content-Type").find("application/json") != string::npos, "Body entity must be 'application/json'", zapata::HTTP406);

	zapata::JSONObj _params;
	zapata::fromstr(_body, _params);

	assert(!!_params["uploaded_file"], "The 'uploaded_file' parameter must be provided.", zapata::HTTP412);

	string _cmd("sudo chmod a+rw ");
	_cmd.insert(_cmd.length(), (string) _params["uploaded_file"]);
	_cmd.insert(_cmd.length(), " > /dev/null");
	assert(system(_cmd.data()) == 0, "There was an error giving permissions to the uploaded file.", zapata::HTTP500);

	string _mime;
	zapata::get_mime((string) _params["uploaded_file"], _mime);
	_mime.assign(_mime.substr(_mime.find("/") + 1));

	string _to((string) this->configuration()["zapata"]["rest"]["uploads"]["upload_path"]);
	zapata::normalize_path(_to, true);

	string _path;
	string _name;
	do {
		_path.assign("");
		zapata::generate_key(_path);
		_path.insert(_path.length(), ".");
		_path.insert(_path.length(), _mime);

		_name.assign(_path);

		_path.insert(0, _to);
	}
	while(zapata::path_exists(_path));

	assert(zapata::move_path((string) _params["uploaded_file"], _path), "There was an error moving the uploaded file.", zapata::HTTP500);

	string _location((string) this->configuration()["zapata"]["rest"]["uploads"]["upload_url"]);
	zapata::normalize_path(_location, true);
	_location.insert(_location.length(), _name);

	_rep->status(zapata::HTTP201);
	_rep << "Location" << _location;
}
