
# Registering all handlers for a collection

	/*
	 *  definition of handlers for the '/users' collection
	 *  registered as a Collection
	 */
	{
		_pool.on("^/name$",
		                    //--- GET ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- PUT ---//
		                    NULL,
		                    //--- POST ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- DELETE ---//
		                    NULL,
		                    //--- HEAD ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
		                    },
		                    //--- TRACE ---//
		                    NULL,
		                    //--- OPTIONS ---//
		                    NULL,
		                    //--- PATCH ---//
		                    NULL,
		                    //--- CONNECT ---//
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
		                    //--- GET ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- PUT ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- POST ---//
		                    NULL,
		                    //--- DELETE ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- HEAD ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
		                    },
		                    //--- TRACE ---//
		                    NULL,
		                    //--- OPTIONS ---//
		                    NULL,
		                    //--- PATCH ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- CONNECT ---//
		                    NULL,
		                    zapata::RESTfulDocument);
	}
	
# Registering all handlers for a store

	/*
	 *  definition of handlers for the '/users' store
	 *  registered as a Store
	 */
	{
		_pool.on("^/name",
		                    //--- GET ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- PUT ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- POST ---//
		                    NULL,
		                    //--- DELETE ---//
		                    NULL,
		                    //--- HEAD ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool _pool) -> void {
		                    },
		                    //--- TRACE ---//
		                    NULL,
		                    //--- OPTIONS ---//
		                    NULL,
		                    //--- PATCH ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- CONNECT ---//
		                    NULL,
		                    zapata::RESTfulStore);
	}
	
# Registering all handlers for a controller

	/*
	 *  definition of handlers for the '/users' controller
	 *  registered as a Controller
	 */
	{
		_pool.on("^/name",
		                    //--- GET ---//
		                    NULL,
		                    //--- PUT ---//
		                    NULL,
		                    //--- POST ---//
		                    [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		                    },
		                    //--- DELETE ---//
		                    NULL,
		                    //--- HEAD ---//
		                    NULL,
		                    //--- TRACE ---//
		                    NULL,
		                    //--- OPTIONS ---//
		                    NULL,
		                    //--- PATCH ---//
		                    NULL,
		                    //--- CONNECT ---//
		                    NULL,
		                    zapata::RESTfulController);
	}
	