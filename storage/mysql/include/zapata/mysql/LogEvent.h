/*
Copyright (c) 2016
*/

#pragma once

#include <bitset>
#include <iostream>
#include <memory>
#include <zapata/json.h>

typedef unsigned long long ulonglong;

#define MAX_DBS_IN_EVENT_MTS 16
#define NAME_LEN 64 * 3

namespace zpt {
namespace mysql {

extern uint8_t* post_header_lengths;
extern uint16_t post_header_lengths_size;

auto
arr2str(char* _arr, size_t _arr_len) -> std::string;

class LogEventHeader {
  public:
    virtual auto to_json() -> zpt::json;

    uint32_t __timestamp = 0x0;
    uint8_t __type_code = 0x0;
    uint32_t __server_id = 0x0;
    uint32_t __event_length = 0x0;
    uint32_t __next_position = 0x0;
    std::bitset<16> __flags = 0x0;
};

class LogEventPtr;
class LogEventHeaderPtr;

typedef zpt::mysql::LogEventPtr event;
typedef zpt::mysql::LogEventHeaderPtr event_header;

enum LogEventType {
    UNKNOWN_EVENT = 0,
    START_EVENT_V3 = 1,
    QUERY_EVENT = 2,
    STOP_EVENT = 3,
    ROTATE_EVENT = 4,
    INTVAR_EVENT = 5,
    LOAD_EVENT = 6,
    SLAVE_EVENT = 7,
    CREATE_FILE_EVENT = 8,
    APPEND_BLOCK_EVENT = 9,
    EXEC_LOAD_EVENT = 10,
    DELETE_FILE_EVENT = 11,
    NEW_LOAD_EVENT = 12,
    RAND_EVENT = 13,
    USER_VAR_EVENT = 14,
    FORMAT_DESCRIPTION_EVENT = 15,
    XID_EVENT = 16,
    BEGIN_LOAD_QUERY_EVENT = 17,
    EXECUTE_LOAD_QUERY_EVENT = 18,
    TABLE_MAP_EVENT = 19,
    PRE_GA_WRITE_ROWS_EVENT = 20,
    PRE_GA_UPDATE_ROWS_EVENT = 21,
    PRE_GA_DELETE_ROWS_EVENT = 22,
    WRITE_ROWS_EVENT = 23,
    UPDATE_ROWS_EVENT = 24,
    DELETE_ROWS_EVENT = 25,
    INCIDENT_EVENT = 26,
    HEARTBEAT_LOG_EVENT = 27,
    IGNORABLE_LOG_EVENT = 28,
    ROWS_QUERY_LOG_EVENT = 29,
    O_WRITE_ROWS_EVENT = 30,
    O_UPDATE_ROWS_EVENT = 31,
    O_DELETE_ROWS_EVENT = 32,
    GTID_LOG_EVENT = 33,
    ANONYMOUS_GTID_LOG_EVENT = 34,
    PREVIOUS_GTIDS_LOG_EVENT = 35
};

extern std::string event_names[];

namespace lengths {
extern size_t magic_number;
extern size_t header;
extern size_t timestamp;
extern size_t type_code;
extern size_t server_id;
extern size_t event_length;
extern size_t next_position;
extern size_t flags;

namespace fixed {
extern size_t start_event;
extern size_t query_event;
} // namespace fixed
} // namespace lengths

class LogEventHeaderPtr : public std::shared_ptr<zpt::mysql::LogEventHeader> {
  public:
    LogEventHeaderPtr();
    virtual ~LogEventHeaderPtr();

