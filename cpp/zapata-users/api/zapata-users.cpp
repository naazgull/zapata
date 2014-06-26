#include <zapata/rest.h>
#include <zapata/users.h>

extern "C" void populate(zapata::RESTPool& _pool) {
	_pool.add(new zapata::UserLogin());
	_pool.add(new zapata::UsersCollection());
}
