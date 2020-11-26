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
#include <zapata/couchdb/convert_couchdb.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>

#define _VALID_OPS std::string("$gt^$gte^$lt^$lte^$ne^$type^$exists^$in^$nin^$elemMatch^")

auto
zpt::couchdb::get_query(zpt::json _in) -> zpt::json {
    zpt::json _selector = { "_id", { "$lt", "_" } };
    zpt::json _query = { "selector", _selector };
    if (!_in->is_object()) {
        _query << "limit" << INT_MAX;
        return _query;
    }
    for (auto _i : _in->object()) {
        std::string _key = _i.first;
        zpt::json _value = _i.second;

        if (_key == "page_size") {
            _query << "limit" << ((unsigned long)_value);
            continue;
        }
        else if (_key == "page_start_index") {
            _query << "skip" << ((unsigned long)_value);
            continue;
        }
        else if (_key == "order_by" && (_value->is_string() || _value->is_array())) {
            if (!_query["sort"]->is_array()) { _query << "sort" << zpt::json::array(); }
            zpt::json _splited;
            if (_value->is_string()) { _splited = zpt::split(std::string(_value), ",", true); }
            else {
                _splited = _value;
            }
            for (auto _field : _splited->array()) {
                std::string _name = std::string(_field);
                std::string _direction;
                if (_name[0] == '-') {
                    _direction.assign("desc");
                    _name = _name.substr(1);
                }
                else if (_name[0] == '+') {
                    _direction.assign("asc");
                    _name = _name.substr(1);
                }
                else {
                    _direction.assign("asc");
                }
                _query["sort"] << zpt::json{ _name, _direction };
            }
            continue;
        }
        else if (_key == "fields" && (_value->is_string() || _value->is_array())) {
            _query << "fields"
                   << (_value->is_string() ? zpt::split(_value->string(), ",", true) : _value);
            continue;
        }
        else if (_key == "embed") {
            continue;
        }

        if (_value->type() != zpt::JSString) {
            _selector << std::string(_key) << _value;
            continue;
        }

        std::string _s_value = (std::string)_value;
        if (_s_value.length() > 3 && _s_value.find('/') != std::string::npos) {
            int _bar_count = 0;

            std::istringstream _lss(_s_value);
            std::string _part;

            std::string _command;
            std::string _expression;
            std::string _options;
            while (std::getline(_lss, _part, '/')) {
                if (_bar_count == 0) {
                    _command = _part;
                    ++_bar_count;
                }
                else if (_bar_count == 1) {
                    _expression.append(_part);

                    if (_expression.length() == 0 ||
                        _expression[_expression.length() - 1] != '\\') {
                        ++_bar_count;
                    }
                    else {
                        if (_expression.length() > 0) {
                            _expression[_expression.length() - 1] = '/';
                        }
                    }
                }
                else if (_bar_count == 2) {
                    _options = _part;
                    ++_bar_count;
                }
                else {
                    ++_bar_count;
                }
            }

            if (_command == "m") {
                if (_bar_count == 3) {
                    _selector << std::string(_key) << zpt::json{ "$regex", _expression };
                    continue;
                }
            }
            else if (_command == "n") {
                if (_bar_count == 2) {
                    std::istringstream iss(_expression);
                    int i = 0;
                    iss >> i;
                    if (!iss.eof()) {
                        iss.clear();
                        double d = 0;
                        iss >> d;
                        if (!iss.eof()) {
                            std::string bexpr(_expression.data());
                            std::transform(bexpr.begin(), bexpr.end(), bexpr.begin(), ::tolower);
                            if (bexpr != "true" && bexpr != "false") {
                                _selector << std::string(_key) << _expression;
                            }
                            else {
                                _selector << std::string(_key)
                                          << zpt::json::boolean(bexpr == "true");
                            }
                        }
                        else {
                            _selector << std::string(_key) << d;
                        }
                    }
                    else {
                        _selector << std::string(_key) << i;
                    }
                    continue;
                }
            }
            else {
                std::string comp("$");
                comp.insert(comp.length(), _command);

                if (_VALID_OPS.find(comp + std::string("^")) != std::string::npos) {
                    if (_bar_count == 2) {
                        _selector << std::string(_key) << zpt::json{ comp, _expression };
                    }
                    else if (_command == "elemMatch") {
                        std::string other_comp("$");
                        other_comp.insert(other_comp.length(), _options);
                        zpt::json _json_expression;
                        try {
                            _json_expression = zpt::json(_expression);
                            if (_VALID_OPS.find(other_comp + std::string("^")) !=
                                std::string::npos) {
                                _selector << std::string(_key)
                                          << zpt::json{ comp, { other_comp, _json_expression } };
                            }
                            else {
                                _selector << std::string(_key)
                                          << zpt::json{ comp, { _options, _json_expression } };
                            }
                        }
                        catch (std::exception const& _e) {
                            if (_VALID_OPS.find(other_comp + std::string("^")) !=
                                std::string::npos) {
                                _selector << std::string(_key)
                                          << zpt::json{ comp, { other_comp, _expression } };
                            }
                            else {
                                _selector << std::string(_key)
                                          << zpt::json{ comp, { _options, _expression } };
                            }
                        }
                    }
                    else if (_options == "n") {
                        istringstream iss(_expression);
                        int i = 0;
                        iss >> i;
                        if (!iss.eof()) {
                            iss.str(_expression);
                            double d = 0;
                            iss >> d;
                            if (!iss.eof()) {
                                std::string bexpr(_expression.data());
                                std::transform(
                                  bexpr.begin(), bexpr.end(), bexpr.begin(), ::tolower);
                                if (bexpr != "true" && bexpr != "false") {
                                    _selector << std::string(_key)
                                              << zpt::json{ comp, _expression };
                                }
                                else {
                                    _selector
                                      << std::string(_key)
                                      << zpt::json{ comp, zpt::json::boolean(bexpr == "true") };
                                }
                            }
                            else {
                                _selector << std::string(_key) << zpt::json{ comp, d };
                            }
                        }
                        else {
                            _selector << std::string(_key) << zpt::json{ comp, i };
                        }
                    }
                    else if (_options == "j") {
                        try {
                            zpt::json _json = zpt::json(_expression);
                            _selector << std::string(_key) << zpt::json{ comp, _json };
                        }
                        catch (std::exception const& _e) {
                        }
                    }
                    else if (_options == "d") {
                        zpt::json _json({ "time", zpt::timestamp(_expression) });
                        _selector << std::string(_key) << zpt::json{ comp, _json["time"] };
                    }
                    continue;
                }
            }
        }

        _selector << std::string(_key) << _value;
    }

    if (!_query["limit"]->ok()) { _query << "limit" << INT_MAX; }

    return _query;
}

auto
zpt::couchdb::get_fields(zpt::json _opts) -> zpt::json {
    zpt::json _return = zpt::json::object();
    zpt::json _fields = _opts["fields"];
    if (_fields->ok()) {
        if (_fields->is_string()) { _fields = zpt::split(std::string(_opts["fields"]), ","); }
        if (_fields->is_array()) {
            for (auto _f : _fields->array()) { _return << std::string(_f) << true; }
            _return << "_id" << true;
        }
    }
    return _return;
}
