#include <json/JSONObj.h>
#include <mongo/client/dbclient.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/connpool.h>
#include <mongo/client/dbclientcursor.h>
#include <mongo/client/dbclientinterface.h>
#include <resource/UserLogin.h>
#include <memory>
#include <string>

zapata::UserLogin::UserLogin() {
}

zapata::UserLogin::~UserLogin() {
}

bool zapata::UserLogin::authenticate(string _id, string _secret, string& _out_code) {
	bool _exists = false;

	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), ".z_users");

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "secret" << _secret));
	_exists = _ptr->more();
	_ptr->decouple();
	 (*_conn)->killCursor(_ptr->getCursorId());
	 delete _ptr.release();

	_conn->done();
	delete _conn;

	return _exists;
}
