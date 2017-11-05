#include <zapata/mysql/LogEvent.h>

zpt::mysql::LogEventPtr::LogEventPtr() : std::shared_ptr< zpt::mysql::LogEvent >(nullptr) {
}

zpt::mysql::LogEventPtr::LogEventPtr(zpt::mysql::LogEvent* _target) : std::shared_ptr< zpt::mysql::LogEvent >(_target) {
}

zpt::mysql::LogEventPtr::~LogEventPtr() {
}

auto zpt::mysql::LogEventPtr::instance(zpt::mysql::LogEventType _type) -> zpt::mysql::event {
	zpt::mysql::event_header _header;
	_header->__type_code = (uint) _type;
	return zpt::mysql::consumers[(size_t) _type](_header, std::cin);
}

zpt::mysql::LogEventHeaderPtr::LogEventHeaderPtr() : std::shared_ptr< zpt::mysql::LogEventHeader >(new zpt::mysql::LogEventHeader()) {
}

zpt::mysql::LogEventHeaderPtr::~LogEventHeaderPtr() {
}

auto zpt::mysql::LogEventHeader::to_json() -> zpt::json {
	return {
		"timestamp", zpt::json::date(this->__timestamp * 1000),
		"type_code", this->__type_code,
		"server_id", this->__server_id,
		"event_length", this->__event_length,
		"next_position", this->__next_position,
		"flags", this->__flags.to_ulong()
	};
}

auto zpt::mysql::LogEvent::header() -> zpt::mysql::event_header {
	return this->__header;
}

auto zpt::mysql::LogEvent::header(zpt::mysql::event_header _header) -> void {
	this->__header.swap(_header);
}

auto zpt::mysql::LogEvent::to_json() -> zpt::json {
	return { "header", this->__header->to_json(), "data", this->data_to_json() };
}

auto zpt::mysql::StartEvent::data_to_json() -> zpt::json {
	return {
		"fixed", {
			"binlog_version", this->__binlog_version,
			"server_version", this->__server_version,
			"timestamp", zpt::json::date(this->__timestamp * 1000)
		},
		"variable", zpt::undefined
	};
}

auto zpt::mysql::StartEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::StartEvent* _event = new zpt::mysql::StartEvent();
	_event->header(_header);
	if (_header->__event_length) {
		char _server_version[50] = { 0 };
		
		_in.read((char*) &_event->__binlog_version, 2);
		_in.read(_server_version, 50);
		_in.read((char*) &_event->__timestamp, 4);

		_event->__server_version = std::string(_server_version, 50);
	}
	return zpt::mysql::event(_event);
}

auto zpt::mysql::QueryEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::QueryEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::QueryEvent* _event = new zpt::mysql::QueryEvent();
	_event->header(_header);
	if (_header->__event_length) {
		_in.read((char*) &_event->__thread_id, 4);
		_in.read((char*) &_event->__execution_time, 4);
		_in.read((char*) &_event->__database_name_length, 1);
		_in.read((char*) &_event->__error_code, 2);
		_in.read((char*) &_event->__variable_length, 2);

		for (size_t _idx = 0; _idx != _event->__variable_length; _idx++) {
			ushort _var_type;
			_in.read((char*) &_var_type, 1);

			switch(_var_type) {
				case zpt::mysql::Q_FLAGS2_CODE : { // Value is a 4-byte bit-field. This variable is written only as of MySQL 5.0.
					ulong _var_value;
					_in.read((char*) &_var_value, 4);
					zpt::mysql::QueryEvent::query_variables _q_var;
					_event->__variables.emplace_back((zpt::mysql::QueryVarType) _var_type, .__flags2_code = _var_value }));
					break;
				}
				case zpt::mysql::Q_SQL_MODE_CODE : { // Value is an 8-byte SQL mode value.
					ulonglong _var_value;
					_in.read((char*) &_var_value, 8);
					break;
				}
				case zpt::mysql::Q_CATALOG_CODE : { // Value is the catalog name: a length byte followed by that many bytes, plus a terminating null byte. This variable is present only in MySQL 5.0.0 to 5.0.3. It was replaced with Q_CATALOG_NZ_CODE in MySQL 5.0.4 because the terminating null is unnecessary.
					break;
				}
				case zpt::mysql::Q_AUTO_INCREMENT : { // Value is two 2-byte unsigned integers representing the auto_increment_increment and auto_increment_offset system variables. This variable is present only if auto_increment is greater than 1.
					uint _var_value_1;
					_in.read((char*) &_var_value_1, 2);
					uint _var_value_2;
					_in.read((char*) &_var_value_2, 2);
					break;
				}
				case zpt::mysql::Q_CHARSET_CODE : { // Value is three 2-byte unsigned integers representing the character_set_client, collation_connection, and collation_server system variables.
					break;
				}
				case zpt::mysql::Q_TIME_ZONE_CODE : { // Value is the time zone name: a length byte followed by that many bytes. This variable is present only if the time zone string is non-empty.
					break;
				}
				case zpt::mysql::Q_CATALOG_NZ_CODE : { // Value is the catalog name: a length byte followed by that many bytes. Value is always std. This variable is present only if the catalog name is non-empty.
					break;
				}
				case zpt::mysql::Q_LC_TIME_NAMES_CODE : { // Value is a 2-byte unsigned integer representing the lc_time_names number. This variable is present only if the value is not 0 (that is, not en_US).
					break;
				}
				case zpt::mysql::Q_CHARSET_DATABASE_CODE : { // Value is a 2-byte unsigned integer representing the collation_database system variable.
					break;
				}
				case zpt::mysql::Q_TABLE_MAP_FOR_UPDATE_CODE : {// Value is 8 bytes representing the table map to be updated by a multiple-table update statement. Each bit of this variable represents a table, and is set to 1 if the corresponding table is to be updated by the statement.
					break;
				}
			}
		}
	}
	return zpt::mysql::event(_event);
}

