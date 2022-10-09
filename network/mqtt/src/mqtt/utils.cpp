#include <zapata/mqtt/utils.h>

auto
zpt::mqtt::utils::check_err(int _return, int _errno, const std::string& _connection, zpt::LogLevel _log_level) -> int {

    if (_return == MOSQ_ERR_SUCCESS) { return _return; }

    std::string _error_description;

    switch (_return) {
        case MOSQ_ERR_ERRNO: _error_description = mosquitto_strerror(_errno); break;
        case MOSQ_ERR_INVAL: _error_description = "input parameters were invalid"; break;
        case MOSQ_ERR_NOMEM: _error_description = "out of memory condition occurred"; break;
        case MOSQ_ERR_NO_CONN: _error_description = "the client isn't connected to a broker"; break;
        case MOSQ_ERR_CONN_LOST: _error_description = "the connection to the broker was lost"; break;
        case MOSQ_ERR_PROTOCOL: _error_description = "there is a protocol error communicating with the broker"; break;
        case MOSQ_ERR_CONN_REFUSED: _error_description = "MOSQ_ERR_CONN_REFUSED"; break;
        case MOSQ_ERR_NOT_FOUND: _error_description = "MOSQ_ERR_NOT_FOUND"; break;
        case MOSQ_ERR_TLS: _error_description = "MOSQ_ERR_TLS"; break;
        case MOSQ_ERR_PAYLOAD_SIZE: _error_description = "MOSQ_ERR_PAYLOAD_SIZE"; break;
        case MOSQ_ERR_NOT_SUPPORTED: _error_description = "MOSQ_ERR_NOT_SUPPORTED"; break;
        case MOSQ_ERR_AUTH: _error_description = "MOSQ_ERR_AUTH"; break;
        case MOSQ_ERR_ACL_DENIED: _error_description = "MOSQ_ERR_ACL_DENIED"; break;
        case MOSQ_ERR_UNKNOWN: _error_description = "MOSQ_ERR_UNKNOWN"; break;
        case MOSQ_ERR_EAI: _error_description = "MOSQ_ERR_EAI"; break;
        case MOSQ_ERR_PROXY: _error_description = "MOSQ_ERR_PROXY"; break;
        default: _error_description = std::string("unknown error: code=") + std::to_string(_return);
    }

    zlog(std::string("mqtt: error(") + std::to_string(_errno) + std::string(") while connecting to ") + _connection +
           std::string(": ") + _error_description,
         _log_level);

    return _return;
};