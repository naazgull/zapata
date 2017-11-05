/*
Copyright (c) 2016
*/

#pragma once

#include <iostream>
#include <memory>
#include <bitset>
#include <zapata/json.h>

typedef unsigned long long ulonglong;

namespace zpt {
	namespace mysql {

		class LogEventHeader {
		public:
			virtual auto to_json() -> zpt::json;
			
			ulong __timestamp = 0x0;
			ushort __type_code = 0x0;
			uint __server_id = 0x0;
			uint __event_length = 0x0;
			uint __next_position = 0x0;
			std::bitset< 16 > __flags = 0x0;			
		};

		class LogEventPtr;
		class LogEventHeaderPtr;
		
		typedef zpt::mysql::LogEventPtr event;
		typedef zpt::mysql::LogEventHeaderPtr event_header;
		
		enum LogEventType { 
			UNKNOWN_EVENT= 0, 
			START_EVENT_V3= 1, 
			QUERY_EVENT= 2, 
			STOP_EVENT= 3, 
			ROTATE_EVENT= 4, 
			INTVAR_EVENT= 5, 
			LOAD_EVENT= 6, 
			SLAVE_EVENT= 7, 
			CREATE_FILE_EVENT= 8, 
			APPEND_BLOCK_EVENT= 9, 
			EXEC_LOAD_EVENT= 10, 
			DELETE_FILE_EVENT= 11, 
			NEW_LOAD_EVENT= 12, 
			RAND_EVENT= 13, 
			USER_VAR_EVENT= 14, 
			FORMAT_DESCRIPTION_EVENT= 15, 
			XID_EVENT= 16, 
			BEGIN_LOAD_QUERY_EVENT= 17, 
			EXECUTE_LOAD_QUERY_EVENT= 18, 
			TABLE_MAP_EVENT = 19, 
			PRE_GA_WRITE_ROWS_EVENT = 20, 
			PRE_GA_UPDATE_ROWS_EVENT = 21, 
			PRE_GA_DELETE_ROWS_EVENT = 22, 
			WRITE_ROWS_EVENT = 23, 
			UPDATE_ROWS_EVENT = 24, 
			DELETE_ROWS_EVENT = 25, 
			INCIDENT_EVENT= 26, 
			HEARTBEAT_LOG_EVENT= 27, 
			IGNORABLE_LOG_EVENT= 28,
			ROWS_QUERY_LOG_EVENT= 29
		};

		namespace lengths {
			size_t magic_number = 4;
			size_t timestamp = 4;
			size_t type_code = 1;
			size_t server_id = 4;
			size_t event_length = 4;
			size_t next_position = 4;
			size_t flags = 2;

			namespace fixed {
				size_t start_event = 56;
				
			}
		}
		
		class LogEventHeaderPtr : public std::shared_ptr< zpt::mysql::LogEventHeader > {
		public:
			LogEventHeaderPtr();
			virtual ~LogEventHeaderPtr();
			
			inline friend std::istream& operator>>(std::istream& _in, zpt::mysql::event_header& _out) {
				_in.read((char*) &_out->__timestamp, zpt::mysql::lengths::timestamp);
				_in.read((char*) &_out->__type_code, zpt::mysql::lengths::type_code);
				_in.read((char*) &_out->__server_id, zpt::mysql::lengths::server_id);
				_in.read((char*) &_out->__event_length, zpt::mysql::lengths::event_length);
				_in.read((char*) &_out->__next_position, zpt::mysql::lengths::next_position);
				_in.read((char*) &_out->__flags, zpt::mysql::lengths::flags);
				return _in;
			};
		};
		
		class LogEvent {
			/*
			  +=====================================+
			  | event  | timestamp         0 : 4    |
			  | header +----------------------------+
			  |        | type_code         4 : 1    |
			  |        +----------------------------+
			  |        | server_id         5 : 4    |
			  |        +----------------------------+
			  |        | event_length      9 : 4    |
			  |        +----------------------------+
			  |        | next_position    13 : 4    |
			  |        +----------------------------+
			  |        | flags            17 : 2    |
			  |        +----------------------------+
			  |        | extra_headers    19 : x-19 |
			  +=====================================+
			  | event  | fixed part        x : y    |
			  | data   +----------------------------+
			  |        | variable part              |
			  +=====================================+
			 */
		public:
			virtual auto header() -> zpt::mysql::event_header;
			virtual auto header(zpt::mysql::event_header _header) -> void;