    inline friend std::istream& operator>>(std::istream& _in, zpt::mysql::event_header& _out) {
        _in.read((char*)&_out->__timestamp, zpt::mysql::lengths::timestamp);
        _in.read((char*)&_out->__type_code, zpt::mysql::lengths::type_code);
        _in.read((char*)&_out->__server_id, zpt::mysql::lengths::server_id);
        _in.read((char*)&_out->__event_length, zpt::mysql::lengths::event_length);
        _in.read((char*)&_out->__next_position, zpt::mysql::lengths::next_position);
        _in.read((char*)&_out->__flags, zpt::mysql::lengths::flags);
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

    uint16_t __binlog_version = 0x0;
    std::string __server_version;
    uint32_t __timestamp = 0x0;
};

enum QueryVarType {
    Q_NONE = -1,
    Q_FLAGS2_CODE = 0,   // Value is a 4-byte bit-field. This variable is written
                         // only as of MySQL 5.0.
    Q_SQL_MODE_CODE = 1, // Value is an 8-byte SQL mode value.
    Q_CATALOG_CODE = 2,  // Value is the catalog name: a length byte followed by
                         // that many bytes,
                         // plus a terminating
    // null byte. This variable is present only in MySQL 5.0.0 to 5.0.3. It was
    // replaced with
    // Q_CATALOG_NZ_CODE in MySQL 5.0.4 because the terminating null is
    // unnecessary.
    Q_AUTO_INCREMENT = 3, // Value is two 2-byte unsigned integers representing
                          // the auto_increment_increment and
    // auto_increment_offset system variables. This variable is present only if
    // auto_increment
    // is greater than 1.
    Q_CHARSET_CODE = 4, // Value is three 2-byte unsigned integers representing
                        // the character_set_client,
    // collation_connection, and collation_server system variables.
    Q_TIME_ZONE_CODE = 5, // Value is the time zone name: a length byte followed
                          // by that many bytes. This variable
    // is present only if the time zone string is non-empty.
    Q_CATALOG_NZ_CODE = 6, // Value is the catalog name: a length byte followed by
                           // that many bytes. Value is always
    // std. This variable is present only if the catalog name is non-empty.
    Q_LC_TIME_NAMES_CODE = 7, // Value is a 2-byte unsigned integer representing
                              // the lc_time_names number. This
    // variable is present only if the value is not 0 (that is, not en_US).
    Q_CHARSET_DATABASE_CODE = 8, // Value is a 2-byte unsigned integer representing the
                                 // collation_database system variable.
    Q_TABLE_MAP_FOR_UPDATE_CODE =
      9, // Value is 8 bytes representing the table map to be updated by a
    // multiple-table update statement. Each bit of this variable represents a
    // table, and is set to 1 if the corresponding table is to be updated by the
    // statement.
    Q_MASTER_DATA_WRITTEN_CODE = 10,
    Q_INVOKER = 11,
    Q_UPDATED_DB_NAMES = 12,
    Q_MICROSECONDS = 13,
    Q_COMMIT_TS = 14,
    Q_COMMIT_TS2 = 15,
    Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP = 16
};

extern std::string query_var_names[];

typedef struct q_variables {
    q_variables()
      : __type(zpt::mysql::Q_NONE){};
    q_variables(q_variables const& _other) {
        __type = _other.__type;
        switch (_other.__type) {
            case zpt::mysql::Q_NONE: {
                break;
            }
            case zpt::mysql::Q_FLAGS2_CODE: { // Value is a 4-byte bit-field. This
                                              // variable is written
                // only as of
                // MySQL 5.0.
                new (&__flags2_code) std::bitset<32>;
                __flags2_code = _other.__flags2_code;
                break;
            }
            case zpt::mysql::Q_SQL_MODE_CODE: { // Value is an 8-byte SQL mode value.
                __sql_mode_code = _other.__sql_mode_code;
                break;
            }
            case zpt::mysql::Q_CATALOG_CODE: { // Value is the catalog name: a length
                                               // byte followed by
                // that many
                // bytes, plus a terminating null byte. This variable is present only
                // in MySQL 5.0.0 to 5.0.3. It was replaced with Q_CATALOG_NZ_CODE in
                // MySQL 5.0.4 because the terminating null is unnecessary.
                new (&__catalog_code) std::string;
                __catalog_code.assign(_other.__catalog_code);
                break;
            }
            case zpt::mysql::Q_AUTO_INCREMENT: { // Value is two 2-byte unsigned
                                                 // integers representing
                // the
                // auto_increment_increment and auto_increment_offset system
                // variables. This variable is present only if auto_increment is
                // greater than 1.
                new (&__auto_increment) std::tuple<uint16_t, uint16_t>;
                std::get<0>(__auto_increment) = std::get<0>(_other.__auto_increment);
                std::get<1>(__auto_increment) = std::get<1>(_other.__auto_increment);
                break;
            }
            case zpt::mysql::Q_CHARSET_CODE: { // Value is three 2-byte unsigned
                                               // integers representing
                // the
                // character_set_client, collation_connection, and collation_server
                // system variables.
                new (&__charset_code) std::tuple<uint16_t, uint16_t, uint16_t>;
                std::get<0>(__charset_code) = std::get<0>(_other.__charset_code);
                std::get<1>(__charset_code) = std::get<1>(_other.__charset_code);
                std::get<2>(__charset_code) = std::get<2>(_other.__charset_code);
                break;
            }
            case zpt::mysql::Q_TIME_ZONE_CODE: { // Value is the time zone name: a
                                                 // length byte followed
                // by that many
                // bytes. This variable is present only if the time zone string is
                // non-empty.
                new (&__time_zone_code) std::string;
                __time_zone_code.assign(_other.__time_zone_code);
                break;
            }
            case zpt::mysql::Q_CATALOG_NZ_CODE: { // Value is the catalog name: a length
                                                  // byte followed
                // by that many
                // bytes. Value is always std. This variable is present only if
                // the catalog name is non-empty.
                new (&__catalog_nz_code) std::string;
                __catalog_nz_code.assign(_other.__catalog_nz_code);
                break;
            }
            case zpt::mysql::Q_LC_TIME_NAMES_CODE: { // Value is a 2-byte unsigned
                                                     // integer representing
                // the
                // lc_time_names number. This variable is present only if the
                // value is not 0 (that is, not en_US).
                __lc_time_names_code = _other.__lc_time_names_code;
                break;
            }
            case zpt::mysql::Q_CHARSET_DATABASE_CODE: { // Value is a 2-byte unsigned
                                                        // integer
                // representing the
                // collation_database system variable.
                __charset_database_code = _other.__charset_database_code;
                break;
            }
            case zpt::mysql::Q_TABLE_MAP_FOR_UPDATE_CODE: { // Value is 8 bytes
                                                            // representing the table
                // map to be
                // updated by a multiple-table update statement. Each
                // bit of this variable represents a table, and is set
                // to 1 if the corresponding table is to be updated by
                // the statement.
                __table_map_for_update_code = _other.__table_map_for_update_code;
                break;
            }
            case zpt::mysql::Q_MASTER_DATA_WRITTEN_CODE: {
                __master_data_written_code = _other.__master_data_written_code;
                break;
            }
            case zpt::mysql::Q_INVOKER: {
                new (&__invoker) std::tuple<std::string, std::string>;
                std::get<0>(__invoker) = std::get<0>(_other.__invoker);
                std::get<1>(__invoker) = std::get<1>(_other.__invoker);
                break;
            }
            case zpt::mysql::Q_UPDATED_DB_NAMES: {
                new (&__updated_db_names) std::vector<std::string>;
                __updated_db_names = _other.__updated_db_names;
                break;
            }
            case zpt::mysql::Q_MICROSECONDS: {
                break;
            }
            case zpt::mysql::Q_COMMIT_TS: {
                __commit_ts = _other.__commit_ts;
                break;
            }
            case zpt::mysql::Q_COMMIT_TS2: {
                __commit_ts2 = _other.__commit_ts2;
                break;
            }
            case zpt::mysql::Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP: {
                break;
            }
        }
    };
    q_variables(q_variables&& _other) {
        __type = _other.__type;
        switch (_other.__type) {
            case zpt::mysql::Q_NONE: {
                break;
            }
            case zpt::mysql::Q_FLAGS2_CODE: { // Value is a 4-byte bit-field. This
                                              // variable is written
                // only as of
                // MySQL 5.0.
                new (&__flags2_code) std::bitset<32>;
                __flags2_code = _other.__flags2_code;
                break;
            }
            case zpt::mysql::Q_SQL_MODE_CODE: { // Value is an 8-byte SQL mode value.
                __sql_mode_code = _other.__sql_mode_code;
                break;
            }
            case zpt::mysql::Q_CATALOG_CODE: { // Value is the catalog name: a length
                                               // byte followed by
                // that many
                // bytes, plus a terminating null byte. This variable is present only
                // in MySQL 5.0.0 to 5.0.3. It was replaced with Q_CATALOG_NZ_CODE in
                // MySQL 5.0.4 because the terminating null is unnecessary.
                new (&__catalog_code) std::string;
                __catalog_code.assign(_other.__catalog_code);
                break;
            }
            case zpt::mysql::Q_AUTO_INCREMENT: { // Value is two 2-byte unsigned
                                                 // integers representing
                // the
                // auto_increment_increment and auto_increment_offset system
                // variables. This variable is present only if auto_increment is
                // greater than 1.
                new (&__auto_increment) std::tuple<uint16_t, uint16_t>;
                std::get<0>(__auto_increment) = std::get<0>(_other.__auto_increment);
                std::get<1>(__auto_increment) = std::get<1>(_other.__auto_increment);
                break;
            }
            case zpt::mysql::Q_CHARSET_CODE: { // Value is three 2-byte unsigned
                                               // integers representing
                // the
                // character_set_client, collation_connection, and collation_server
                // system variables.
                new (&__charset_code) std::tuple<uint16_t, uint16_t, uint16_t>;
                std::get<0>(__charset_code) = std::get<0>(_other.__charset_code);
                std::get<1>(__charset_code) = std::get<1>(_other.__charset_code);
                std::get<2>(__charset_code) = std::get<2>(_other.__charset_code);
                break;
            }
            case zpt::mysql::Q_TIME_ZONE_CODE: { // Value is the time zone name: a
                                                 // length byte followed
                // by that many
                // bytes. This variable is present only if the time zone string is
                // non-empty.
                new (&__time_zone_code) std::string;
                __time_zone_code.assign(_other.__time_zone_code);
                break;
            }
            case zpt::mysql::Q_CATALOG_NZ_CODE: { // Value is the catalog name: a length
                                                  // byte followed
                // by that many
                // bytes. Value is always std. This variable is present only if
                // the catalog name is non-empty.
                new (&__catalog_nz_code) std::string;
                __catalog_nz_code.assign(_other.__catalog_nz_code);
                break;
            }
            case zpt::mysql::Q_LC_TIME_NAMES_CODE: { // Value is a 2-byte unsigned
                                                     // integer representing
                // the
                // lc_time_names number. This variable is present only if the
                // value is not 0 (that is, not en_US).
                __lc_time_names_code = _other.__lc_time_names_code;
                break;
            }
            case zpt::mysql::Q_CHARSET_DATABASE_CODE: { // Value is a 2-byte unsigned
                                                        // integer
                // representing the
                // collation_database system variable.
                __charset_database_code = _other.__charset_database_code;
                break;
            }
            case zpt::mysql::Q_TABLE_MAP_FOR_UPDATE_CODE: { // Value is 8 bytes
                                                            // representing the table
                // map to be
                // updated by a multiple-table update statement. Each
                // bit of this variable represents a table, and is set
                // to 1 if the corresponding table is to be updated by
                // the statement.
                __table_map_for_update_code = _other.__table_map_for_update_code;
                break;
            }
            case zpt::mysql::Q_MASTER_DATA_WRITTEN_CODE: {
                __master_data_written_code = _other.__master_data_written_code;
                break;
            }
            case zpt::mysql::Q_INVOKER: {
                new (&__invoker) std::tuple<std::string, std::string>;
                std::get<0>(__invoker) = std::get<0>(_other.__invoker);
                std::get<1>(__invoker) = std::get<1>(_other.__invoker);
                break;
            }
            case zpt::mysql::Q_UPDATED_DB_NAMES: {
                new (&__updated_db_names) std::vector<std::string>;
                __updated_db_names = _other.__updated_db_names;
                break;
            }
            case zpt::mysql::Q_MICROSECONDS: {
                break;
            }
            case zpt::mysql::Q_COMMIT_TS: {
                __commit_ts = _other.__commit_ts;
                break;
            }
            case zpt::mysql::Q_COMMIT_TS2: {
                __commit_ts2 = _other.__commit_ts2;
                break;
            }
            case zpt::mysql::Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP: {
                break;
            }
        }
    };
    ~q_variables() {
        switch (__type) {
            case zpt::mysql::Q_FLAGS2_CODE: {
                __flags2_code.~bitset<32>();
                break;
            }
            case zpt::mysql::Q_CATALOG_CODE: {
                __catalog_code.~string();
                break;
            }
            case zpt::mysql::Q_AUTO_INCREMENT: {
                __auto_increment.~tuple<uint16_t, uint16_t>();
                break;
            }
            case zpt::mysql::Q_CHARSET_CODE: {
                __charset_code.~tuple<uint16_t, uint16_t, uint16_t>();
                break;
            }
            case zpt::mysql::Q_TIME_ZONE_CODE: {
                __time_zone_code.~string();
                break;
            }
            case zpt::mysql::Q_CATALOG_NZ_CODE: {
                __catalog_nz_code.~string();
                break;
            }
            case zpt::mysql::Q_INVOKER: {
                __invoker.~tuple<std::string, std::string>();
                break;
            }
            case zpt::mysql::Q_UPDATED_DB_NAMES: {
                __updated_db_names.~vector<std::string>();
                break;
            }
            default: {
                break;
            }
        }
    };

    zpt::json to_json() {
        switch (__type) {
            case zpt::mysql::Q_NONE: {
                return zpt::undefined;
            }
            case zpt::mysql::Q_FLAGS2_CODE: {
                return zpt::json::string(__flags2_code.to_string());
            }
            case zpt::mysql::Q_SQL_MODE_CODE: {
                return zpt::json::ulong(__sql_mode_code);
            }
            case zpt::mysql::Q_CATALOG_CODE: {
                return zpt::json::string(__catalog_code);
            }
            case zpt::mysql::Q_AUTO_INCREMENT: {
                return { "auto_increment_increment",
                         std::get<0>(__auto_increment),
                         "auto_increment_offset",
                         std::get<1>(__auto_increment) };
            }
            case zpt::mysql::Q_CHARSET_CODE: {
                return { "character_set_client", std::get<0>(__charset_code),
                         "collation_connection", std::get<1>(__charset_code),
                         "collation_server",     std::get<2>(__charset_code) };
            }
            case zpt::mysql::Q_TIME_ZONE_CODE: {
                return zpt::json::string(__time_zone_code);
            }
            case zpt::mysql::Q_CATALOG_NZ_CODE: {
                return zpt::json::string(__catalog_nz_code);
            }
            case zpt::mysql::Q_LC_TIME_NAMES_CODE: {
                return zpt::json::ulong(__lc_time_names_code);
            }
            case zpt::mysql::Q_CHARSET_DATABASE_CODE: {
                return zpt::json::ulong(__charset_database_code);
            }
            case zpt::mysql::Q_TABLE_MAP_FOR_UPDATE_CODE: {
                return zpt::json::ulong(__table_map_for_update_code);
            }
            case zpt::mysql::Q_MASTER_DATA_WRITTEN_CODE: {
                return zpt::json::ulong(__master_data_written_code);
            }
            case zpt::mysql::Q_INVOKER: {
                return { "user", std::get<0>(__invoker), "host", std::get<1>(__invoker) };
            }
            case zpt::mysql::Q_UPDATED_DB_NAMES: {
                zpt::json _ret = zpt::json::array();
                for (auto _e : __updated_db_names) {
                    _ret << _e;
                }
                return _ret;
            }
            case zpt::mysql::Q_MICROSECONDS: {
                break;
            }
            case zpt::mysql::Q_COMMIT_TS: {
                return zpt::json::ulong(__commit_ts);
                break;
            }
            case zpt::mysql::Q_COMMIT_TS2: {
                return zpt::json::ulong(__commit_ts2);
                break;
            }
            case zpt::mysql::Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP: {
                break;
            }
        }
        return zpt::undefined;
    };

    zpt::mysql::QueryVarType __type;
    union {
        std::bitset<32> __flags2_code = 0x0;
        uint64_t __sql_mode_code;
        std::string __catalog_code;
        std::tuple<uint16_t, uint16_t> __auto_increment;
        std::tuple<uint16_t, uint16_t, uint16_t> __charset_code;
        std::string __time_zone_code;
        std::string __catalog_nz_code;
        uint16_t __lc_time_names_code;
        uint16_t __charset_database_code;
        uint64_t __table_map_for_update_code;
        uint32_t __master_data_written_code;
        std::tuple<std::string, std::string> __invoker;
        std::vector<std::string> __updated_db_names;
        uint64_t __microseconds;
        uint64_t __commit_ts;
        uint64_t __commit_ts2;
        uint64_t __explicit_defaults_for_timestam;
    };
} query_variable;

class QueryEvent : public zpt::mysql::LogEvent {
  public:
    virtual auto data_to_json() -> zpt::json;
    static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;

