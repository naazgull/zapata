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
#include <zapata/http/HTTPObj.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>
#include <zapata/mongodb/convert_mongo.h>

#define _VALID_OPS std::string("$gt^$gte^$lt^$lte^$ne^$type^$exists^$in^$nin^")

void
zpt::mongodb::frommongo(mongo::BSONObj& _in, zpt::JSONObj& _out) {
    for (mongo::BSONObjIterator _i = _in.begin(); _i.more();) {
        mongo::BSONElement _it = _i.next();
        std::string _key(_it.fieldName());

        switch (_it.type()) {
            case mongo::jstNULL: {
                _out << _key << zpt::undefined;
                break;
            }
            case mongo::Bool: {
                _out << _key << _it.Bool();

                break;
            }
            case mongo::NumberDouble: {
                _out << _key << _it.Double();
                break;
            }
            case mongo::NumberLong:
            case mongo::NumberInt: {
                _out << _key << _it.Int();
                break;
            }
            case mongo::String: {
                _out << _key << _it.String();
                break;
            }
            case mongo::Object: {
                mongo::BSONObj _mobj = _it.Obj();
                zpt::JSONObj _obj;
                zpt::mongodb::frommongo(_mobj, _obj);
                _out << _key << _obj;
                break;
            }
            case mongo::Array: {
                zpt::JSONArr _arr;
                zpt::mongodb::frommongo(_it, _arr);
                _out << _key << _arr;
                break;
            }
            default: {
                continue;
            }
        }
    }
}

void
zpt::mongodb::frommongo(mongo::BSONElement& _in, zpt::JSONArr& _out) {
    std::vector<mongo::BSONElement> _obj = _in.Array();
    for (auto _it : _obj) {
        switch (_it.type()) {
            case mongo::jstNULL: {
                _out << zpt::undefined;
                break;
            }
            case mongo::Bool: {
                _out << _it.Bool();

                break;
            }
            case mongo::NumberDouble: {
                _out << _it.Double();
                break;
            }
            case mongo::NumberLong:
            case mongo::NumberInt: {
                _out << _it.Int();
                break;
            }
            case mongo::String: {
                _out << _it.String();
                break;
            }
            case mongo::Object: {
                mongo::BSONObj _mobj = _it.Obj();
                zpt::JSONObj _obj;
                zpt::mongodb::frommongo(_mobj, _obj);
                _out << _obj;
                break;
            }
            case mongo::Array: {
                zpt::JSONArr _arr;
                zpt::mongodb::frommongo(_it, _arr);
                _out << _arr;
                break;
            }
            default: {
                continue;
            }
        }
    }
}

void
zpt::mongodb::tomongo(zpt::JSONObj& _in, mongo::BSONObjBuilder& _out) {
    for (auto _i : *_in) {
        std::string _key = _i.first;
        JSONElement _value = _i.second;

        switch (_value->type()) {
            case zpt::JSObject: {
                mongo::BSONObjBuilder _mobj;
                zpt::mongodb::tomongo(_value->obj(), _mobj);
                _out << _key << _mobj.obj();
                break;
            }
            case zpt::JSArray: {
                mongo::BSONArrayBuilder _mobj;
                zpt::mongodb::tomongo(_value->arr(), _mobj);
                _out << _key << _mobj.arr();
                break;
            }
            case zpt::JSString: {
                _out << _key << (std::string)_value;
                break;
            }
            case zpt::JSBoolean: {
                _out << _key << (bool)_value;
                break;
            }
            case zpt::JSDouble: {
                _out << _key << (double)_value;
                break;
            }
            case zpt::JSInteger: {
                _out << _key << (int)_value;
                break;
            }
            case zpt::JSLambda: {
                break;
            }
            case zpt::JSNil: {
                _out.appendNull(_key);
                break;
            }
            default: {
                continue;
            }
        }
    }
}

void
zpt::mongodb::tomongo(zpt::JSONArr& _in, mongo::BSONArrayBuilder& _out) {
    for (auto _i : *_in) {
        JSONPtr _value = _i;

        switch (_value->type()) {
            case zpt::JSObject: {
                mongo::BSONObjBuilder _mobj;
                zpt::mongodb::tomongo(_value->obj(), _mobj);
                _out << _mobj.obj();
                break;
            }
            case zpt::JSArray: {
                mongo::BSONArrayBuilder _mobj;
                zpt::mongodb::tomongo(_value->arr(), _mobj);
                _out << _mobj.arr();
                break;
            }
            case zpt::JSString: {
                _out << (std::string)_value;
                break;
            }
            case zpt::JSBoolean: {
                _out << (bool)_value;
                break;
            }
            case zpt::JSDouble: {
                _out << (double)_value;
                break;
            }
            case zpt::JSInteger: {
                _out << (int)_value;
                break;
            }
            case zpt::JSLambda: {
                break;
            }
            case zpt::JSNil: {
                _out.appendNull();
                break;
            }
            default: {
                continue;
            }
        }
    }
}

