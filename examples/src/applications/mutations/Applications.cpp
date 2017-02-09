#include <zapata/applications/mutations/Applications.h>

auto zpt::apps::mutations::Applications::mutify(zpt::mutation::emitter _emitter) -> void {

_emitter->on("^/v2/datums/user-applications$",
{
{
zpt::mutation::Insert,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

,
{
zpt::mutation::Update,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

,
{
zpt::mutation::Remove,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

,
{
zpt::mutation::Replace,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

},
{}
);


_emitter->on("^/v2/datums/users$",
{
{
zpt::mutation::Insert,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

,
{
zpt::mutation::Update,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

,
{
zpt::mutation::Remove,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

,
{
zpt::mutation::Replace,
[] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
}
}

},
{}
);


}

