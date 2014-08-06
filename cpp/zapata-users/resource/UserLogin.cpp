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
	_collection.insert(_collection.length(), "." + ((string) this->configuration()["zapata_users"]["mongodb"]["collection"]));

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_collection, QUERY("id" << _id << "password" << _secret));
	_exists = _ptr->more();
	_ptr->decouple();
	 (*_conn)->killCursor(_ptr->getCursorId());
	 delete _ptr.release();

	_conn->done();
	delete _conn;

	if (_exists) {
		zapata::generate_hash(_out_code);
	}

	return _exists;
}
