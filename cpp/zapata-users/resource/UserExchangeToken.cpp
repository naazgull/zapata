/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <base/smart_ptr.h>
#include <db/convert_mongo.h>
#include <json/JSONObj.h>
#include <mongo/base/string_data.h>
#include <mongo/bson/bsonmisc.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <mongo/client/connpool.h>
#include <mongo/client/dbclientcursor.h>
#include <mongo/client/dbclientinterface.h>
#include <resource/UserExchangeToken.h>
#include <text/convert.h>
#include <text/manip.h>
#include <ctime>
#include <memory>
#include <string>

zapata::UserExchangeToken::UserExchangeToken() {
}

zapata::UserExchangeToken::~UserExchangeToken() {
}

bool zapata::UserExchangeToken::usrtoken(string _id, string _secret, string _code, zapata::JSONObj& _out) {
	string _out_token;
	string _cur_date;
	zapata::tostr(_cur_date, time(NULL));
	_out_token.insert(_out_token.length(), _cur_date);
	_out_token.insert(_out_token.length(), "|");
	_out_token.insert(_out_token.length(), _id);
	_out_token.insert(_out_token.length(), "|");

	mongo::ScopedDbConnection* _conn = mongo::ScopedDbConnection::getScopedDbConnection((string) this->configuration()["zapata"]["mongodb"]["address"]);
	string _collection((string) this->configuration()["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), "." + ((string) this->configuration()["zapata_users"]["mongodb"]["collection"]));

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "password" << _secret));
	if(_ptr->more()) {
		mongo::BSONObj _bo = _ptr->next();
		zapata::frommongo(_bo, _out);
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

	string _enc_token;
	zapata::encrypt(_enc_token, _out_token, (string) this->configuration()["zapata"]["auth"]["signing_key"]);

	_out >> "password" >> "confirmation_password";
	_out << "access_token" << _enc_token;

	return true;
}

bool zapata::UserExchangeToken::apptoken(string _id, string _secret, string _code, zapata::JSONObj& _out) {
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

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "password" << _secret));
	if(_ptr->more()) {
		mongo::BSONObj _bo = _ptr->next();
		zapata::frommongo(_bo, _out);
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

	string _enc_token;
	zapata::encrypt(_enc_token, _out_token, (string) this->configuration()["zapata"]["auth"]["signing_key"]);

	_out >> "password" >> "confirmation_password";
	_out << "access_token" << _enc_token;

	return true;
}