			virtual auto to_json() -> zpt::json;
			virtual auto data_to_json() -> zpt::json = 0;
			
		private:
			zpt::mysql::event_header __header;
		};

		class StartEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
			
			uint __binlog_version = 0x0;
			std::string __server_version;
			ulong __timestamp = 0x0;
		};

		enum QueryVarType {
			Q_NONE = -1,
			Q_FLAGS2_CODE = 0, // Value is a 4-byte bit-field. This variable is written only as of MySQL 5.0.
			Q_SQL_MODE_CODE = 1, // Value is an 8-byte SQL mode value.
			Q_CATALOG_CODE = 2, // Value is the catalog name: a length byte followed by that many bytes, plus a terminating null byte. This variable is present only in MySQL 5.0.0 to 5.0.3. It was replaced with Q_CATALOG_NZ_CODE in MySQL 5.0.4 because the terminating null is unnecessary.
			Q_AUTO_INCREMENT = 3, // Value is two 2-byte unsigned integers representing the auto_increment_increment and auto_increment_offset system variables. This variable is present only if auto_increment is greater than 1.
			Q_CHARSET_CODE = 4, // Value is three 2-byte unsigned integers representing the character_set_client, collation_connection, and collation_server system variables.
			Q_TIME_ZONE_CODE = 5, // Value is the time zone name: a length byte followed by that many bytes. This variable is present only if the time zone string is non-empty.
			Q_CATALOG_NZ_CODE = 6, // Value is the catalog name: a length byte followed by that many bytes. Value is always std. This variable is present only if the catalog name is non-empty.
			Q_LC_TIME_NAMES_CODE = 7, // Value is a 2-byte unsigned integer representing the lc_time_names number. This variable is present only if the value is not 0 (that is, not en_US).
			Q_CHARSET_DATABASE_CODE = 8, // Value is a 2-byte unsigned integer representing the collation_database system variable.
			Q_TABLE_MAP_FOR_UPDATE_CODE = 9 // Value is 8 bytes representing the table map to be updated by a multiple-table update statement. Each bit of this variable represents a table, and is set to 1 if the corresponding table is to be updated by the statement.
		};
		
		class QueryEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;

			typedef struct q_variables {
				q_variables() : __type(zpt::mysql::Q_NONE) { };
				~q_variables() { 
					switch(__type) {
						case zpt::mysql::Q_CATALOG_CODE : {
							__catalog_code.~string();
							break;
						}
						case zpt::mysql::Q_AUTO_INCREMENT : {
							__auto_increment.~tuple< uint, uint>();
							break;
						}
						case zpt::mysql::Q_CHARSET_CODE : {
							__charset_code.~tuple< uint, uint, uint>();
							break;
						}
						case zpt::mysql::Q_TIME_ZONE_CODE : {
							__time_zone_code.~string();
							break;
						}
						case zpt::mysql::Q_CATALOG_NZ_CODE : {
							__catalog_nz_code.~string();
							break;
						}
						default : {
							break;
						}
					} 
				};

				q_variables(q_variables const&) = delete;
				q_variables& operator=(q_variables const&) = delete;

				q_variables(q_variables&&) = delete;
				q_variables& operator=(q_variables&&) = delete;
				
				zpt::mysql::QueryVarType __type;
				union {
					ulong __flags2_code;
					ulonglong __sql_mode_code;
					std::string __catalog_code;
					std::tuple< uint, uint> __auto_increment;
					std::tuple< uint, uint, uint> __charset_code;
					std::string __time_zone_code;
					std::string __catalog_nz_code;
					uint __lc_time_names_code;
					uint __charset_database_code;
					ulonglong __table_map_for_update_code;
				};
			} query_variables;

