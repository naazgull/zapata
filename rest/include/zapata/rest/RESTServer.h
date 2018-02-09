/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <zapata/events.h>
#include <zapata/zmq.h>
#include <zapata/zmq/SocketStreams.h>
#include <zapata/rest/RESTEmitter.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

namespace conf {
namespace rest {
zpt::json init(int argc, char* argv[]);
}
}

namespace rest {
extern pid_t root;
extern bool interrupted;

auto terminate(int _signal) -> void;
auto shutdown(int _signal) -> void;

namespace uri {
auto get_simplified_topics(std::string _pattern) -> zpt::json;
}

namespace authorization {
auto serialize(zpt::json _info) -> std::string;
auto deserialize(std::string _token) -> zpt::json;
auto extract(zpt::json _envelope) -> std::string;
auto headers(std::string _token) -> zpt::json;
auto validate(std::string _topic,
	      zpt::json _envelope,
	      zpt::ev::emitter _emitter,
	      zpt::json _roles_needed = zpt::undefined) -> zpt::json;
auto has_roles(zpt::json _indentity, zpt::json _roles_needed) -> bool;
}

namespace scopes {
std::string serialize(zpt::json _info);
zpt::json deserialize(std::string _scope);
bool has_permission(std::string _scope, std::string _ns, std::string _permissions);
bool has_permission(zpt::json _scope, std::string _ns, std::string _permissions);
}
}
}