void
zpt::mongodb::tosetcommand(zpt::JSONObj& _in, mongo::BSONObjBuilder& _out, std::string _prefix) {
    for (auto _i : *_in) {
        std::string _key = _i.first;
        JSONElement _value = _i.second;

        switch (_value->type()) {
            case zpt::JSObject: {
                zpt::mongodb::tosetcommand(
                  _value->obj(),
                  _out,
                  (_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key));
                break;
            }
            case zpt::JSArray: {
                zpt::mongodb::tosetcommand(
                  _value->arr(),
                  _out,
                  (_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key));
                break;
            }
            case zpt::JSString: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key)
                     << (std::string)_value;
                break;
            }
            case zpt::JSBoolean: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key)
                     << (bool)_value;
                break;
            }
            case zpt::JSDouble: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key)
                     << (double)_value;
                break;
            }
            case zpt::JSInteger: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key)
                     << (int)_value;
                break;
            }
            case zpt::JSLambda: {
                break;
            }
            case zpt::JSNil: {
                _out.appendNull((_prefix.length() != 0 ? _prefix + std::string(".") + _key : _key));
                break;
            }
            default: {
                continue;
            }
        }
    }
}

void
zpt::mongodb::tosetcommand(zpt::JSONArr& _in, mongo::BSONObjBuilder& _out, std::string _prefix) {
    size_t _idx = 0;
    for (auto _i : *_in) {
        JSONPtr _value = _i;

        switch (_value->type()) {
            case zpt::JSObject: {
                zpt::mongodb::tosetcommand(_value->obj(),
                                           _out,
                                           (_prefix.length() != 0
                                              ? _prefix + std::string(".") + std::to_string(_idx)
                                              : std::to_string(_idx)));
                break;
            }
            case zpt::JSArray: {
                zpt::mongodb::tosetcommand(_value->arr(),
                                           _out,
                                           (_prefix.length() != 0
                                              ? _prefix + std::string(".") + std::to_string(_idx)
                                              : std::to_string(_idx)));
                break;
            }
            case zpt::JSString: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + std::to_string(_idx)
                                               : std::to_string(_idx))
                     << (std::string)_value;
                break;
            }
            case zpt::JSBoolean: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + std::to_string(_idx)
                                               : std::to_string(_idx))
                     << (bool)_value;
                break;
            }
            case zpt::JSDouble: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + std::to_string(_idx)
                                               : std::to_string(_idx))
                     << (double)_value;
                break;
            }
            case zpt::JSInteger: {
                _out << (_prefix.length() != 0 ? _prefix + std::string(".") + std::to_string(_idx)
                                               : std::to_string(_idx))
                     << (int)_value;
                break;
            }
            case zpt::JSLambda: {
                break;
            }
            case zpt::JSNil: {
                _out.appendNull((_prefix.length() != 0
                                   ? _prefix + std::string(".") + std::to_string(_idx)
                                   : std::to_string(_idx)));
                break;
            }
            default: {
                continue;
            }
        }

        _idx++;
    }
}

