/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <zapata/transport.h>
#include <zapata/net/socket.h>
#include <zapata/net/tcp.h>

auto main(int argc, char* argv[]) -> int {
    if (argc > 2) {
        std::string _type{ argv[1] };
        zpt::transport _transport{ new zpt::net::transport::tcp{} };
        std::istringstream _iss;
        _iss.str(std::string{ argv[2] });
        std::uint16_t _port{ 0 };
        _iss >> _port;

        if (_type == "server") {

            zpt::serversocketstream _ssock{ _port };
            do {
                auto _stream = _ssock->accept();
                _stream->transport("tcp");
                auto _t1 = std::chrono::high_resolution_clock::now();
                auto _received = _transport->receive(_stream);
                auto _t2 = std::chrono::high_resolution_clock::now();
                auto _duration1 =
                  std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
                auto _t3 = std::chrono::high_resolution_clock::now();
                _transport->send(_stream, _received);
                auto _t4 = std::chrono::high_resolution_clock::now();
                auto _duration2 =
                  std::chrono::duration_cast<std::chrono::microseconds>(_t4 - _t3).count();
                std::cout << "# processing time" << std::endl
                          << "\trequest: " << _duration1 << "µs" << std::endl
                          << "\tresponse: " << _duration2 << "µs" << std::endl
                          << std::flush;
            } while (true);
        }
        else {
            auto _message = _transport->make_request();
            auto& _json = zpt::message_cast<zpt::json_message>(_message);
            _json //
              .performative(zpt::Post)
              .uri("/test")
              .body() =
              R"({"body":[{"_id":"/NOTIFY/minions/boot","hash":0,"metadata":"{\"host\":\"localhost\"}","provider":"<self>"},{"_id":"/POST/minions/hello","hash":1,"metadata":"{\"host\":\"localhost\"}","provider":"<self>"}],"headers":{"Cache-Control":"no-store","Content-Type":"application/json","Date":"Sun, 10 Aug 2025 15:17:01 WEST","Host":"192.168.50.11:8083","X-Conversation-ID":"26a97074-a68d-4227-a6fd-fdfd2d6f6369","X-Version":"1.1"},"performative":"POST","uri":{"domain":"192.168.50.11","is_relative":false,"path":["minions","hello"],"port":8083,"raw_path":"/minions/hello","scheme":"tcp"}})"_JSON;

            auto _stream =
              zpt::make_stream<zpt::socketstream>("127.0.0.1", _port, zpt::NO_SSL, IPPROTO_TCP);
            _stream //
              ->transport("tcp");

            _transport->send(_stream, _message);

            if (zpt::stream_cast<zpt::socketstream>(_stream).is_error()) {
                zlog(zpt::stream_cast<zpt::socketstream>(_stream).error_string(), zpt::debug);
            }
        }
    }
}
