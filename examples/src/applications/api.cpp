#include <zapata/rest.h>
#include <string>
#include <zapata/applications/collections/applications.h>
#include <zapata/applications/documents/applications.h>
#include <zapata/applications/collections/channels.h>


using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

extern "C" void restify(zpt::ev::emitter _emitter) {
_emitter->connector({ { "dbms.mongodb.zpt.apps", zpt::connector(new zpt::mongodb::Client(_emitter->options(), "mongodb.zpt.apps")) }, { "dbms.pgsql.zpt.apps", zpt::connector(new zpt::pgsql::Client(_emitter->options(), "pgsql.zpt.apps")) }, { "dbms.redis.zpt.apps", zpt::connector(new zpt::redis::Client(_emitter->options(), "redis.zpt.apps")) }, });

zpt::apps::collections::applications::restify(_emitter);
zpt::apps::documents::applications::restify(_emitter);
zpt::apps::collections::channels::restify(_emitter);

}

