/*
  Copyright (c) 2017, Muzzley
*/

#include <zapata/mqtt/MQTT.h>
#include <ossp/uuid++.hh>
#include <zapata/mqtt/utils.h>

// typedef int mosq_sock_t;

// enum mosquitto_msg_direction {
// 	mosq_md_in = 0,
// 	mosq_md_out = 1
// };

// enum mosquitto_msg_state {
// 	mosq_ms_invalid = 0,
// 	mosq_ms_publish_qos0 = 1,
// 	mosq_ms_publish_qos1 = 2,
// 	mosq_ms_wait_for_puback = 3,
// 	mosq_ms_publish_qos2 = 4,
// 	mosq_ms_wait_for_pubrec = 5,
// 	mosq_ms_resend_pubrel = 6,
// 	mosq_ms_wait_for_pubrel = 7,
// 	mosq_ms_resend_pubcomp = 8,
// 	mosq_ms_wait_for_pubcomp = 9,
// 	mosq_ms_send_pubrec = 10,
// 	mosq_ms_queued = 11
// };

// enum mosquitto_client_state {
// 	mosq_cs_new = 0,
// 	mosq_cs_connected = 1,
// 	mosq_cs_disconnecting = 2,
// 	mosq_cs_connect_async = 3,
// 	mosq_cs_connect_pending = 4,
// 	mosq_cs_connect_srv = 5,
// 	mosq_cs_disconnect_ws = 6,
// 	mosq_cs_disconnected = 7,
// 	mosq_cs_socks5_new = 8,
// 	mosq_cs_socks5_start = 9,
// 	mosq_cs_socks5_request = 10,
// 	mosq_cs_socks5_reply = 11,
// 	mosq_cs_socks5_auth_ok = 12,
// 	mosq_cs_socks5_userpass_reply = 13,
// 	mosq_cs_socks5_send_userpass = 14,
// 	mosq_cs_expiring = 15,
// };

// enum _mosquitto_protocol {
// 	mosq_p_invalid = 0,
// 	mosq_p_mqtt31 = 1,
// 	mosq_p_mqtt311 = 2,
// 	mosq_p_mqtts = 3
// };

// enum _mosquitto_transport {
// 	mosq_t_invalid = 0,
// 	mosq_t_tcp = 1,
// 	mosq_t_ws = 2,
// 	mosq_t_sctp = 3
// };

// struct _mosquitto_packet{
// 	uint8_t *payload;
// 	struct _mosquitto_packet *next;
// 	uint32_t remaining_mult;
// 	uint32_t remaining_length;
// 	uint32_t packet_length;
// 	uint32_t to_process;
// 	uint32_t pos;
// 	uint16_t mid;
// 	uint8_t command;
// 	int8_t remaining_count;
// };

// struct mosquitto_message_all{
// 	struct mosquitto_message_all *next;
// 	time_t timestamp;
// 	//enum mosquitto_msg_direction direction;
// 	enum mosquitto_msg_state state;
// 	bool dup;
// 	struct mosquitto_message msg;
// };

