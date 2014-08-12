
# Registering all handlers for a collection

	/*
	 *  definition of handlers for the '/users' collection
	 *  registered as a Collection
	 */
	{
		_pool.on("^/name$",
		                    //get
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //put
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //post
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //delete
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //head
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
		                    },
		                    //trace
		                    NULL,
		                    //options
		                    NULL,
		                    //patch
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //connect
		                    NULL,
		                    zapata::RESTfulCollection);
	}
	
# Registering all handlers for a document

	/*
	 *  definition of handlers for the '/users/{id}' document
	 *  registered as a Document
	 */
	{
		_pool.on("^/name/([^/]+)$",
		                    //get
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //put
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //post
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //delete
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //head
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
		                    },
		                    //trace
		                    NULL,
		                    //options
		                    NULL,
		                    //patch
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //connect
		                    NULL,
		                    zapata::RESTfulDocument);
	}