			ulong __thread_id = 0x0;
			ulong __execution_time = 0x0;
			ushort __database_name_length = 0x0;
			uint __error_code = 0x0;
			uint __variable_length = 0x0;
			std::vector< query_variables > __variables;
		};

		class StopEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class RotateEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class IntvarEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class LoadEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class SlaveEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class CreateFileEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class AppendBlockEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class ExecLoadEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class DeleteFileEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class NewLoadEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class RandEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class UserVarEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class FormatDescriptionEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
			
			uint __binlog_version = 0x0;
			std::string __server_version;
			ulong __timestamp = 0x0;
			ushort __header_length = 0x0;
			std::string __post_header_lengths;
		};

		class XidEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class BeginLoadQueryEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class ExecuteLoadQueryEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class TableMapEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class PreGAWriteRowsEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class PreGAUpdateRowsEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class PreGADeleteRowsEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class WriteRowsEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class UpdateRowsEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class DeleteRowsEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		class IncidentEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};
		
		class HeartbeatLogEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};
		
		class IgnorableLogEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};
		
		class RowsQueryLogEvent : public zpt::mysql::LogEvent {
		public:
			virtual auto data_to_json() -> zpt::json;
			static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
		};

		std::vector< std::function< zpt::mysql::event (zpt::mysql::event_header _header, std::istream& _in) > > consumers = {
			nullptr, 
			zpt::mysql::StartEvent::consume, 
			zpt::mysql::QueryEvent::consume, 
			zpt::mysql::StopEvent::consume, 
			zpt::mysql::RotateEvent::consume, 
			zpt::mysql::IntvarEvent::consume, 
			zpt::mysql::LoadEvent::consume, 
			zpt::mysql::SlaveEvent::consume, 
			zpt::mysql::CreateFileEvent::consume, 
			zpt::mysql::AppendBlockEvent::consume, 
			zpt::mysql::ExecLoadEvent::consume, 
			zpt::mysql::DeleteFileEvent::consume, 
			zpt::mysql::NewLoadEvent::consume, 
			zpt::mysql::RandEvent::consume, 
			zpt::mysql::UserVarEvent::consume, 
			zpt::mysql::FormatDescriptionEvent::consume, 
			zpt::mysql::XidEvent::consume, 
			zpt::mysql::BeginLoadQueryEvent::consume, 
			zpt::mysql::ExecuteLoadQueryEvent::consume, 
			zpt::mysql::TableMapEvent::consume, 
			zpt::mysql::PreGAWriteRowsEvent::consume, 
			zpt::mysql::PreGAUpdateRowsEvent::consume, 
			zpt::mysql::PreGADeleteRowsEvent::consume, 
			zpt::mysql::WriteRowsEvent::consume, 
			zpt::mysql::UpdateRowsEvent::consume, 
			zpt::mysql::DeleteRowsEvent::consume, 
			zpt::mysql::IncidentEvent::consume, 
			zpt::mysql::HeartbeatLogEvent::consume, 
			zpt::mysql::IgnorableLogEvent::consume,
			zpt::mysql::RowsQueryLogEvent::consume
		};
		
		class LogEventPtr : public std::shared_ptr< zpt::mysql::LogEvent > {
		public:
			LogEventPtr();
			LogEventPtr(zpt::mysql::LogEvent* _target);
			virtual ~LogEventPtr();

			inline friend auto operator>>(std::istream& _in, zpt::mysql::event& _out) -> std::istream& {
				if (_out.get() == nullptr) {
					zpt::mysql::event_header _header;
					_in >> _header;
					_out = zpt::mysql::consumers[(size_t) _header->__type_code](_header, _in);
				}
				else {
					do {
						zpt::mysql::event_header _header;
						_in >> _header;
						if (_header->__type_code == _out->header()->__type_code) {
							_out = zpt::mysql::consumers[(size_t) _header->__type_code](_header, _in);
							break;
						}
						else {
							char _dispose[_header->__event_length - 19];
							_in.read(_dispose, _header->__event_length - 19);
						}
					}
					while(true);
				}
				return _in;
			};

			static auto instance(zpt::mysql::LogEventType _type) -> zpt::mysql::event;

		};

		class magic_number {
		public:
			inline friend auto operator>>(std::istream& _in, zpt::mysql::magic_number& _out) -> std::istream& {
				char _byte[zpt::mysql::lengths::magic_number] = { 0 };
				_in.read(_byte, zpt::mysql::lengths::magic_number);
				return _in;
			};
		};

	}

}
