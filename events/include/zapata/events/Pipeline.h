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

#include <zapata/base.h>
#include <zapata/events/EventEmitter.h>
#include <zapata/events/Polling.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

class pipeline;

typedef std::tuple<std::string, std::function<void(zpt::json, zpt::stage&)>> initializer;
typedef std::tuple<std::string, std::function<void(zpt::socket, zpt::stage&)>> receiver;
typedef std::function<void(std::string, zpt::json, zpt::stage&)>> request_transformer;
typedef std::tuple<
    std::string,
    std::map<zpt::performative, std::function<void(zpt::ev::performative, std::string, zpt::json, zpt::stage&)>>>
    replier;
typedef std::function<void(zpt::json, zpt::stage&)>> reply_transformer;

class stage {
      public:
	stage(std::string _name, zpt::json _options);
	stage(const zpt::stage& _rhs);
	stage(zpt::stage&& _rhs);
	virtual ~stage();

	auto operator=(const zpt::stage& _rhs) -> zpt::stage&;
	auto operator=(zpt::stage&& _rhs) -> zpt::stage&;

	auto forward(zpt::json _msg) -> void;
	auto reply(zpt::json _reply) -> void;
};

class pipeline {
      public:
	pipeline(std::string _name, zpt::json _options);
	virtual ~pipeline();

	virtual void start();

	virtual auto name() -> std::string;
	virtual auto uuid() -> std::string;
	virtual auto options() -> zpt::json;

	virtual auto add(zpt::initializer _callback) -> void;
	virtual auto add(zpt::receiver _callback) -> void;
	virtual auto add(zpt::request_transformer _callback) -> void;
	virtual auto add(zpt::replier _callback) -> void;
	virtual auto add(zpt::reply_transformer _callback) -> void;

	virtual auto suicidal() -> bool;

      private:
	std::string __name;
	std::string __uuid;
	zpt::json __options;
	bool __suicidal;
	short __stage;
	std::vector<zpt::initializer> __stage_0;
	std::vector<zpt::receiver> __stage_1;
	std::vector<zpt::request_transformer> __stage_2;
	std::vector<zpt::replier> __stage_3;
	std::vector<zpt::reply_transformer> __stage_4;
};

namespace conf {
namespace pipeline {
zpt::json init(int argc, char* argv[]);
}
}

namespace pipeline {
extern pid_t root;
extern bool interrupted;

auto terminate(int _signal) -> void;
auto shutdown(int _signal) -> void;

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