// struct mosquitto {
// 	mosq_sock_t sock;
// 	mosq_sock_t sockpairR, sockpairW;
// 	enum _mosquitto_protocol protocol;
// 	char *address;
// 	char *id;
// 	char *username;
// 	char *password;
// 	uint16_t keepalive;
// 	uint16_t last_mid;
// 	enum mosquitto_client_state state;
// 	time_t last_msg_in;
// 	time_t last_msg_out;
// 	time_t ping_t;
// 	struct _mosquitto_packet in_packet;
// 	struct _mosquitto_packet *current_out_packet;
// 	struct _mosquitto_packet *out_packet;
// 	struct mosquitto_message *will;
// 	SSL *ssl;
// 	SSL_CTX *ssl_ctx;
// 	char *tls_cafile;
// 	char *tls_capath;
// 	char *tls_certfile;
// 	char *tls_keyfile;
// 	int (*tls_pw_callback)(char *buf, int size, int rwflag, void *userdata);
// 	char *tls_version;
// 	char *tls_ciphers;
// 	char *tls_psk;
// 	char *tls_psk_identity;
// 	int tls_cert_reqs;
// 	bool tls_insecure;
// 	bool want_write;
// 	bool want_connect;
// 	pthread_mutex_t callback_mutex;
// 	pthread_mutex_t log_callback_mutex;
// 	pthread_mutex_t msgtime_mutex;
// 	pthread_mutex_t out_packet_mutex;
// 	pthread_mutex_t current_out_packet_mutex;
// 	pthread_mutex_t state_mutex;
// 	pthread_mutex_t in_message_mutex;
// 	pthread_mutex_t out_message_mutex;
// 	pthread_mutex_t mid_mutex;
// 	pthread_t thread_id;
// 	bool clean_session;
// 	void *userdata;
// 	bool in_callback;
// 	unsigned int message_retry;
// 	time_t last_retry_check;
// 	struct mosquitto_message_all *in_messages;
// 	struct mosquitto_message_all *in_messages_last;
// 	struct mosquitto_message_all *out_messages;
// 	struct mosquitto_message_all *out_messages_last;
// 	void (*on_connect)(struct mosquitto *, void *userdata, int rc);
// 	void (*on_disconnect)(struct mosquitto *, void *userdata, int rc);
// 	void (*on_publish)(struct mosquitto *, void *userdata, int mid);
// 	void (*on_message)(struct mosquitto *, void *userdata, const struct mosquitto_message *message);
// 	void (*on_subscribe)(struct mosquitto *, void *userdata, int mid, int qos_count, const int *granted_qos);
// 	void (*on_unsubscribe)(struct mosquitto *, void *userdata, int mid);
// 	void (*on_log)(struct mosquitto *, void *userdata, int level, const char *str);
// 	//void (*on_error)();
// 	char *host;
// 	int port;
// 	int in_queue_len;
// 	int out_queue_len;
// 	char *bind_address;
// 	unsigned int reconnect_delay;
// 	unsigned int reconnect_delay_max;
// 	bool reconnect_exponential_backoff;
// 	bool threaded;
// 	struct _mosquitto_packet *out_packet_last;
// 	int inflight_messages;
// 	int max_inflight_messages;
// };

zpt::MQTT::MQTT()
    : zpt::Channel("", zpt::undefined), __self(this), __connected(false), __postponed(zpt::json::object()) {
	this->__mosq = nullptr;
	/**
	 * Init mosquitto.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_init
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_new
	 */
	mosquitto_lib_init();
	this->__mosq = mosquitto_new(nullptr, true, this);
	int _protocol = MQTT_PROTOCOL_V311;
	mosquitto_opts_set(this->__mosq, MOSQ_OPT_PROTOCOL_VERSION, &_protocol);

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
}

zpt::MQTT::~MQTT() {
	/**
	 * Destroy and clean up.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_destroy
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_cleanup
	 */
	this->__self.reset();
	if (this->__mosq != nullptr) {
		mosquitto_destroy(this->__mosq);
		mosquitto_lib_cleanup();
		this->__mosq = nullptr;
	}
}

auto zpt::MQTT::unbind() -> void { this->__self.reset(); }

auto zpt::MQTT::credentials(std::string _user, std::string _passwd) -> void {
	
	this->__user = _user;
	this->__passwd = _passwd;

	/**
	 * Sets MQTT server access credentials.
	 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_username_pw_set
	 */
	errno = 0;
	int _ret = mosquitto_username_pw_set(this->__mosq, _user.data(), _passwd.data());
	zpt::mqtt_utils::check_err(_ret, errno, this->connection(), zpt::error);
}

auto zpt::MQTT::user() -> std::string { return this->__user; }

auto zpt::MQTT::passwd() -> std::string { return this->__passwd; }

auto zpt::MQTT::self() const -> zpt::mqtt::broker { return this->__self; }

auto zpt::MQTT::connected() -> bool {
	std::lock_guard<std::mutex> _lock(this->__mtx_conn);
	return this->__connected;
}