void
zpt::mongodb::get_query(zpt::json _in, mongo::BSONObjBuilder& _queryr) {
    if (!_in->is_object()) {
        return;
    }
    for (auto _i : _in->obj()) {
        std::string _key = _i.first;
        JSONElement _value = _i.second;

        if (_key == "page_size" || _key == "page_start_index" || _key == "order_by" ||
            _key == "fields" || _key == "embed") {
            continue;
        }

        std::ostringstream oss;
        oss << _key << std::flush;

        std::string key = oss.str();
        if (_value->type() == zpt::JSObject) {
            mongo::BSONObjBuilder _tmp;
            zpt::mongodb::tomongo(_value, _tmp);
            _queryr.append(key, _tmp.done());
            continue;
        }
        if (_value->type() == zpt::JSArray) {
            mongo::BSONArrayBuilder _tmp;
            zpt::mongodb::tomongo(_value->arr(), _tmp);
            _queryr.append(key, _tmp.arr());
            continue;
        }

        std::string value = (std::string)_value;
        if (value.length() > 3 && value.find('/') != std::string::npos) {
            int bar_count = 0;

            istringstream lss(value);
            std::string part;

            std::string command;
            std::string expression;
            std::string options;
            while (std::getline(lss, part, '/')) {
                if (bar_count == 0) {
                    command = part;
                    ++bar_count;
                }
                else if (bar_count == 1) {
                    expression.append(part);

                    if (expression.length() == 0 || expression[expression.length() - 1] != '\\') {
                        ++bar_count;
                    }
                    else {
                        if (expression.length() > 0) {
                            expression[expression.length() - 1] = '/';
                        }
                    }
                }
                else if (bar_count == 2) {
                    options = part;
                    ++bar_count;
                }
                else {
                    ++bar_count;
                }
            }

            if (command == "m") {
                if (bar_count == 3) {
                    _queryr.appendRegex(key, expression, options);
                    continue;
                }
            }
            else if (command == "n") {
                if (bar_count == 2) {
                    std::istringstream iss(expression);
                    int i = 0;
                    iss >> i;
                    if (!iss.eof()) {
                        iss.clear();
                        double d = 0;
                        iss >> d;
                        if (!iss.eof()) {
                            std::string bexpr(expression.data());
                            std::transform(bexpr.begin(), bexpr.end(), bexpr.begin(), ::tolower);
                            if (bexpr != "true" && bexpr != "false") {
                                _queryr.append(key, expression);
                            }
                            else {
                                _queryr.append(key, bexpr == "true");
                            }
                        }
                        else {
                            _queryr.append(key, d);
                        }
                    }
                    else {
                        _queryr.append(key, i);
                    }
                    continue;
                }
            }
            else {
                std::string comp("$");
                comp.insert(comp.length(), command);

                if (_VALID_OPS.find(comp + std::string("^")) != std::string::npos) {
                    if (bar_count == 2) {
                        _queryr.append(key, BSON(comp << expression));
                    }
                    else if (options == "n") {
                        istringstream iss(expression);
                        int i = 0;
                        iss >> i;
                        if (!iss.eof()) {
                            iss.str(expression);
                            double d = 0;
                            iss >> d;
                            if (!iss.eof()) {
                                std::string bexpr(expression.data());
                                std::transform(
                                  bexpr.begin(), bexpr.end(), bexpr.begin(), ::tolower);
                                if (bexpr != "true" && bexpr != "false") {
                                    _queryr.append(key, BSON(comp << expression));
                                }
                                else {
                                    _queryr.append(key, BSON(comp << (bexpr == "true")));
                                }
                            }
                            else {
                                _queryr.append(key, BSON(comp << d));
                            }
                        }
                        else {
                            _queryr.append(key, BSON(comp << i));
                        }
                    }
                    else if (options == "j") {
                        istringstream iss(expression);
                        zpt::json _json;
                        try {
                            iss >> _json;
                            if (_json->type() == zpt::JSObject) {
                                mongo::BSONObjBuilder _mongo_json;
                                zpt::mongodb::tomongo(_json, _mongo_json);
                                _queryr.append(key, BSON(comp << _mongo_json.obj()));
                            }
                            else if (_json->type() == zpt::JSArray) {
                                mongo::BSONArrayBuilder _mongo_json;
                                zpt::mongodb::tomongo(_json, _mongo_json);
                                _queryr.append(key, BSON(comp << _mongo_json.arr()));
                            }
                        }
                        catch (std::exception& _e) {
                        }
                    }
                    else if (options == "d") {
                        zpt::json _json({ "time", zpt::timestamp(expression) });
                        mongo::BSONObjBuilder _mongo_json;
                        zpt::mongodb::tomongo(_json, _mongo_json);
                        _queryr.append(key, BSON(comp << _mongo_json.obj()["time"]));
                    }
                    continue;
                }
            }
        }

        _queryr.append(key, value);
    }
}

auto
zpt::mongodb::get_fields(zpt::json _opts) -> zpt::json {
    zpt::json _return = zpt::json::object();
    zpt::json _fields = _opts["fields"];
    if (_fields->ok()) {
        if (_fields->is_string()) {
            _fields = zpt::split(std::string(_opts["fields"]), ",");
        }
        if (_fields->is_array()) {
            for (auto _f : _fields->arr()) {
                _return << std::string(_f) << true;
            }
            _return << "_id" << true;
        }
    }
    return _return;
}
