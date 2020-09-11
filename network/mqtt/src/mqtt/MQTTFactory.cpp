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

#include <zapata/mqtt/MQTTFactory.h>

zpt::MQTTFactory::MQTTFactory()
  : zpt::ChannelFactory() {}

zpt::MQTTFactory::~MQTTFactory() {}

auto
zpt::MQTTFactory::produce(zpt::json _options) -> zpt::socket {
    zpt::socket _return;
    auto _found = this->__channels.find(_options["connection"]->string());
    if (_found != this->__channels.end()) {
        _return = _found->second;
    }
    else {
        zpt::MQTT* _mqtt = new zpt::MQTT();
        int _attempts = 0;
        do {
            if (_mqtt->connect(zpt::uri::parse(_options["connection"]->string()))) {
                break;
            }
            ++_attempts;
            sleep(1);
        } while (_attempts < 10);
        if (_mqtt->connected()) {
            zlog(std::string("binding ") + _mqtt->protocol() + std::string(" listener to ") +
                   std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) +
                   std::string(":") + std::string(_uri["port"]),
                 zpt::info);
        }
        else {
            zlog(std::string("unable to bind ") + _mqtt->protocol() + std::string(" listener to ") +
                   std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) +
                   std::string(":") + std::string(_uri["port"]),
                 zpt::warning);
        }
        _return = zpt::socket(_mqtt);
    }
    return _return;
}

auto
zpt::MQTTFactory::is_reusable(std::string const& _type) -> bool {
    return true;
}

auto
zpt::MQTTFactory::clean(zpt::socket _socket) -> bool {
    return false;
}

auto
zpt::MQTTFactory::on_connect(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void {
    if (_data->__rc == 0) {
        zlog(std::string("MQTT server is up and connection authenticated"), zpt::notice);
    }
}

auto
zpt::MQTTFactory::on_disconnect(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) mutable -> void {
    zpt::poll::instance()->vanished(_mqtt.get(), [=](zpt::ev::emitter _emitter) mutable -> void {
        int _attempts = 0;
        do {
            if (_mqtt->reconnect()) {
                break;
            }
            ++_attempts;
            sleep(1);
        } while (_attempts < 10);
        if (_mqtt->connected()) {
            zlog(std::string("binding ") + _mqtt->protocol() + std::string(" listener to ") +
                   std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) +
                   std::string(":") + std::string(_uri["port"]),
                 zpt::info);
            zpt::poll::instance()->poll(zpt::poll::instance()->add(_mqtt.get()));
        }
        else {
            zlog(std::string("unable to bind ") + _mqtt->protocol() + std::string(" listener to ") +
                   std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) +
                   std::string(":") + std::string(_uri["port"]),
                 zpt::warning);
        }
    });
}

auto
zpt::MQTTFactory::on_message(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) mutable -> void {
    zpt::json _envelope = zpt::json::object();

    _envelope << "performative" << int(zpt::ev::Reply);
    if (!_data->__message["channel"]->ok() ||
        !zpt::test::uuid(std::string(_data->__message["channel"]))) {
        _envelope << "channel" << zpt::generate::r_uuid();
    }
    else {
        _envelope << "channel" << _data->__message["channel"];
    }

    _envelope << "resource" << _data->__topic;

    if (!_data->__message["payload"]->ok()) {
        _envelope << "payload" << _data->__message;
    }
    else {
        _envelope << "payload" << _data->__message["payload"];
    }
    if (_data->__message["headers"]->ok()) {
        _envelope << "headers" << _data->__message["headers"];
    }
    if (_data->__message["params"]->ok()) {
        _envelope << "params" << _data->__message["params"];
    }
    _envelope << "protocol" << _mqtt->protocol();
    ztrace(std::string("MQTT ") + std::string(_data->__topic));
    zverbose(zpt::ev::pretty(_envelope));

    _mqtt->buffer(_envelope);
}

extern "C" void
_zpt_plugin_load_() {
    zpt::ev::emitter_factory _emitter = zpt::emitter();
    zpt::channel_factory _factory(new zpt::MQTTFactory());
    _emitter->channel({ { "mqtt", _factory }, { "mqtts", _factory } });
    zpt::json _options = _emitter->options();
    zpt::json _credentials = _emitter->credentials();

    if (_credentials["endpoints"]["mqtt"]->ok()) {
        if (!_options["mqtt"]->ok())
            _options << "mqtt" << zpt::json::array();
        _options["mqtt"] << _credentials["endpoints"]["mqtt"];
    }

    if (_options["mqtt"]->ok() && _options["mqtt"]->is_array()) {
        for (auto _definition : _options["mqtt"]->array()) {
            zpt::mqtt::broker _mqtt;
            zpt::json _uri = zpt::uri::parse(std::string(_definition["bind"]));

            if (!_uri["port"]->ok()) {
                _uri << "port" << (_uri["scheme"] == zpt::json::string("mqtts") ? 8883 : 1883);
            }
            if (_uri["user"]->ok() && _uri["password"]->ok()) {
                _mqtt->credentials(std::string(_uri["user"]), std::string(_uri["password"]));
            }
            else if (_credentials["client_id"]->is_string() &&
                     _credentials["access_token"]->is_string()) {
                _mqtt->credentials(std::string(_credentials["client_id"]),
                                   std::string(_credentials["access_token"]));
            }

            _mqtt->on("connect", zpt::MQTTFactory::on_connect);
            _mqtt->on("disconnect", zpt::MQTTFactory::on_disconnect);
            _mqtt->on("message", zpt::MQTTFactory::on_message);

            int _attempts = 0;
            do {
                if (_mqtt->connect(std::string(_uri["domain"]),
                                   _uri["scheme"] == zpt::json::string("mqtts"),
                                   int(_uri["port"]))) {
                    break;
                }
                ++_attempts;
                sleep(1);
            } while (_attempts < 10);
            if (_mqtt->connected()) {
                zlog(std::string("binding ") + _mqtt->protocol() + std::string(" listener to ") +
                       std::string(_uri["scheme"]) + std::string("://") +
                       std::string(_uri["domain"]) + std::string(":") + std::string(_uri["port"]),
                     zpt::info);
                zpt::poll::instance()->poll(zpt::poll::instance()->add(_mqtt.get()));
            }
            else {
                zlog(std::string("unable to bind ") + _mqtt->protocol() +
                       std::string(" listener to ") + std::string(_uri["scheme"]) +
                       std::string("://") + std::string(_uri["domain"]) + std::string(":") +
                       std::string(_uri["port"]),
                     zpt::warning);
            }
        }
    }
}