auto zpt::MQTT::connect(std::string _host, bool _tls, int _port, int _keep_alive) -> bool {
	int _rc = 0;
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_conn);
		this->connection(std::string(_tls ? "mqtts://" : "mqtt://") + _host + std::string(":") +
				 std::to_string(_port));

		if (_tls) {
			
			errno = 0;
			int _ret = mosquitto_tls_insecure_set(this->__mosq, false);
			zpt::mqtt_utils::check_err(_ret, errno, this->connection(), zpt::error);
	
			errno = 0, _ret = mosquitto_tls_opts_set(this->__mosq, 1, nullptr, nullptr);
			zpt::mqtt_utils::check_err(_ret, errno, this->connection(), zpt::error);

			errno = 0, _ret = mosquitto_tls_set(this->__mosq, nullptr, "/usr/lib/ssl/certs/", nullptr, nullptr, nullptr);
			zpt::mqtt_utils::check_err(_ret, errno, this->connection(), zpt::error);

		}

		/**
		 * Connects to the MQTT server.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
		 */
		
		zlog(std::string("going to connect to ") + this->connection(), zpt::notice);
		errno = 0, _rc = mosquitto_connect(this->__mosq, _host.data(), _port, _keep_alive);

	}
	if (_rc == MOSQ_ERR_SUCCESS) {
		
		bool _run = true;
		
		do {
			
			int _ret = mosquitto_loop(this->__mosq, 100, 1);

			if (zpt::mqtt_utils::check_err(_ret, errno, this->connection(), zpt::error) != MOSQ_ERR_SUCCESS) {
				return false;
			}
			{
				std::lock_guard<std::mutex> _lock(this->__mtx_conn);
				_run = !this->__connected;
			}
		} while (_run);
		
		zlog(std::string("connection to ") + this->connection() + std::string(" succeeded"), zpt::notice);

	} else {
		zpt::mqtt_utils::check_err(_rc, errno, this->connection(), zpt::warning);
	}

	return this->__connected;
}

auto zpt::MQTT::reconnect() -> bool {
	int _rc = 0;
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_conn);
		/**
		 * Connects to the MQTT server.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
		 */
		errno = 0;
		zlog(std::string("going to reconnect to ") + this->connection(), zpt::notice);
		_rc = mosquitto_reconnect(this->__mosq);
	}
	if (_rc == MOSQ_ERR_SUCCESS) {
		
		errno = 0;
		bool _run = true;
		
		do {

			int _ret = mosquitto_loop(this->__mosq, 100, 1);
			
			if (zpt::mqtt_utils::check_err(_ret, errno, this->connection(), zpt::error) != MOSQ_ERR_SUCCESS) {
				return false;
			}
			{
				std::lock_guard<std::mutex> _lock(this->__mtx_conn);
				_run = !this->__connected;
			}
		} while (_run);

		zlog(std::string("connection to ") + this->connection() + std::string(" succeeded"), zpt::notice);

	} else {
		zpt::mqtt_utils::check_err(_rc, errno, this->connection(), zpt::warning);
	}

	return this->__connected;
}

auto zpt::MQTT::subscribe(std::string _topic) -> void {
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_conn);
		if (this->__postponed[_topic]->is_string()) {
			return;
		}

		this->__postponed << _topic << _topic;
		if (this->__connected) {
			zlog(std::string("subscribing MQTT topic ") + _topic, zpt::notice);
			int _return;
			/**
			 * Subscribes to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic
			 * subscription patterns.
			 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_subscribe
			 */
			mosquitto_subscribe(this->__mosq, &_return, _topic.data(), 0);
			mosquitto_loop_write(this->__mosq, 1);
		}
	}
}

