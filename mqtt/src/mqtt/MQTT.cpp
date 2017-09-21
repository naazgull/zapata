/*
  Copyright (c) 2017, Muzzley
*/

#include <zapata/mqtt/MQTT.h>
#include <ossp/uuid++.hh>

zpt::MQTT::MQTT() : zpt::ZMQ("", zpt::undefined), __self(this), __connected(false), __postponed(zpt::json::object()) {
#if defined(ZPT_USE_MOSQUITTO)
	this->__mosq =  nullptr;
	/**
	 * Init mosquitto.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_init
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_new
	 */
	mosquitto_lib_init();
	this->__mosq = mosquitto_new(nullptr, true, this);

	/**
	 * Register the delegating callbacks.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect_callback_set
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_disconnect_callback_set
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_publish_callback_set
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_message_callback_set
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_subscribe_callback_set
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_unsubscribe_callback_set
	 */
	mosquitto_connect_callback_set(this->__mosq, MQTT::on_connect);
	mosquitto_disconnect_callback_set(this->__mosq, MQTT::on_disconnect);
	mosquitto_publish_callback_set(this->__mosq, MQTT::on_publish);
	mosquitto_message_callback_set(this->__mosq, MQTT::on_message);
	mosquitto_subscribe_callback_set(this->__mosq, MQTT::on_subscribe);
	mosquitto_unsubscribe_callback_set(this->__mosq, MQTT::on_unsubscribe);
	mosquitto_log_callback_set(this->__mosq, MQTT::on_log);
#elif false && defined(ZPT_USE_PAHO)
	this->__paho = nullptr;
	this->__paho_opts = new ::mqtt::connect_options();
	this->__paho_opts->set_clean_session(true);
	this->__paho_opts->set_automatic_reconnect(1, 1);
#endif	       

	}

zpt::MQTT::~MQTT() {
	/**
	 * Destroy and clean up.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_destroy
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_cleanup
	 */
#if defined(ZPT_USE_MOSQUITTO)
	this->__self.reset();
	if (this->__mosq != nullptr) {
		mosquitto_destroy(this->__mosq);
		mosquitto_lib_cleanup();
		this->__mosq = nullptr;
	}
#elif false && defined(ZPT_USE_PAHO)
	if (this->__paho != nullptr) {
		delete this->__paho;
	}
	if (this->__paho_opts != nullptr) {
		delete this->__paho_opts;
	}
#endif	       
}

auto zpt::MQTT::unbind() -> void {
#if defined(ZPT_USE_MOSQUITTO)
	this->__self.reset();
#endif	       
}

auto zpt::MQTT::credentials(std::string _user, std::string _passwd) -> void {
	this->__user = _user;
	this->__passwd = _passwd;
#if defined(ZPT_USE_MOSQUITTO)
	/**
	 * Sets MQTT server access credentials.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_username_pw_set
	 */
	mosquitto_username_pw_set(this->__mosq, _user.data(), _passwd.data());
#elif false && defined(ZPT_USE_PAHO)
	this->__paho_opts->set_user_name(this->__user);
	this->__paho_opts->set_password(this->__passwd);
#endif
}

auto zpt::MQTT::user() -> std::string {
	return this->__user;
}

auto zpt::MQTT::passwd() -> std::string {
	return this->__passwd;
}

auto zpt::MQTT::self() const -> zpt::mqtt::broker {
	return this->__self;
}

auto zpt::MQTT::connected() -> bool {
	std::lock_guard< std::mutex > _lock(this->__mtx_conn);
	return this->__connected;
}

auto zpt::MQTT::connect(std::string _host, bool _tls, int _port, int _keep_alive) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx_conn);
	this->connection(std::string(_tls ? "mqtts://" : "mqtt://") + _host + std::string(":") + std::to_string(_port));

#if defined(ZPT_USE_MOSQUITTO)
	if (_tls) {
		mosquitto_tls_insecure_set(this->__mosq, false);
		mosquitto_tls_opts_set(this->__mosq, 1, nullptr, nullptr);
		mosquitto_tls_set(this->__mosq, nullptr, "/usr/lib/ssl/certs/", nullptr, nullptr, nullptr);
	}

	/**
	 * Connects to the MQTT server.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
	 */
	mosquitto_connect(this->__mosq, _host.data(), _port, _keep_alive);
