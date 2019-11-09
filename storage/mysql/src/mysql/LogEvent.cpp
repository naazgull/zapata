#include <zapata/mysql/LogEvent.h>

namespace zpt {
namespace mysql {

uint8_t* post_header_lengths = nullptr;
uint16_t post_header_lengths_size = 0x0;

namespace lengths {
size_t magic_number = 4;
size_t header = 19;
size_t timestamp = 4;
size_t type_code = 1;
size_t server_id = 4;
size_t event_length = 4;
size_t next_position = 4;
size_t flags = 2;

namespace fixed {
size_t start_event = 56;
size_t query_event = 13;
} // namespace fixed
} // namespace lengths
} // namespace mysql
} // namespace zpt

auto
zpt::mysql::arr2str(char* _arr, size_t _arr_len) -> std::string {
    std::ostringstream _oss;
    size_t _c = 0;
    while (_arr[_c] == 0x0) {
        _c++;
    }
    for (; _c != _arr_len; _c++) {
        _oss << _arr[_c] << std::flush;
    }
    _oss << std::flush;
    return _oss.str();
}

zpt::mysql::LogEventPtr::LogEventPtr()
  : std::shared_ptr<zpt::mysql::LogEvent>(nullptr) {}

zpt::mysql::LogEventPtr::LogEventPtr(zpt::mysql::LogEvent* _target)
  : std::shared_ptr<zpt::mysql::LogEvent>(_target) {}

zpt::mysql::LogEventPtr::~LogEventPtr() {}

auto
zpt::mysql::LogEventPtr::instance(zpt::mysql::LogEventType _type) -> zpt::mysql::event {
    zpt::mysql::event_header _header;
    _header->__type_code = (uint)_type;
    return zpt::mysql::consumers[(size_t)_type](_header, std::cin);
}

auto
zpt::mysql::LogEventPtr::register_callback(
  zpt::mysql::LogEventType _event_type,
  std::function<zpt::mysql::event(zpt::mysql::event_header, std::istream& _in)> _consumer) -> void {
    zpt::mysql::consumers[_event_type] = _consumer;
}

zpt::mysql::LogEventHeaderPtr::LogEventHeaderPtr()
  : std::shared_ptr<zpt::mysql::LogEventHeader>(new zpt::mysql::LogEventHeader()) {}

zpt::mysql::LogEventHeaderPtr::~LogEventHeaderPtr() {}

auto
zpt::mysql::LogEventHeader::to_json() -> zpt::json {
    return { "timestamp",     zpt::timestamp_t(((zpt::timestamp_t)this->__timestamp) * 1000),
             "type_code",     (unsigned int)this->__type_code,
             "server_id",     (unsigned long)this->__server_id,
             "event_length",  (unsigned long)this->__event_length,
             "next_position", (unsigned long)this->__next_position,
             "flags",         this->__flags.to_string() };
}

auto
zpt::mysql::LogEvent::header() -> zpt::mysql::event_header {
    return this->__header;
}

auto
zpt::mysql::LogEvent::header(zpt::mysql::event_header _header) -> void {
    this->__header.swap(_header);
}

auto
zpt::mysql::LogEvent::to_json() -> zpt::json {
    return { zpt::mysql::event_names[(unsigned int)this->__header->__type_code],
             { "header", this->__header->to_json(), "parts", this->data_to_json() } };
}

auto
zpt::mysql::StartEvent::data_to_json() -> zpt::json {
    return { zpt::array,
             { "binlog_version",
               this->__binlog_version,
               "server_version",
               this->__server_version,
               "timestamp",
               zpt::json::date(this->__timestamp * 1000) } };
}