auto zpt::MQTT::publish(std::string _topic, zpt::json _payload) -> void {
	{
		std::lock_guard<std::mutex> _lock(this->__mtx);
		if (this->__connected) {
			std::string _payload_str = (std::string)_payload;
			int _return;
			/**
			 * Publishes a message to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic
			 * subscription patterns.
			 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_publish
			 */
			mosquitto_publish(this->__mosq,
					  &_return,
					  _topic.data(),
					  _payload_str.length(),
					  (const uint8_t*)_payload_str.data(),
					  0,
					  false);
			mosquitto_loop_write(this->__mosq, 1);
		}
	}
}

auto zpt::MQTT::on(std::string _event, zpt::mqtt::handler _callback) -> void {
	/**
	 * Add to the callback list, the callback *_callback*, attached to the event type *_event*.
	 */
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_callbacks);
		auto _found = this->__callbacks.find(_event);
		if (_found != this->__callbacks.end()) {
			_found->second.push_back(_callback);
		} else {
			std::vector<zpt::mqtt::handler> _callbacks;
			_callbacks.push_back(_callback);
			this->__callbacks.insert(std::make_pair(_event, _callbacks));
		}
	}
}

auto zpt::MQTT::off(std::string _event) -> void {
	/**
	 * Remove from the callback list, the callback *_callback*, attached to the event type *_event*.
	 */
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_callbacks);
		auto _found = this->__callbacks.find(_event);
		if (_found != this->__callbacks.end()) {
			this->__callbacks.erase(_found);
		}
	}
}

auto zpt::MQTT::trigger(std::string _event, zpt::mqtt::data _data) -> void {
	/**
	 * Searches for and executes registered callbacks under the event type *_event*.
	 */
	std::vector<zpt::mqtt::handler> _callbacks;
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_callbacks);
		auto _found = this->__callbacks.find(_event);
		if (_found != this->__callbacks.end()) {
			_callbacks = _found->second;
		}
	}
	for (auto _c : _callbacks) {
		_c(_data, this->self());
	}
}

auto zpt::MQTT::on_connect(struct mosquitto* _mosq, void* _ptr, int _rc) -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	if (_rc == 0) {
		{
			std::lock_guard<std::mutex> _lock(_self->__mtx_conn);
			int _return;
			for (auto _topic : _self->__postponed->obj()) {
				zlog(std::string("subscribing MQTT topic ") + _topic.first, zpt::notice);
				mosquitto_subscribe(_mosq, &_return, _topic.first.data(), 0);
				mosquitto_loop_write(_mosq, 1);
			}
			mosquitto_loop_misc(_mosq);
			_self->__connected = true;
		}
	}
	zpt::mqtt::data _data(new MQTTData());
	_data->__rc = _rc;
	_self->trigger("connect", _data);
}

auto zpt::MQTT::on_disconnect(struct mosquitto* _mosq, void* _ptr, int _reason) -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	{
		std::lock_guard<std::mutex> _lock(_self->__mtx_conn);
		_self->__connected = false;
	}
	zpt::mqtt::data _data(new MQTTData());
	_self->trigger("disconnect", _data);
}

auto zpt::MQTT::on_publish(struct mosquitto* _mosq, void* _ptr, int _mid) -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	zpt::mqtt::data _data(new MQTTData());
	_data->__mid = _mid;
	_self->trigger("publish", _data);
}

auto zpt::MQTT::on_message(struct mosquitto* _mosq, void* _ptr, const struct mosquitto_message* _message) -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	zpt::mqtt::data _data(new MQTTData());
	try {
		_data->__message = zpt::json(std::string((char*)_message->payload, _message->payloadlen));
		ztrace("zpt::MQTT::on_message");
		_data->__topic = zpt::json::string(_message->topic);
		_self->trigger("message", _data);
	} catch (std::exception& _e) {
		zlog(std::string("zpt::MQTT::on_message error: ") + _e.what(), zpt::error);
	}
}

auto zpt::MQTT::on_subscribe(struct mosquitto* _mosq, void* _ptr, int _mid, int _qos_count, const int* _granted_qos)
    -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	zpt::mqtt::data _data(new MQTTData());
	_data->__mid = _mid;
	_data->__qos_count = _qos_count;
	_data->__granted_qos = _granted_qos;
	_self->trigger("subscribe", _data);
}