#elif false && defined(ZPT_USE_PAHO)
	this->__paho_opts->set_keep_alive_interval(_keep_alive);
	
	std::string _uri;
	if (_tls) {
		_uri.assign((std::string("ssl://") + _host + std::string(":") + std::to_string(_port)));
		::mqtt::ssl_options _ssl_opts;
		_ssl_opts.set_trust_store("/usr/lib/ssl/certs/");
		this->__paho_opts->set_ssl(_ssl_opts);
	}
	else {
		_uri.assign((std::string("tcp://") + _host + std::string(":") + std::to_string(_port)));
	}
	this->__paho = new ::mqtt::async_client(_uri, zpt::generate::r_uuid());
#endif	       
}

auto zpt::MQTT::reconnect() -> bool {
	std::lock_guard< std::mutex > _lock(this->__mtx_conn);

#if defined(ZPT_USE_MOSQUITTO)
	/**
	 * Connects to the MQTT server.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
	 */
	bool _return = mosquitto_reconnect(this->__mosq) == MOSQ_ERR_SUCCESS;
	zlog(std::string("trying to reconnect to MQTT: ") + (_return ? std::string("success!") : std::string("failed!")), zpt::notice);
	return _return;
#elif false && defined(ZPT_USE_PAHO)		
#endif	       
}

auto zpt::MQTT::subscribe(std::string _topic) -> void {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx_conn);
		if (this->__postponed[_topic]->is_string()) {
			return;
		}
		
		this->__postponed << _topic << _topic;
		if (this->__connected) {
			zlog(std::string("subscribing MQTT topic ") + _topic, zpt::notice);
#if defined(ZPT_USE_MOSQUITTO)
			int _return;
			/**
			 * Subscribes to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic subscription patterns.
			 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_subscribe
			 */
			mosquitto_subscribe(this->__mosq, & _return, _topic.data(), 0);
			mosquitto_loop_write(this->__mosq, 1);
#elif false && defined(ZPT_USE_PAHO)
			this->__paho->subscribe(_topic, 0);
#endif	       
		} }
}

auto zpt::MQTT::publish(std::string _topic, zpt::json _payload) -> void {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		if (this->__connected) {
			std::string _payload_str = (std::string) _payload;
#if defined(ZPT_USE_MOSQUITTO)
			int _return;
			/**
			 * Publishes a message to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic subscription patterns.
			 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_publish
			 */
			mosquitto_publish(this->__mosq, & _return, _topic.data(), _payload_str.length(), (const uint8_t *) _payload_str.data(), 0, false);
			mosquitto_loop_write(this->__mosq, 1);
#elif false && defined(ZPT_USE_PAHO)
			::mqtt::message_ptr _msg = ::mqtt::make_message(_topic, _payload_str);
			_msg->set_qos(0);
			this->__paho->publish(_msg);
#endif	       
		} } 
}

auto zpt::MQTT::on(std::string _event, zpt::mqtt::handler _callback) -> void {
	/**
	 * Add to the callback list, the callback *_callback*, attached to the event type *_event*.
	 */
	{ std::lock_guard< std::mutex > _lock(this->__mtx_callbacks);
		auto _found = this->__callbacks.find(_event);
		if (_found != this->__callbacks.end()) {
			_found->second.push_back(_callback);
		}
		else {
			std::vector< zpt::mqtt::handler > _callbacks;
			_callbacks.push_back(_callback);
			this->__callbacks.insert(std::make_pair(_event, _callbacks));
		} }
}

auto zpt::MQTT::off(std::string _event) -> void {
	/**
	 * Remove from the callback list, the callback *_callback*, attached to the event type *_event*.
	 */
	{ std::lock_guard< std::mutex > _lock(this->__mtx_callbacks);
		auto _found = this->__callbacks.find(_event);
		if (_found != this->__callbacks.end()) {
			this->__callbacks.erase(_found);
		} }
}

auto zpt::MQTT::trigger(std::string _event, zpt::mqtt::data _data) -> void {
	/**
	 * Searches for and executes registered callbacks under the event type *_event*.
	 */
	std::vector<zpt::mqtt::handler> _callbacks;
	{ std::lock_guard< std::mutex > _lock(this->__mtx_callbacks);
		auto _found = this->__callbacks.find(_event);
		if (_found != this->__callbacks.end()) {
			_callbacks = _found->second; 
		} }
	for( auto _c : _callbacks) {
		_c(_data, this->self());
	}
}

