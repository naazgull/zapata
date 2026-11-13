#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <zapata/http.h>
#include <zapata/http/retrieve.h>
#include <zapata/net/socket/socket_stream.h>

auto zpt::http::retrieve(zpt::message _to_send) -> zpt::message {
    auto _scheme = _to_send->uri()("scheme")->string();
    auto _use_ssl = (_scheme == "https");
    auto _domain = _to_send->uri()("domain")->string();
    auto _port = _to_send->uri()("port")->ok() ? _to_send->uri()("port")->integer() : 0;

    auto _stream = zpt::make_stream<zpt::socketstream>(
      "http", _domain, (_port == 0 ? (_use_ssl ? 443 : 80) : _port), _use_ssl, IPPROTO_TCP);
    _stream //
      ->write<zpt::message>(_to_send);

    auto _reply = zpt::make_message<zpt::http::basic_reply>();
    (*_stream) >> std::noskipws >> _reply;

    return _reply;
}
