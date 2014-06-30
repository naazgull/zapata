#include <json/JSONObj.h>
#include <mongo/client/dbclient.h>
#include <mongo/base/string_data.h>
#include <mongo/bson/bson-inl.h>
#include <mongo/bson/bsonmisc.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/connpool.h>
#include <mongo/client/dbclientcursor.h>
#include <mongo/client/dbclientinterface.h>
#include <resource/UserExchangeToken.h>
#include <text/convert.h>
#include <ctime>
#include <memory>
#include <string>

zapata::UserExchangeToken::UserExchangeToken() {
}

zapata::UserExchangeToken::~UserExchangeToken() {
}

bool zapata::UserExchangeToken::usrtoken(string _id, string _secret, string _code, string& _out) {
	string _out_token;
	string _cur_date;
	zapata::tostr(_cur_date, time(NULL));
	_out_token.insert(_out_token.length(), _cur_date);
	_out_token.insert(_out_token.length(), "|");
	_out_token.insert(_out_token.length(), _id);
	_out_token.insert(_out_token.length(), "|");

	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), this->configuration()["zapata_users"]["mongodb"]["collection"]);

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "secret" << _secret));
	if(_ptr->more()) {
		mongo::BSONObj _bo = _ptr->next();
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

	zapata::encrypt(_out, _out_token, (string) this->configuration()["zapata"]["auth"]["signing_key"]);

	return true;
}

bool zapata::UserExchangeToken::apptoken(string _id, string _secret, string _code, string& _out) {
	string _out_token;
	string _cur_date;
	zapata::tostr(_cur_date, time(NULL));
	_out_token.insert(_out_token.length(), _cur_date);
	_out_token.insert(_out_token.length(), "|");
	_out_token.insert(_out_token.length(), _id);
	_out_token.insert(_out_token.length(), "|");

	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), this->configuration()["zapata_users"]["mongodb"]["collection"]);

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "secret" << _secret));
	if(_ptr->more()) {
		mongo::BSONObj _bo = _ptr->next();
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

	zapata::encrypt(_out, _out_token, (string) this->configuration()["zapata"]["auth"]["signing_key"]);

	return true;
}