auto zpt::MQTT::on_unsubscribe(struct mosquitto* _mosq, void* _ptr, int _mid) -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	zpt::mqtt::data _data(new MQTTData());
	_data->__mid = _mid;
	_self->trigger("unsubscribe", _data);
}

auto zpt::MQTT::on_error(struct mosquitto* _mosq, void* _ptr) -> void {
	zpt::MQTT* _self = (zpt::MQTT*)_ptr;
	zpt::mqtt::data _data(new MQTTData());
	_self->trigger("error", _data);
}

auto zpt::MQTT::on_log(struct mosquitto* _mosq, void* _ptr, int _level, const char* _message) -> void {
	zlog(std::string(_message), (zpt::LogLevel)_level);
}

auto zpt::MQTT::id() -> std::string { return "__mqtt_connection__"; }

auto zpt::MQTT::uri(size_t _idx) -> zpt::json { return zpt::undefined; }

auto zpt::MQTT::uri(std::string _uris) -> void {}

auto zpt::MQTT::detach() -> void {}

auto zpt::MQTT::close() -> void {
	if (this->__mosq != nullptr) {
		std::lock_guard<std::mutex> _lock(this->__mtx_conn);
		mosquitto_destroy(this->__mosq);
		mosquitto_lib_cleanup();
		this->__mosq = nullptr;
	}
}

auto zpt::MQTT::available() -> bool { return true; }

auto zpt::MQTT::buffer(zpt::json _envelope) -> void { this->__buffer = _envelope; }

auto zpt::MQTT::recv() -> zpt::json {
	std::lock_guard<std::mutex> _lock(this->__mtx_conn);
	if (!this->__connected) {
		return {
		    "protocol",
		    this->protocol(),
		    "error",
		    true,
		    "status",
		    502,
		    "payload",
		    {"text", "connection lost to MQTT server", "assertion_failed", "this->__connected", "code", 1062}};
	}
	mosquitto_loop_read(this->__mosq, 1);
	mosquitto_loop_misc(this->__mosq);
	zpt::json _return = this->__buffer;
	this->__buffer = zpt::undefined;
	return _return;
}

auto zpt::MQTT::send(zpt::ev::performative _performative, std::string _resource, zpt::json _payload) -> zpt::json {
	this->publish(_resource, _payload);
	{
		std::lock_guard<std::mutex> _lock(this->__mtx_conn);
		mosquitto_loop_misc(this->__mosq);
	}
	return zpt::undefined;
}

auto zpt::MQTT::send(zpt::json _envelope) -> zpt::json {
	assertz(_envelope["payload"]->ok() && _envelope["resource"]->ok(),
		"'performative' and 'resource' attributes are required",
		412,
		0);
	this->send(zpt::ev::Reply, std::string(_envelope["resource"]), _envelope["payload"]);
	return zpt::undefined;
}

auto zpt::MQTT::loop_iteration() -> void {
	std::lock_guard<std::mutex> _lock(this->__mtx_conn);
	mosquitto_loop_misc(this->__mosq);
}

auto zpt::MQTT::socket() -> zmq::socket_ptr { return zmq::socket_ptr(nullptr); }

auto zpt::MQTT::in() -> zmq::socket_ptr { return zmq::socket_ptr(nullptr); }

auto zpt::MQTT::out() -> zmq::socket_ptr { return zmq::socket_ptr(nullptr); }

auto zpt::MQTT::fd() -> int {
	std::lock_guard<std::mutex> _lock(this->__mtx_conn);
	return mosquitto_socket(this->__mosq);
}

auto zpt::MQTT::in_mtx() -> std::mutex& { return this->__mtx_conn; }

auto zpt::MQTT::out_mtx() -> std::mutex& { return this->__mtx_conn; }

auto zpt::MQTT::type() -> short int { return ZMQ_MQTT_RAW; }

auto zpt::MQTT::protocol() -> std::string { return "MQTT/3.1"; }

extern "C" auto zpt_mqtt() -> int { return 1; }