auto
zpt::mysql::StartEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::StartEvent* _event = new zpt::mysql::StartEvent();
    _event->header(_header);
    if (_header->__event_length) {
        char _server_version[50] = { 0 };

        _in.read((char*)&_event->__binlog_version, sizeof(uint16_t));
        _in.read(_server_version, 50);
        _in.read((char*)&_event->__timestamp, sizeof(uint32_t));

        _event->__server_version = zpt::mysql::arr2str(_server_version, 50);
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::QueryEvent::data_to_json() -> zpt::json {
    zpt::json _status_variables = zpt::json::object();
    for (auto _v : this->__variables) {
        _status_variables << zpt::mysql::query_var_names[_v.__type] << _v.to_json();
    }
    return { zpt::array,
             {
               "thread_id",
               (unsigned long)this->__thread_id,
               "execution_time",
               (unsigned long)this->__execution_time,
               "database_name_length",
               (unsigned int)this->__database_name_length,
               "error_code",
               (unsigned int)this->__error_code,
               "variable_length",
               (unsigned int)this->__variable_length,
             },
             { "database_name",
               this->__database_name,
               "status_variables",
               _status_variables,
               "sql_statement",
               this->__query } };
}

auto
zpt::mysql::QueryEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::QueryEvent* _event = new zpt::mysql::QueryEvent();
    _event->header(_header);
    if (_header->__event_length) {
        _in.read((char*)&_event->__thread_id, sizeof(uint32_t));
        _in.read((char*)&_event->__execution_time, sizeof(uint32_t));
        _in.read((char*)&_event->__database_name_length, sizeof(uint8_t));
        _in.read((char*)&_event->__error_code, sizeof(uint16_t));
        _in.read((char*)&_event->__variable_length, sizeof(uint16_t));

        uint32_t _idx = 0;
        for (; _idx != _event->__variable_length;) {
            uint8_t _var_type = 0;
            _in.read((char*)&_var_type, sizeof(uint8_t));
            zdbg(zpt::json({ zpt::array, _idx, _var_type, _event->__variable_length }));
            _idx += sizeof(uint8_t);

            zpt::mysql::query_variable _q_var;
            _q_var.__type = (zpt::mysql::QueryVarType)_var_type;

            switch (_var_type) {
                case zpt::mysql::Q_FLAGS2_CODE: { // Value is a 4-byte bit-field. This
                                                  // variable is
                    // written only
                    // as of MySQL 5.0.
                    std::bitset<32> _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint32_t));
                    new (&_q_var.__flags2_code) std::bitset<32>;
                    _q_var.__flags2_code = _var_value;
                    _idx += sizeof(uint32_t);
                    break;
                }
                case zpt::mysql::Q_SQL_MODE_CODE: { // Value is an 8-byte SQL mode value.
                    uint64_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint64_t));
                    _q_var.__sql_mode_code = _var_value;
                    _idx += sizeof(uint64_t);
                    break;
                }
                case zpt::mysql::Q_CATALOG_CODE: { // Value is the catalog name: a length
                                                   // byte followed
                    // by that
                    // many bytes, plus a terminating null byte. This variable is
                    // present only in MySQL 5.0.0 to 5.0.3. It was replaced with
                    // Q_CATALOG_NZ_CODE in MySQL 5.0.4 because the terminating
                    // null is unnecessary.
                    uint8_t _var_length = 0;
                    _in.read((char*)&_var_length, sizeof(uint8_t));
                    char _var_value[_var_length + 1] = { 0 };
                    _in.read(_var_value, _var_length + 1);
                    new (&_q_var.__catalog_code) std::string;
                    _q_var.__catalog_code = zpt::mysql::arr2str(_var_value, _var_length);
                    _idx += sizeof(uint8_t) + _var_length + 1;
                    break;
                }
                case zpt::mysql::Q_AUTO_INCREMENT: { // Value is two 2-byte unsigned
                                                     // integers
                    // representing the
                    // auto_increment_increment and auto_increment_offset
                    // system variables. This variable is present only if
                    // auto_increment is greater than 1.
                    uint16_t _var_value_1 = 0;
                    _in.read((char*)&_var_value_1, sizeof(uint16_t));
                    uint16_t _var_value_2 = 0;
                    _in.read((char*)&_var_value_2, sizeof(uint16_t));
                    new (&_q_var.__auto_increment) std::tuple<uint16_t, uint16_t>;
                    std::get<0>(_q_var.__auto_increment) = _var_value_1;
                    std::get<1>(_q_var.__auto_increment) = _var_value_2;
                    _idx += 2 * sizeof(uint16_t);
                    break;
                }
                case zpt::mysql::Q_CHARSET_CODE: { // Value is three 2-byte unsigned
                                                   // integers
                    // representing the
                    // character_set_client, collation_connection, and
                    // collation_server system variables.
                    uint16_t _var_value_1 = 0;
                    _in.read((char*)&_var_value_1, sizeof(uint16_t));
                    uint16_t _var_value_2 = 0;
                    _in.read((char*)&_var_value_2, sizeof(uint16_t));
                    uint16_t _var_value_3 = 0;
                    _in.read((char*)&_var_value_3, sizeof(uint16_t));
                    new (&_q_var.__charset_code) std::tuple<uint16_t, uint16_t, uint16_t>;
                    std::get<0>(_q_var.__charset_code) = _var_value_1;
                    std::get<1>(_q_var.__charset_code) = _var_value_2;
                    std::get<2>(_q_var.__charset_code) = _var_value_3;
                    _idx += 3 * sizeof(uint16_t);
                    break;
                }
                case zpt::mysql::Q_TIME_ZONE_CODE: { // Value is the time zone name: a
                                                     // length byte
                    // followed by
                    // that many bytes. This variable is present only if the
                    // time zone string is non-empty.
                    uint8_t _var_length = 0;
                    _in.read((char*)&_var_length, sizeof(uint8_t));
                    char _var_value[_var_length] = { 0 };
                    _in.read(_var_value, _var_length);
                    new (&_q_var.__time_zone_code) std::string;
                    _q_var.__time_zone_code = zpt::mysql::arr2str(_var_value, _var_length);
                    _idx += sizeof(uint8_t) + _var_length;
                    break;
                }
                case zpt::mysql::Q_CATALOG_NZ_CODE: { // Value is the catalog name: a
                                                      // length byte
                    // followed by
                    // that many bytes. Value is always std. This variable is
                    // present only if the catalog name is non-empty.
                    uint8_t _var_length = 0;
                    _in.read((char*)&_var_length, sizeof(uint8_t));
                    char _var_value[_var_length] = { 0 };
                    _in.read(_var_value, _var_length);
                    new (&_q_var.__catalog_nz_code) std::string;
                    _q_var.__catalog_nz_code = zpt::mysql::arr2str(_var_value, _var_length);
                    _idx += sizeof(uint8_t) + _var_length;
                    break;
                }
                case zpt::mysql::Q_LC_TIME_NAMES_CODE: { // Value is a 2-byte unsigned
                                                         // integer
                    // representing the
                    // lc_time_names number. This variable is present only
                    // if the value is not 0 (that is, not en_US).
                    uint16_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint16_t));
                    _q_var.__lc_time_names_code = _var_value;
                    _idx += sizeof(uint16_t);
                    break;
                }
                case zpt::mysql::Q_CHARSET_DATABASE_CODE: { // Value is a 2-byte unsigned
                                                            // integer
                    // representing
                    // the collation_database system variable.
                    uint16_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint16_t));
                    _q_var.__charset_database_code = _var_value;
                    _idx += sizeof(uint16_t);
                    break;
                }
                case zpt::mysql::Q_TABLE_MAP_FOR_UPDATE_CODE: { // Value is 8 bytes
                                                                // representing the
                    // table map
                    // to be updated by a multiple-table update
                    // statement. Each bit of this variable
                    // represents a table, and is set to 1 if the
                    // corresponding table is to be updated by the
                    // statement.
                    uint64_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint64_t));
                    _q_var.__table_map_for_update_code = _var_value;
                    _idx += sizeof(uint64_t);
                    break;
                }
                case zpt::mysql::Q_MASTER_DATA_WRITTEN_CODE: {
                    uint32_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint32_t));
                    _q_var.__master_data_written_code = _var_value;
                    _idx += sizeof(uint32_t);
                    break;
                }
                case zpt::mysql::Q_INVOKER: {
                    new (&_q_var.__invoker) std::tuple<std::string, std::string>;
                    {
                        uint8_t _var_length = 0;
                        _in.read((char*)&_var_length, sizeof(uint8_t));
                        char _var_value[_var_length] = { 0 };
                        _in.read(_var_value, _var_length);
                        std::get<0>(_q_var.__invoker) =
                          zpt::mysql::arr2str(_var_value, _var_length);
                        _idx += sizeof(uint8_t) + _var_length;
                    }
                    {
                        uint8_t _var_length = 0;
                        _in.read((char*)&_var_length, sizeof(uint8_t));
                        char _var_value[_var_length] = { 0 };
                        _in.read(_var_value, _var_length);
                        std::get<1>(_q_var.__invoker) =
                          zpt::mysql::arr2str(_var_value, _var_length);
                        _idx += sizeof(uint8_t) + _var_length;
                    }
                    break;
                }
                case zpt::mysql::Q_UPDATED_DB_NAMES: {
                    new (&_q_var.__updated_db_names) std::vector<std::string>;
                    uint8_t _arr_length = 0;
                    _in.read((char*)&_arr_length, sizeof(uint8_t));
                    _idx += sizeof(uint8_t);
                    for (uint8_t _arr_idx = 0; _arr_idx != _arr_length; _arr_idx++) {
                        uint32_t _str_len =
                          std::min<uint32_t>(NAME_LEN, _event->__variable_length - _idx);
                        char _var_value[_str_len + 1] = { 0 };
                        _in.read(_var_value, _str_len);
                        _idx += _str_len;
                        _q_var.__updated_db_names.push_back(
                          zpt::mysql::arr2str(_var_value, _str_len));
                    }
                    break;
                }
                case zpt::mysql::Q_MICROSECONDS: {
                    uint64_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint64_t));
                    _q_var.__commit_ts = _var_value;
                    _idx += sizeof(uint64_t);
                    break;
                }
                case zpt::mysql::Q_COMMIT_TS: {
                    uint64_t _var_value = 0;
                    _in.read((char*)&_var_value, sizeof(uint64_t));
                    _q_var.__commit_ts = _var_value;
                    _idx += sizeof(uint64_t);
                    break;
                }
                case zpt::mysql::Q_COMMIT_TS2: {
                    break;
                }
                case zpt::mysql::Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP: {
                    break;
                }
                default: {
                    assertz(_var_type < 17,
                            "deserialization error: query variable type is higher than 16",
                            0,
                            500);
                }
            }
            _event->__variables.push_back(_q_var);
        }

        char _database_name[_event->__database_name_length] = { 0 };
        _in.read(_database_name, _event->__database_name_length);
        _event->__database_name =
          zpt::mysql::arr2str(_database_name, _event->__database_name_length);

        size_t _query_length = _event->header()->__event_length - zpt::mysql::lengths::header -
                               zpt::mysql::lengths::fixed::query_event -
                               _event->__database_name_length - _event->__variable_length;
        char _query[_query_length] = { 0 };
        _in.read(_query, _query_length);
        _event->__query = zpt::mysql::arr2str(_query, _query_length);
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::StopEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::StopEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::StopEvent* _event = new zpt::mysql::StopEvent();
    _event->header(_header);
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::RotateEvent::data_to_json() -> zpt::json {
    return { zpt::array,
             { "next_event_position",
               (unsigned long)this->__position_next_event,
               "next_binlog_filename",
               this->__next_binlog_filename } };
}