auto zpt::mysql::StopEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::StopEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::StopEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::RotateEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::RotateEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::RotateEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::IntvarEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::IntvarEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::IntvarEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::LoadEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::LoadEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::LoadEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::SlaveEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::SlaveEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::SlaveEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::CreateFileEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::CreateFileEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::CreateFileEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::AppendBlockEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::AppendBlockEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::AppendBlockEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::ExecLoadEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::ExecLoadEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::ExecLoadEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::DeleteFileEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::DeleteFileEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::DeleteFileEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::NewLoadEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::NewLoadEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::NewLoadEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::RandEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::RandEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::RandEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::UserVarEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::UserVarEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::UserVarEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::FormatDescriptionEvent::data_to_json() -> zpt::json {
	zpt::json _post_header_lengths = zpt::json::array();
	for (char _c : this->__post_header_lengths) {
		_post_header_lengths << (unsigned int) _c;
	}
	return {
		"fixed", {
			"binlog_version", (unsigned int) this->__binlog_version,
			"server_version", this->__server_version,
			"timestamp", zpt::json::date(this->__timestamp * 1000),
			"header_length", (unsigned int) this->__header_length,
			"post_header_lengths", _post_header_lengths
		},
		"variable", zpt::undefined
	};
}

auto zpt::mysql::FormatDescriptionEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::FormatDescriptionEvent* _event = new zpt::mysql::FormatDescriptionEvent();
	_event->header(_header);
	if (_header->__event_length) {
		char _server_version[50] = { 0 };
		size_t _post_header_lengths_size = _event->header()->__event_length - 19 - 57;
		char _post_header_lengths[_post_header_lengths_size] = { 0 };
		
		_in.read((char*) &_event->__binlog_version, 2);
		_in.read(_server_version, 50);
		_in.read((char*) &_event->__timestamp, 4);
		_in.read((char*) &_event->__header_length, 1);

		_in.read(_post_header_lengths, _post_header_lengths_size);

		_event->__server_version = std::string(_server_version, 50);
		_event->__post_header_lengths = std::string(_post_header_lengths, _post_header_lengths_size);
	}
	return zpt::mysql::event(_event);
}

auto zpt::mysql::XidEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::XidEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::XidEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::BeginLoadQueryEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::BeginLoadQueryEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::BeginLoadQueryEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::ExecuteLoadQueryEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::ExecuteLoadQueryEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::ExecuteLoadQueryEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::TableMapEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::TableMapEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::TableMapEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::PreGAWriteRowsEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::PreGAWriteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::PreGAWriteRowsEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::PreGAUpdateRowsEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::PreGAUpdateRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::PreGAUpdateRowsEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::PreGADeleteRowsEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::PreGADeleteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::PreGADeleteRowsEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::WriteRowsEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::WriteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::WriteRowsEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::UpdateRowsEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::UpdateRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::UpdateRowsEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::DeleteRowsEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::DeleteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::DeleteRowsEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::IncidentEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::IncidentEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::IncidentEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::HeartbeatLogEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::HeartbeatLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::HeartbeatLogEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::IgnorableLogEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::IgnorableLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::IgnorableLogEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto zpt::mysql::RowsQueryLogEvent::data_to_json() -> zpt::json {
	zpt::json _return = zpt::json::object();
	return _return;
}

auto zpt::mysql::RowsQueryLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
	zpt::mysql::event _event(new zpt::mysql::RowsQueryLogEvent());
	_event->header(_header);
	if (_header->__event_length) {
	}
	return _event;
}

auto consume(std::string _binlog) -> int {
	zpt::mysql::event _event;
	return 0;
}