    uint32_t __thread_id = 0x0;
    uint32_t __execution_time = 0x0;
    uint8_t __database_name_length = 0x0;
    uint16_t __error_code = 0x0;
    uint16_t __variable_length = 0x0;
    std::string __database_name;
    std::string __query;
    std::vector<zpt::mysql::query_variable> __variables;
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

    uint64_t __position_next_event;
    std::string __next_binlog_filename;
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

    uint16_t __binlog_version = 0x0;
    std::string __server_version;
    uint32_t __timestamp = 0x0;
    uint8_t __header_length = 0x0;
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

    uint64_t __table_id;
    uint64_t __number_of_columns;
    std::vector<bool> __nullable_columns;
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

    uint64_t __table_id;
    uint64_t __number_of_columns;
    std::vector<bool> __nullable_columns;
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

class GTIDLogEvent : public zpt::mysql::LogEvent {
  public:
    virtual auto data_to_json() -> zpt::json;
    static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
};

class AnonymousGTIDLogEvent : public zpt::mysql::LogEvent {
  public:
    virtual auto data_to_json() -> zpt::json;
    static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
};

class PreviousGTIDSLogEvent : public zpt::mysql::LogEvent {
  public:
    virtual auto data_to_json() -> zpt::json;
    static auto consume(zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event;
};

extern std::vector<
  std::function<zpt::mysql::event(zpt::mysql::event_header _header, std::istream& _in)>>
  consumers;

class LogEventPtr : public std::shared_ptr<zpt::mysql::LogEvent> {
  public:
    LogEventPtr();
    LogEventPtr(zpt::mysql::LogEvent* _target);
    virtual ~LogEventPtr();

