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

#include <api/RESTPool.h>
#include <resource/UserExchangeToken.h>
#include <resource/UserLogin.h>
#include <resource/UsersCollection.h>
#include <resource/UsersDocument.h>

extern "C" void populate(zapata::RESTPool& _pool) {
	_pool.add(new zapata::UserLogin());
	_pool.add(new zapata::UserExchangeToken());
	_pool.add(new zapata::UsersCollection());
	_pool.add(new zapata::UsersDocument());
}
