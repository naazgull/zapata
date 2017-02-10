#include <zapata/applications/mutations/MyApplications.h>

auto zpt::apps::mutations::MyApplications::mutify(zpt::mutation::emitter _emitter) -> void {
zpt::mutation::callback _h_on_change;
 
_emitter->on("^/MyApplications$",
{
{
zpt::mutation::Insert,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
zpt::json _c_names = { zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps" };
for (auto _c_name : _c_names->arr()) {
zpt::connector _c = _emitter->connector(_c_name->str());
zpt::json _r_data = _envelope["payload"]["new"];
_c->insert("MyApplications", _envelope["payload"]["href"]->str(), _r_data, { "mutated-event", true });
}
}
}

,
{
zpt::mutation::Update,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
zpt::json _c_names = { zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps" };
for (auto _c_name : _c_names->arr()) {
zpt::connector _c = _emitter->connector(_c_name->str());
if (_envelope["payload"]["filter"]->ok()) {
_c->set("MyApplications", _envelope["payload"]["filter"], _envelope["payload"]["changes"], { "mutated-event", true });
}
else {
_c->set("MyApplications", _envelope["payload"]["href"]->str(), _envelope["payload"]["changes"], { "mutated-event", true });
}
}
}
}

,
{
zpt::mutation::Remove,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
zpt::json _c_names = { zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps" };
for (auto _c_name : _c_names->arr()) {
zpt::connector _c = _emitter->connector(_c_name->str());
if (_envelope["payload"]["filter"]->ok()) {
_c->remove("MyApplications", _envelope["payload"]["filter"], { "mutated-event", true });
}
else {
_c->remove("MyApplications", _envelope["payload"]["href"]->str(), { "mutated-event", true });
}
}
}
}

,
{
zpt::mutation::Replace,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
zpt::json _c_names = { zpt::array, "dbms.mongodb.zpt.apps", "dbms.redis.zpt.apps" };
for (auto _c_name : _c_names->arr()) {
zpt::connector _c = _emitter->connector(_c_name->str());
zpt::json _r_data = _envelope["payload"]["new"];
_c->save("MyApplications", _envelope["payload"]["href"]->str(), _r_data, { "mutated-event", true });
}
}
}

}
);


}