    inline friend auto operator>>(std::istream& _in, zpt::mysql::event& _out) -> std::istream& {
        if (_out.get() == nullptr) {
            zpt::mysql::event_header _header;
            _in >> _header;
            zdbg(std::string("type_code: ") + std::to_string(_header->__type_code));
            if (_header->__type_code == 0) {
                return _in;
            }
            _out = zpt::mysql::consumers[(size_t)_header->__type_code](_header, _in);
        }
        else {
            do {
                zpt::mysql::event_header _header;
                _in >> _header;
                if (_header->__type_code == _out->header()->__type_code) {
                    _out = zpt::mysql::consumers[(size_t)_header->__type_code](_header, _in);
                    break;
                }
                else {
                    char _dispose[_header->__event_length - 19];
                    _in.read(_dispose, _header->__event_length - 19);
                }
            } while (true);
        }
        return _in;
    };

    static auto instance(zpt::mysql::LogEventType _type) -> zpt::mysql::event;
    static auto register_callback(
      zpt::mysql::LogEventType _event_type,
      std::function<zpt::mysql::event(zpt::mysql::event_header, std::istream& _in)> _consumer)
      -> void;
};

class magic_number {
  public:
    inline friend auto operator>>(std::istream& _in, zpt::mysql::magic_number& _out)
      -> std::istream& {
        char _byte[zpt::mysql::lengths::magic_number] = { 0 };
        _in.read(_byte, zpt::mysql::lengths::magic_number);
        return _in;
    };
};
} // namespace mysql
} // namespace zpt
