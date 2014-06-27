#include <api/RESTPool.h>
#include <resource/UserLogin.h>
#include <resource/UsersCollection.h>
#include <resource/UsersDocument.h>

extern "C" void populate(zapata::RESTPool& _pool) {
	_pool.add(new zapata::UserLogin());
	_pool.add(new zapata::UsersCollection());
	_pool.add(new zapata::UsersDocument());
}
