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

	string _from((string) _params["uploaded_file"]);

	/*
	 * This block of code is to be removed, since zapata-rest-daemon must
	 * be executed with the same user that apache or nginx
	 */
	{
		string _cmd("sudo chmod a+rw ");
		_cmd.insert(_cmd.length(), _from);
		_cmd.insert(_cmd.length(), " > /dev/null");
		assert(system(_cmd.data()) == 0, "There was an error giving permissions to the uploaded file.", zapata::HTTP500);
	}

	string _encoding(_req->header("X-Content-Transfer-Encoding"));
	transform(_encoding.begin(), _encoding.end(), _encoding.begin(), ::toupper);
	if (_encoding == "BASE64") {
		string _content;
		zapata::load_path(_from, _content);
		zapata::base64_decode(_content);
		zapata::dump_path(_from, _content);
	}

	string _to((string) this->configuration()["zapata"]["rest"]["uploads"]["upload_path"]);
	zapata::normalize_path(_to, true);

	string _originalname(_req->header("X-Original-FIlename"));
	string _path;
	string _name;
	do {
		_path.assign("");
		if (_originalname.length() != 0) {
			string _dir(_to);
			zapata::generate_hash(_path);
			_dir.insert(_dir.length(), _path);
			zapata::mkdir_recursive(_dir, 0777);
			_path.insert(_path.length(), "/");
			_path.insert(_path.length(), _originalname);
		}
		else {
			zapata::MIMEType _mime = zapata::get_mime((string) _params["uploaded_file"]);
			zapata::generate_hash(_path);
			_path.insert(_path.length(), zapata::mimetype_extensions[_mime]);
		}

		_name.assign(_path);
		_path.insert(0, _to);
	}
	while(zapata::path_exists(_path));

	assert(zapata::copy_path((string) _params["uploaded_file"], _path), "There was an error copying the temporary file to the 'upload_path' directory.", zapata::HTTP500);

	string _location((string) this->configuration()["zapata"]["rest"]["uploads"]["upload_url"]);
	zapata::normalize_path(_location, true);
	_location.insert(_location.length(), _name);

	_rep->status(zapata::HTTP201);
	_rep << "Location" << _location;
}