// auto zpt::MQTT::start() -> void {
// #if defined(ZPT_USE_MOSQUITTO)
// 	/**
// 	 * Checks if some data is available from MQTT server.
// 	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_loop_forever
// 	 */
// 	mosquitto_loop_start(this->__mosq);
// #elif false && defined(ZPT_USE_PAHO)
// 	zpt::MQTT::callback* _callback = new zpt::MQTT::callback(this);
// 	this->__paho->set_callback(*_callback);

// 	try {
// 		this->__paho->connect(*this->__paho_opts, nullptr, *_callback);
// 	}
// 	catch (::mqtt::exception& _e) {
// 	}
// #endif	       
// }

// auto zpt::MQTT::loop() -> void {
// #if defined(ZPT_USE_MOSQUITTO)
// 	/**
// 	 * Checks if some data is available from MQTT server.
// 	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_loop_forever
// 	 */
// 	mosquitto_loop_forever(this->__mosq, -1, 1);
// #elif false && defined(ZPT_USE_PAHO)		
// #endif	       
// }

#if defined(ZPT_USE_MOSQUITTO)
auto zpt::MQTT::on_connect(struct mosquitto * _mosq, void * _ptr, int _rc) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	if (_rc == 0) {
		{ std::lock_guard< std::mutex > _lock(_self->__mtx_conn);
			int _return;
			for (auto _topic : _self->__postponed->obj()) {
				zlog(std::string("subscribing MQTT topic ") + _topic.first, zpt::notice);
				mosquitto_subscribe(_mosq, & _return, _topic.first.data(), 0);
				mosquitto_loop_write(_mosq, 1);
			}
			mosquitto_loop_misc(_mosq);
			_self->__connected = true; }
	}
	zpt::mqtt::data _data(new MQTTData());
	_data->__rc = _rc;
	_self->trigger("connect", _data);
}

auto zpt::MQTT::on_disconnect(struct mosquitto * _mosq, void * _ptr, int _reason) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	{ std::lock_guard< std::mutex > _lock(_self->__mtx_conn);
		_self->__connected = false; }
	zpt::mqtt::data _data(new MQTTData());
	_self->trigger("disconnect", _data);
}

auto zpt::MQTT::on_publish(struct mosquitto * _mosq, void * _ptr, int _mid) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	zpt::mqtt::data _data(new MQTTData());
	_data->__mid = _mid;
	_self->trigger("publish", _data);
}

auto zpt::MQTT::on_message(struct mosquitto * _mosq, void * _ptr, const struct mosquitto_message * _message) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	zpt::mqtt::data _data(new MQTTData());
	try {
		_data->__message = zpt::json(std::string((char*) _message->payload, _message->payloadlen));
		ztrace("zpt::MQTT::on_message");
		_data->__topic = zpt::json::string(_message->topic);
		_self->trigger("message", _data);
	}
	catch(std::exception& _e) {
		zlog(std::string("zpt::MQTT::on_message error: ") + _e.what(), zpt::error);
	}
}

auto zpt::MQTT::on_subscribe(struct mosquitto * _mosq, void * _ptr, int _mid, int _qos_count, const int * _granted_qos) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	zpt::mqtt::data _data(new MQTTData());
	_data->__mid = _mid;
	_data->__qos_count = _qos_count;
	_data->__granted_qos = _granted_qos;
	_self->trigger("subscribe", _data);
}

auto zpt::MQTT::on_unsubscribe(struct mosquitto * _mosq, void * _ptr, int _mid) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	zpt::mqtt::data _data(new MQTTData());
	_data->__mid = _mid;
	_self->trigger("unsubscribe", _data);
}

auto zpt::MQTT::on_error(struct mosquitto * _mosq, void * _ptr) -> void {
	zpt::MQTT* _self = (zpt::MQTT*) _ptr;
	zpt::mqtt::data _data(new MQTTData());
	_self->trigger("error", _data);
}

auto zpt::MQTT::on_log(struct mosquitto * _mosq, void * _ptr, int _level, const char* _message) -> void {
	zlog(std::string(_message), (zpt::LogLevel) _level);
}

#elif false && defined(ZPT_USE_PAHO)

zpt::MQTT::callback::callback(zpt::MQTT* _broker) : __broker(_broker) {
}