auto
zpt::mysql::RotateEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::RotateEvent* _event = new zpt::mysql::RotateEvent();
    _event->header(_header);
    if (_header->__event_length) {
        _in.read((char*)&_event->__position_next_event, sizeof(uint64_t));

        size_t _next_binlog_filename_size = _event->header()->__event_length - sizeof(uint64_t);
        char _next_binlog_filename[_next_binlog_filename_size] = { 0 };
        _in.read(_next_binlog_filename, _next_binlog_filename_size);
        _event->__next_binlog_filename =
          zpt::mysql::arr2str(_next_binlog_filename, _next_binlog_filename_size - 8);
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::IntvarEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::IntvarEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::IntvarEvent* _event = new zpt::mysql::IntvarEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::LoadEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::LoadEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::LoadEvent* _event = new zpt::mysql::LoadEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::SlaveEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::SlaveEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::SlaveEvent* _event = new zpt::mysql::SlaveEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::CreateFileEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::CreateFileEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::CreateFileEvent* _event = new zpt::mysql::CreateFileEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::AppendBlockEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::AppendBlockEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::AppendBlockEvent* _event = new zpt::mysql::AppendBlockEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::ExecLoadEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::ExecLoadEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::ExecLoadEvent* _event = new zpt::mysql::ExecLoadEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::DeleteFileEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::DeleteFileEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::DeleteFileEvent* _event = new zpt::mysql::DeleteFileEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::NewLoadEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::NewLoadEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::NewLoadEvent* _event = new zpt::mysql::NewLoadEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::RandEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::RandEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::RandEvent* _event = new zpt::mysql::RandEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::UserVarEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::UserVarEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::UserVarEvent* _event = new zpt::mysql::UserVarEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::FormatDescriptionEvent::data_to_json() -> zpt::json {
    zpt::json _post_header_lengths = zpt::json::array();
    for (uint16_t _c = 0; _c != zpt::mysql::post_header_lengths_size; _c++) {
        _post_header_lengths << zpt::mysql::post_header_lengths[_c];
    }
    return { zpt::array,
             { "binlog_version",
               (unsigned long)this->__binlog_version,
               "server_version",
               this->__server_version,
               "timestamp",
               zpt::timestamp_t(((zpt::timestamp_t)this->__timestamp) * 1000),
               "header_length",
               (unsigned int)this->__header_length,
               "post_header_lengths",
               _post_header_lengths } };
}

auto
zpt::mysql::FormatDescriptionEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::FormatDescriptionEvent* _event = new zpt::mysql::FormatDescriptionEvent();
    _event->header(_header);
    if (_header->__event_length) {
        char _server_version[50] = { 0 };
        zpt::mysql::post_header_lengths_size = _event->header()->__event_length - 19 - 57;
        zpt::mysql::post_header_lengths = new uint8_t[zpt::mysql::post_header_lengths_size];

        _in.read((char*)&_event->__binlog_version, sizeof _event->__binlog_version);
        _in.read(_server_version, sizeof _server_version);
        _in.read((char*)&_event->__timestamp, sizeof _event->__timestamp);
        _in.read((char*)&_event->__header_length, sizeof _event->__header_length);
        _in.read((char*)zpt::mysql::post_header_lengths, zpt::mysql::post_header_lengths_size);

        _event->__server_version = zpt::mysql::arr2str(_server_version, sizeof _server_version);
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::XidEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::XidEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::XidEvent* _event = new zpt::mysql::XidEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::BeginLoadQueryEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::BeginLoadQueryEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::BeginLoadQueryEvent* _event = new zpt::mysql::BeginLoadQueryEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::ExecuteLoadQueryEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::ExecuteLoadQueryEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::ExecuteLoadQueryEvent* _event = new zpt::mysql::ExecuteLoadQueryEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::TableMapEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::TableMapEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::TableMapEvent* _event = new zpt::mysql::TableMapEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::PreGAWriteRowsEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::PreGAWriteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::PreGAWriteRowsEvent* _event = new zpt::mysql::PreGAWriteRowsEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::PreGAUpdateRowsEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::PreGAUpdateRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::PreGAUpdateRowsEvent* _event = new zpt::mysql::PreGAUpdateRowsEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::PreGADeleteRowsEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::PreGADeleteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::PreGADeleteRowsEvent* _event = new zpt::mysql::PreGADeleteRowsEvent();
    _event->header(_header);
    if (_header->__event_length) {
        _in.read((char*)&_event->__table_id, 6);
        uint16_t _to_dispose = 0x0;
        _in.read((char*)&_to_dispose, 2);

        char _pi_0 = 0x0;
        _in.read(&_pi_0, 1);
        if (_pi_0 <= 250) {
            _event->__number_of_columns = (uint64_t)_pi_0;
        }
        else if (_pi_0 == 252) {
            _in.read((char*)&_event->__number_of_columns, 2);
        }
        else if (_pi_0 == 253) {
            _in.read((char*)&_event->__number_of_columns, 3);
        }
        else if (_pi_0 == 254) {
            _in.read((char*)&_event->__number_of_columns, 8);
        }
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::WriteRowsEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::WriteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::WriteRowsEvent* _event = new zpt::mysql::WriteRowsEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::UpdateRowsEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::UpdateRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::UpdateRowsEvent* _event = new zpt::mysql::UpdateRowsEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::DeleteRowsEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::DeleteRowsEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::DeleteRowsEvent* _event = new zpt::mysql::DeleteRowsEvent();
    _event->header(_header);
    if (_header->__event_length) {
        _in.read((char*)&_event->__table_id, 6);
        uint16_t _to_dispose = 0x0;
        _in.read((char*)&_to_dispose, 2);

        char _pi_0 = 0x0;
        _in.read(&_pi_0, 1);
        if (_pi_0 <= 250) {
            _event->__number_of_columns = (uint64_t)_pi_0;
        }
        else if (_pi_0 == 252) {
            _in.read((char*)&_event->__number_of_columns, 2);
        }
        else if (_pi_0 == 253) {
            _in.read((char*)&_event->__number_of_columns, 3);
        }
        else if (_pi_0 == 254) {
            _in.read((char*)&_event->__number_of_columns, 8);
        }
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::IncidentEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::IncidentEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::IncidentEvent* _event = new zpt::mysql::IncidentEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::HeartbeatLogEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::HeartbeatLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::HeartbeatLogEvent* _event = new zpt::mysql::HeartbeatLogEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::IgnorableLogEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::IgnorableLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::IgnorableLogEvent* _event = new zpt::mysql::IgnorableLogEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::RowsQueryLogEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::RowsQueryLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::RowsQueryLogEvent* _event = new zpt::mysql::RowsQueryLogEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::GTIDLogEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::GTIDLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::GTIDLogEvent* _event = new zpt::mysql::GTIDLogEvent();
    _event->header(_header);
    if (_header->__event_length) {
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::AnonymousGTIDLogEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::AnonymousGTIDLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::AnonymousGTIDLogEvent* _event = new zpt::mysql::AnonymousGTIDLogEvent();
    _event->header(_header);
    if (_header->__event_length) {
        char _raw[_header->__event_length - 19];
        _in.read(_raw, _header->__event_length - 19);
    }
    return zpt::mysql::event(_event);
}

auto
zpt::mysql::PreviousGTIDSLogEvent::data_to_json() -> zpt::json {
    zpt::json _return = zpt::json::array();
    return _return;
}

auto
zpt::mysql::PreviousGTIDSLogEvent::consume(zpt::mysql::event_header _header, std::istream& _in)
  -> zpt::mysql::event {
    zpt::mysql::PreviousGTIDSLogEvent* _event = new zpt::mysql::PreviousGTIDSLogEvent();
    _event->header(_header);
    if (_header->__event_length) {
        char _raw[_header->__event_length - 19];
        _in.read(_raw, _header->__event_length - 19);
    }
    return zpt::mysql::event(_event);
}

auto
consume(std::string _binlog) -> int {
    zpt::mysql::event _event;
    return 0;
}

namespace zpt {
namespace mysql {
std::vector<std::function<zpt::mysql::event(zpt::mysql::event_header _header, std::istream& _in)>>
  consumers = { nullptr,
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::StartEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::QueryEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::StopEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::RotateEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::IntvarEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::LoadEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::SlaveEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::CreateFileEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::AppendBlockEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::ExecLoadEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::DeleteFileEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::NewLoadEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::RandEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::UserVarEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::FormatDescriptionEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::XidEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::BeginLoadQueryEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::ExecuteLoadQueryEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::TableMapEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::PreGAWriteRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::PreGAUpdateRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::PreGADeleteRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::WriteRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::UpdateRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::DeleteRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::IncidentEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::HeartbeatLogEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::IgnorableLogEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::RowsQueryLogEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::WriteRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::UpdateRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::DeleteRowsEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::GTIDLogEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::AnonymousGTIDLogEvent::consume(_header, _in);
                },
                [](zpt::mysql::event_header _header, std::istream& _in) -> zpt::mysql::event {
                    return zpt::mysql::PreviousGTIDSLogEvent::consume(_header, _in);
                } };

std::string event_names[] = { "unknown_event",
                              "start_event_v3",
                              "query_event",
                              "stop_event",
                              "rotate_event",
                              "intvar_event",
                              "load_event",
                              "slave_event",
                              "create_file_event",
                              "append_block_event",
                              "exec_load_event",
                              "delete_file_event",
                              "new_load_event",
                              "rand_event",
                              "user_var_event",
                              "format_description_event",
                              "xid_event",
                              "begin_load_query_event",
                              "execute_load_query_event",
                              "table_map_event",
                              "pre_ga_write_rows_event",
                              "pre_ga_update_rows_event",
                              "pre_ga_delete_rows_event",
                              "write_rows_event",
                              "update_rows_event",
                              "delete_rows_event",
                              "incident_event",
                              "heartbeat_log_event",
                              "ignorable_log_event",
                              "rows_query_log_event",
                              "o_write_rows_event",
                              "o_update_rows_event",
                              "o_delete_rows_event",
                              "gtid_log_event",
                              "anonymous_gtid_log_event",
                              "previous_gtids_log_event" };

std::string query_var_names[] = { "flags2_code",
                                  "sql_mode_code",
                                  "catalog_code",
                                  "auto_increment",
                                  "charset_code",
                                  "time_zone_code",
                                  "catalog_nz_code",
                                  "lc_time_names_code",
                                  "charset_database_code",
                                  "table_map_for_update_code",
                                  "master_data_written_code",
                                  "invoker",
                                  "updated_db_names",
                                  "microseconds",
                                  "commit_ts",
                                  "commit_ts2",
                                  "explicit_defaults_for_timestamp" };
} // namespace mysql
} // namespace zpt
