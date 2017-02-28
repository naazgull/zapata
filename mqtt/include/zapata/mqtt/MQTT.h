/*
Copyright (c) 2014, Muzzley
*/

/**
 * This example uses the mosquitto C library (http://mosquitto.org).
 * In Ubuntu based systems is installable by executing:
 * $ sudo apt-get install libmosquitto0 libmosquitto0-dev
 * 
 * Compile with '-lmosquitto'.
 */
#include <unistd.h>
#include <iostream>
#include <functional>
#include <memory>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <mosquitto.h>
#include <mutex>
#include <zapata/json.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
	class MQTT;

	/**
	 * Data structure that will hold the data for callbacks.
	 * Attributes will be instantiated according to the callback type being registered. 
	 * For more info, see libmosquitto documentation on setting callbacks (http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect_callback_set)
	 */
	typedef struct MQTTDataStruct {
		int __rc = 0;
		int __mid = 0;
		zpt::json __topic;
		zpt::json __message;
		int __qos_count = 0;
		const int * __granted_qos = nullptr;
	} MQTTData;


	namespace mqtt {

		typedef std::shared_ptr< zpt::MQTT > broker;
		/**
		 * Smart pointer to the callback data.
		 */
		typedef std::shared_ptr< zpt::MQTTData > data;
		/**
		 * Lambda style callback definition.
		 */
		typedef std::function< void (zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) > handler;
		/**
		 * Map for holding the differente callbacks registered for each different event type.
		 */
		typedef std::map< std::string, std::vector< zpt::mqtt::handler > > handlers;
	}
	
	// class MQTTPtr : public std::shared_ptr<zpt::MQTT> {
	// public:
	// 	MQTTPtr();
	// 	MQTTPtr(zpt::MQTT*);
	// 	virtual ~MQTTPtr();
	// };

	//typedef std::shared_ptr<zpt::MQTT> MQTTPtr;
	
	class MQTT {
	public:
		/**
		 * Init mosquitto.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_init
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_new
		 */
		MQTT();
		/**
		 * Destroy and clean up.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_destroy
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_lib_cleanup
		 */
		virtual ~MQTT();
		
		auto unbind() -> void;
		
		/**
		 * Sets MQTT server access credentials.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_username_pw_set
		 */
		virtual auto credentials(std::string _user, std::string _passwd) -> void;
		
		virtual auto user() -> std::string;
		virtual auto passwd() -> std::string;
		virtual auto self() const -> zpt::mqtt::broker;

		/**
		 * Connects to the MQTT server.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_connect
		 */
		virtual auto connect(std::string _host, bool _tls = false, int _port = 1883, int _keep_alive = 25) ->  void;
		virtual auto reconnect() ->  void;

		/**
		 * Subscribes to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic subscription patterns.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_subscribe
		 */
		virtual auto subscribe(std::string _topic) -> void;

		/**
		 * Publishes a message to a given topic. See also http://mosquitto.org/man/mqtt-7.html for topic subscription patterns.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_publish
		 */
		virtual auto publish(std::string _topic, zpt::json _payload) -> void;

		/**
		 * Add to the callback list, the callback *_callback*, attached to the event type *_event*.
		 */
		virtual auto on(std::string _event, zpt::mqtt::handler _callback) -> void;

		/**
		 * Remove from the callback list, the callback *_callback*, attached to the event type *_event*.
		 */
		virtual auto off(std::string _event) -> void;

		/**
		 * Searches for and executes registered callbacks under the event type *_event*.
		 */
		virtual auto trigger(std::string _event, zpt::mqtt::data _data) -> void;

		/**
		 * Checks if some data is available from MQTT server.
		 * - http://mosquitto.org/api/files/mosquitto-h.html#mosquitto_loop
		 */
		virtual auto start() -> void;

	private:
		static auto on_connect(struct mosquitto * _mosq, void * _ptr, int _rc) -> void;
		static auto on_disconnect(struct mosquitto * _mosq, void * _ptr, int _reason) -> void;
		static auto on_publish(struct mosquitto * _mosq, void * _ptr, int _mid) -> void;
		static auto on_message(struct mosquitto * _mosq, void * _ptr, const struct mosquitto_message * _message) -> void;
		static auto on_subscribe(struct mosquitto * _mosq, void * _ptr, int _mid, int _qos_count, const int * _granted_qos) -> void;
		static auto on_unsubscribe(struct mosquitto * _mosq, void * _ptr, int _mid) -> void;
		static auto on_error(struct mosquitto * _mosq, void * _ptr) -> void;

		struct mosquitto * __mosq;
		zpt::mqtt::handlers __callbacks;
		std::string __user;
		std::string __passwd;
		std::mutex __mtx;
		zpt::mqtt::broker __self;
	};
}