auto zpt::MQTT::callback::on_failure(const ::mqtt::token& _tok) -> void {
	zlog("connection to MQTT failed, re-trying in 1 second...", zpt::warning);
}

auto zpt::MQTT::callback::on_success(const ::mqtt::token& _tok) -> void {
	{ std::lock_guard< std::mutex > _lock(this->__broker->__mtx_conn);
		for (auto _topic : this->__broker->__postponed->obj()) {
			zlog(std::string("subscribing MQTT topic ") + _topic.first, zpt::notice);
			this->__broker->__paho->subscribe(_topic.first, 0);
		}
		this->__broker->__connected = true; }
	zpt::mqtt::data _data(new MQTTData());
	_data->__rc = 1;
	this->__broker->trigger("connect", _data);
}

auto zpt::MQTT::callback::connection_lost(const std::string& _cause) -> void {
	{ std::lock_guard< std::mutex > _lock(this->__broker->__mtx_conn);
		this->__broker->__connected = false; }
	zpt::mqtt::data _data(new MQTTData());
	this->__broker->trigger("disconnect", _data);
}

auto zpt::MQTT::callback::message_arrived(::mqtt::const_message_ptr _msg) -> void {
	zpt::mqtt::data _data(new MQTTData());
	try {
		_data->__message = zpt::json(_msg->to_string());
		ztrace("zpt::MQTT::on_message");
		_data->__topic = zpt::json::string(_msg->get_topic());
		this->__broker->trigger("message", _data);
	}
	catch(std::exception& _e) {
		zlog(std::string("zpt::MQTT::on_message error: ") + _e.what(), zpt::error);
	}
}

auto zpt::MQTT::callback::delivery_complete(::mqtt::delivery_token_ptr token) -> void{}

#endif	       

auto zpt::MQTT::id() -> std::string {
	return "__mqtt_connection__";
}

auto zpt::MQTT::uri(size_t _idx) -> zpt::json {
	return zpt::undefined;
}

auto zpt::MQTT::uri(std::string _uris) -> void{
}

auto zpt::MQTT::detach() -> void {
}

auto zpt::MQTT::close() -> void {
#if defined(ZPT_USE_MOSQUITTO)
	if (this->__mosq != nullptr) {
		mosquitto_destroy(this->__mosq);
		mosquitto_lib_cleanup();
		this->__mosq = nullptr;
	}
#elif false && defined(ZPT_USE_PAHO)
	if (this->__paho != nullptr) {
		delete this->__paho;
	}
	if (this->__paho_opts != nullptr) {
		delete this->__paho_opts;
	}
#endif
}

auto zpt::MQTT::available() -> bool {
	return true;
}

auto zpt::MQTT::buffer(zpt::json _envelope) -> void {
	this->__buffer = _envelope;
}

auto zpt::MQTT::recv() -> zpt::json {
	mosquitto_loop_read(this->__mosq, 1);
	mosquitto_loop_misc(this->__mosq);
	zpt::json _return = this->__buffer;
	this->__buffer = zpt::undefined;
	return _return;
}

auto zpt::MQTT::send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload) -> zpt::json {
	this->publish(_resource, _payload);
	mosquitto_loop_misc(this->__mosq);
	return zpt::undefined;
}

auto zpt::MQTT::send(zpt::json _envelope) -> zpt::json {
	assertz(_envelope["payload"]->ok() && _envelope["resource"]->ok(), "'performative' and 'resource' attributes are required", 412, 0);
	this->send(zpt::ev::Reply, std::string(_envelope["resource"]), _envelope["payload"]);
	return zpt::undefined;
}

auto zpt::MQTT::loop_iteration() -> void {
	mosquitto_loop_misc(this->__mosq);
}

auto zpt::MQTT::socket() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::MQTT::in() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::MQTT::out() -> zmq::socket_ptr {
	return zmq::socket_ptr(nullptr);
}

auto zpt::MQTT::fd() -> int {
	return mosquitto_socket(this->__mosq);
}

auto zpt::MQTT::in_mtx() -> std::mutex& {
	return this->__mtx_conn;
}

auto zpt::MQTT::out_mtx() -> std::mutex& {
	return this->__mtx_conn;
}

auto zpt::MQTT::type() -> short int {
	return ZMQ_MQTT_RAW;
}

auto zpt::MQTT::protocol() -> std::string {
	return "MQTT/3.1";
}

extern "C" auto zpt_mqtt() -> int {
	return 1;
}
