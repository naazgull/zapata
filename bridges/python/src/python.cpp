/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <zapata/python/python.h>
#include <zapata/base/sentry.h>
#include <datetime.h>

auto
zpt::PYTHON_BRIDGE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::py_object::py_object(PyObject* _rhs)
  : __underlying{ _rhs } {
    if (this->__underlying != nullptr) { Py_INCREF(this->__underlying); }
}

zpt::py_object::py_object(py_object const& _rhs)
  : __underlying{ _rhs.__underlying } {
    if (this->__underlying != nullptr) { Py_INCREF(this->__underlying); }
}

zpt::py_object::py_object(py_object&& _rhs)
  : __underlying{ _rhs.__underlying } {
    _rhs.__underlying = nullptr;
}

zpt::py_object::~py_object() {
    if (this->__underlying != nullptr) { Py_DECREF(this->__underlying); }
}

auto
zpt::py_object::operator=(py_object const& _rhs) -> py_object& {
    this->__underlying = _rhs.__underlying;
    if (this->__underlying != nullptr) { Py_INCREF(this->__underlying); }
    return (*this);
}

auto
zpt::py_object::operator=(py_object&& _rhs) -> py_object& {
    this->__underlying = _rhs.__underlying;
    _rhs.__underlying = nullptr;
    return (*this);
}

auto
zpt::py_object::operator=(PyObject* _rhs) -> py_object& {
    this->__underlying = _rhs;
    if (this->__underlying != nullptr) { Py_INCREF(this->__underlying); }
    return (*this);
}

auto
zpt::py_object::operator->() -> PyObject* {
    return this->__underlying;
}

auto
zpt::py_object::operator*() -> PyObject& {
    return *this->__underlying;
}

zpt::py_object::operator PyObject*() { return this->__underlying; }

auto
zpt::py_object::get() -> PyObject* {
    return this->__underlying;
}

zpt::python::bridge::~bridge() {
    // if (this->is_initialized()) { Py_Finalize(); }
}

auto
zpt::python::bridge::name() const -> std::string {
    return "python";
}

auto
zpt::python::bridge::setup_module(zpt::json _conf, std::string _name) -> zpt::python::bridge& {
    expect(!this->is_initialized(),
           "Python: bridge already initialized, can't add a module now",
           500,
           0);
    this->__external_to_load.insert(std::make_pair(_name, _conf));
    return (*this);
}

auto
zpt::python::bridge::setup_module(zpt::json _conf, callback_type _callback)
  -> zpt::python::bridge& {
    expect(!this->is_initialized(),
           "Python: bridge already initialized, can't add a module now",
           500,
           0);
    this->__builtin_to_load.insert(
      std::make_pair(_conf["name"]->string(), std::make_tuple(_callback, _conf)));
    return (*this);
}

auto
zpt::python::bridge::find(zpt::json _to_locate) -> object_type {
    expect(_to_locate["module"]->is_string(), "Python: module name must be provided", 500, 0);
    expect(_to_locate["function"]->is_string(), "Python: function name must be provided", 500, 0);
    this->initialize();
    auto _found = this->__modules.find(_to_locate["module"]->string());
    if (_found != this->__modules.end()) {
        auto& _module = _found->second;
        auto _dictionary = PyModule_GetDict(_module);
        auto _func = PyDict_GetItemString(_dictionary, _to_locate["function"]->string().data());
        return _func;
    }
    return nullptr;
}

auto
zpt::python::bridge::to_json(object_type _exp) -> zpt::json {
    if (_exp == nullptr) { return zpt::undefined; }
    this->initialize();

    zpt::json _parent;
    if (_exp == nullptr || _exp == Py_None) {
        // zdbg("Py_None");
        if (_parent->is_object() || _parent->is_array()) { _parent << zpt::undefined; }
        else { _parent = zpt::undefined; }
    }
    else if (PyBool_Check(_exp)) {
        // zdbg("Py_Bool");
        zpt::json _value = zpt::json::boolean(_exp == Py_True ? true : false);
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (PyByteArray_CheckExact(_exp)) {
        // zdbg("Py_ByteArray");
        zpt::json _value =
          zpt::json::string(std::string(PyByteArray_AsString(_exp), PyByteArray_Size(_exp)));
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (PyBytes_CheckExact(_exp)) {
        // zdbg("Py_Bytes");
        zpt::json _value =
          zpt::json::string(std::string(PyBytes_AsString(_exp), PyBytes_Size(_exp)));
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (PyComplex_CheckExact(_exp)) {
        // zdbg("Py_Complex");
        zpt::json _value(
          { "real", PyComplex_RealAsDouble(_exp), "imaginary", PyComplex_ImagAsDouble(_exp) });
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (PyDict_CheckExact(_exp)) {
        // zdbg("Py_Dict");
        zpt::json _object = zpt::json::object();
        underlying_type _key = nullptr;
        underlying_type _value = nullptr;
        Py_ssize_t _pos = 0;
        while (PyDict_Next(_exp, &_pos, &_key, &_value)) {
            _object << std::string(this->to_json(_key)) << this->to_json(_value);
        }
        if (_parent->is_object() || _parent->is_array()) { _parent << _object; }
        else { _parent = _object; }
    }
    else if (PyFloat_CheckExact(_exp)) {
        // zdbg("Py_Float");
        zpt::json _value = zpt::json::floating(PyFloat_AsDouble(_exp));
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (PyList_CheckExact(_exp)) {
        // zdbg("Py_List");
        zpt::json _array = zpt::json::array();
        for (long _idx = 0; _idx != PyList_GET_SIZE(_exp.get()); ++_idx) {
            _array << this->to_json(PyList_GET_ITEM(_exp.get(), _idx));
        }
        if (_parent->is_object() || _parent->is_array()) { _parent << _array; }
        else { _parent = _array; }
    }
    else if (PyLong_CheckExact(_exp)) {
        // zdbg("Py_Long");
        zpt::json _value = zpt::json::integer((long long)PyLong_AsLong(_exp));
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (PyODict_CheckExact(_exp)) {
        // zdbg("Py_ODict");
        zpt::json _object = zpt::json::object();
        underlying_type _key = nullptr;
        underlying_type _value = nullptr;
        Py_ssize_t _pos = 0;
        while (PyDict_Next(_exp, &_pos, &_key, &_value)) {
            _object << std::string(this->to_json(_key)) << this->to_json(_value);
        }
        if (_parent->is_object() || _parent->is_array()) { _parent << _object; }
        else { _parent = _object; }
    }
    else if (PyTuple_CheckExact(_exp)) {
        // zdbg("Py_Tuple");
        zpt::json _array = zpt::json::array();
        for (long _idx = 0; _idx != PyTuple_GET_SIZE(_exp.get()); ++_idx) {
            _array << this->to_json(PyTuple_GET_ITEM(_exp.get(), _idx));
        }
        if (_parent->is_object() || _parent->is_array()) { _parent << _array; }
        else { _parent = _array; }
    }
    else if (PyUnicode_CheckExact(_exp)) {
        // zdbg("Py_Unicode");
        underlying_type _bytes = PyUnicode_EncodeLocale(_exp, nullptr);
        std::string _s = std::string(PyBytes_AsString(_bytes), PyBytes_Size(_bytes));
        zpt::json _value = zpt::json::string(_s);
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (Py_TYPE(_exp) == &PyFilter_Type) {
        expect(Py_TYPE(_exp) != &PyFilter_Type, "Python: unmanaged python type PyFilter", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyMap_Type) {
        expect(Py_TYPE(_exp) != &PyMap_Type, "Python: unmanaged python type PyMap", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyZip_Type) {
        expect(Py_TYPE(_exp) != &PyZip_Type, "Python: unmanaged python type PyZip", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyByteArrayIter_Type) {
        expect(Py_TYPE(_exp) != &PyByteArrayIter_Type,
               "Python: unmanaged python type PyByteArrayIter",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyBytesIter_Type) {
        expect(
          Py_TYPE(_exp) != &PyBytesIter_Type, "Python: unmanaged python type PyBytesIter", 500, 0);
    }
    else if (PyCell_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyCell_Type, "Python: unmanaged python type PyCell", 500, 0);
    }
    else if (PyMethod_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyMethod_Type, "Python: unmanaged python type PyMethod", 500, 0);
    }
    else if (PyInstanceMethod_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyInstanceMethod_Type,
               "Python: unmanaged python type PyInstanceMethod",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyClassMethodDescr_Type) {
        expect(Py_TYPE(_exp) != &PyClassMethodDescr_Type,
               "Python: unmanaged python type PyClassMethodDescr",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyGetSetDescr_Type) {
        expect(Py_TYPE(_exp) != &PyGetSetDescr_Type,
               "Python: unmanaged python type PyGetSetDescr",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyMemberDescr_Type) {
        expect(Py_TYPE(_exp) != &PyMemberDescr_Type,
               "Python: unmanaged python type PyMemberDescr",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyMethodDescr_Type) {
        expect(Py_TYPE(_exp) != &PyClassMethodDescr_Type,
               "Python: unmanaged python type PyClassMethodDescr",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyMemberDescr_Type) {
        expect(Py_TYPE(_exp) != &PyMemberDescr_Type,
               "Python: unmanaged python type PyMemberDescr",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyDictProxy_Type) {
        expect(
          Py_TYPE(_exp) != &PyDictProxy_Type, "Python: unmanaged python type PyDictProxy", 500, 0);
    }
    else if (Py_TYPE(_exp) == &_PyMethodWrapper_Type) {
        expect(Py_TYPE(_exp) != &_PyMethodWrapper_Type,
               "Python: unmanaged python type _PyMethodWrapper",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyProperty_Type) {
        expect(
          Py_TYPE(_exp) != &PyProperty_Type, "Python: unmanaged python type PyProperty", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyDictIterKey_Type) {
        expect(Py_TYPE(_exp) != &PyDictIterKey_Type,
               "Python: unmanaged python type PyDictIterKey",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyDictIterValue_Type) {
        expect(Py_TYPE(_exp) != &PyDictIterValue_Type,
               "Python: unmanaged python type PyDictIterValue",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyDictIterItem_Type) {
        expect(Py_TYPE(_exp) != &PyDictIterItem_Type,
               "Python: unmanaged python type PyDictIterItem",
               500,
               0);
    }
    else if (PyDictKeys_Check(_exp)) {
        expect(
          Py_TYPE(_exp) != &PyDictKeys_Type, "Python: unmanaged python type PyDictKeys", 500, 0);
    }
    else if (PyDictItems_Check(_exp)) {
        expect(
          Py_TYPE(_exp) != &PyDictItems_Type, "Python: unmanaged python type PyDictItems", 500, 0);
    }
    else if (PyDictValues_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyDictValues_Type,
               "Python: unmanaged python type PyDictValues",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyEnum_Type) {
        expect(Py_TYPE(_exp) != &PyEnum_Type, "Python: unmanaged python type PyEnum", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyReversed_Type) {
        expect(
          Py_TYPE(_exp) != &PyReversed_Type, "Python: unmanaged python type PyReversed", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyStdPrinter_Type) {
        expect(Py_TYPE(_exp) != &PyStdPrinter_Type,
               "Python: unmanaged python type PyStdPrinter",
               500,
               0);
    }
    else if (PyCode_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyCode_Type, "Python: unmanaged python type PyCode", 500, 0);
    }
    else if (PyFunction_Check(_exp)) {
        // zdbg("Py_Function");
        std::ostringstream _oss;
        _oss << _exp << std::flush;
        zpt::json _value = zpt::json::lambda(_oss.str(), 0);
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (Py_TYPE(_exp) == &PyClassMethod_Type) {
        expect(Py_TYPE(_exp) != &PyClassMethod_Type,
               "Python: unmanaged python type PyClassMethod",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyStaticMethod_Type) {
        expect(Py_TYPE(_exp) != &PyStaticMethod_Type,
               "Python: unmanaged python type PyStaticMethod",
               500,
               0);
    }
    else if (PyGen_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyGen_Type, "Python: unmanaged python type PyGen", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyCoro_Type) {
        expect(Py_TYPE(_exp) != &PyCoro_Type, "Python: unmanaged python type PyCoro", 500, 0);
    }
    else if (Py_TYPE(_exp) == &_PyCoroWrapper_Type) {
        expect(Py_TYPE(_exp) != &_PyCoroWrapper_Type,
               "Python: unmanaged python type _PyCoroWrapper",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyAsyncGen_Type) {
        expect(
          Py_TYPE(_exp) != &PyAsyncGen_Type, "Python: unmanaged python type PyAsyncGen", 500, 0);
    }
    else if (Py_TYPE(_exp) == &_PyAsyncGenASend_Type) {
        expect(Py_TYPE(_exp) != &_PyAsyncGenASend_Type,
               "Python: unmanaged python type _PyAsyncGenASend",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &_PyAsyncGenWrappedValue_Type) {
        expect(Py_TYPE(_exp) != &_PyAsyncGenWrappedValue_Type,
               "Python: unmanaged python type _PyAsyncGenWrappedValue",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &_PyAsyncGenAThrow_Type) {
        expect(Py_TYPE(_exp) != &_PyAsyncGenAThrow_Type,
               "Python: unmanaged python type _PyAsyncGenAThrow",
               500,
               0);
    }
    else if (PySeqIter_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PySeqIter_Type, "Python: unmanaged python type PySeqIter", 500, 0);
    }
    else if (PyCallIter_Check(_exp)) {
        expect(
          Py_TYPE(_exp) != &PyCallIter_Type, "Python: unmanaged python type PyCallIter", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyListIter_Type) {
        expect(
          Py_TYPE(_exp) != &PyListIter_Type, "Python: unmanaged python type PyListIter", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyListRevIter_Type) {
        expect(Py_TYPE(_exp) != &PyListRevIter_Type,
               "Python: unmanaged python type PyListRevIter",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &_PyManagedBuffer_Type) {
        expect(Py_TYPE(_exp) != &_PyManagedBuffer_Type,
               "Python: unmanaged python type _PyManagedBuffer",
               500,
               0);
    }
    else if (PyMemoryView_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyMemoryView_Type,
               "Python: unmanaged python type PyMemoryView",
               500,
               0);
    }
    else if (PyCFunction_Check(_exp)) {
        expect(
          Py_TYPE(_exp) != &PyCFunction_Type, "Python: unmanaged python type PyCFunction", 500, 0);
    }
    else if (PyModule_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyModule_Type, "Python: unmanaged python type PyModule", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyModuleDef_Type) {
        expect(
          Py_TYPE(_exp) != &PyModuleDef_Type, "Python: unmanaged python type PyModuleDef", 500, 0);
    }
    else if (Py_TYPE(_exp) == &_PyNamespace_Type) {
        expect(Py_TYPE(_exp) != &_PyNamespace_Type,
               "Python: unmanaged python type _PyNamespace",
               500,
               0);
    }
    else if (PyType_Check(_exp)) {
        zpt::json _value = zpt::json::string(((PyTypeObject*)_exp.get())->tp_name);
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    else if (Py_TYPE(_exp) == &PyBaseObject_Type) {
        expect(Py_TYPE(_exp) != &PyBaseObject_Type,
               "Python: unmanaged python type PyBaseObject",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PySuper_Type) {
        expect(Py_TYPE(_exp) != &PySuper_Type, "Python: unmanaged python type PySuper", 500, 0);
    }
    else if (Py_TYPE(_exp) == &_PyNone_Type) {
        expect(Py_TYPE(_exp) != &_PyNone_Type, "Python: unmanaged python type _PyNone", 500, 0);
    }
    else if (Py_TYPE(_exp) == &_PyNotImplemented_Type) {
        expect(Py_TYPE(_exp) != &_PyNotImplemented_Type,
               "Python: unmanaged python type _PyNotImplemented",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyODictIter_Type) {
        expect(
          Py_TYPE(_exp) != &PyODictIter_Type, "Python: unmanaged python type PyODictIter", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyODictKeys_Type) {
        expect(
          Py_TYPE(_exp) != &PyODictKeys_Type, "Python: unmanaged python type PyODictKeys", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyODictItems_Type) {
        expect(Py_TYPE(_exp) != &PyODictItems_Type,
               "Python: unmanaged python type PyODictItems",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyODictValues_Type) {
        expect(Py_TYPE(_exp) != &PyODictValues_Type,
               "Python: unmanaged python type PyODictValues",
               500,
               0);
    }
    else if (Py_TYPE(_exp) == &PyCapsule_Type) {
        expect(Py_TYPE(_exp) != &PyCapsule_Type, "Python: unmanaged python type PyCapsule", 500, 0);
    }
    else if (PyRange_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PyRange_Type, "Python: unmanaged python type PyRange", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyRangeIter_Type) {
        expect(
          Py_TYPE(_exp) != &PyRangeIter_Type, "Python: unmanaged python type PyRangeIter", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyLongRangeIter_Type) {
        expect(Py_TYPE(_exp) != &PyLongRangeIter_Type,
               "Python: unmanaged python type PyLongRangeIter",
               500,
               0);
    }
    else if (PySet_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PySet_Type, "Python: unmanaged python type PySet", 500, 0);
    }
    else if (PyFrozenSet_Check(_exp)) {
        expect(
          Py_TYPE(_exp) != &PyFrozenSet_Type, "Python: unmanaged python type PyFrozenSet", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PySetIter_Type) {
        expect(Py_TYPE(_exp) != &PySetIter_Type, "Python: unmanaged python type PySetIter", 500, 0);
    }
    else if (PySlice_Check(_exp)) {
        expect(Py_TYPE(_exp) != &PySlice_Type, "Python: unmanaged python type PySlice", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyEllipsis_Type) {
        expect(
          Py_TYPE(_exp) != &PyEllipsis_Type, "Python: unmanaged python type PyEllipsis", 500, 0);
    }
    else if (PyTraceBack_Check(_exp)) {
        expect(
          Py_TYPE(_exp) != &PyTraceBack_Type, "Python: unmanaged python type PyTraceBack", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyTupleIter_Type) {
        expect(
          Py_TYPE(_exp) != &PyTupleIter_Type, "Python: unmanaged python type PyTupleIter", 500, 0);
    }
    else if (Py_TYPE(_exp) == &PyUnicodeIter_Type) {
        expect(Py_TYPE(_exp) != &PyUnicodeIter_Type,
               "Python: unmanaged python type PyUnicodeIter",
               500,
               0);
    }
    else {
        // zdbg("SOME OTHER Py THINGY");
        zpt::json _value = this->to_ref(_exp);
        if (_parent->is_object() || _parent->is_array()) { _parent << _value; }
        else { _parent = _value; }
    }
    return _parent;
}

auto
zpt::python::bridge::to_ref(object_type _exp) -> zpt::json {
    if (_exp == nullptr) { return zpt::undefined; }
    this->initialize();
    std::ostringstream _oss;
    _oss << std::hex << _exp << std::flush;
    Py_INCREF(_exp);
    if (PyFunction_Check(_exp)) { return zpt::json::lambda(_oss.str(), 0); }
    else { return zpt::json::string(std::string("ref(") + _oss.str() + std::string(")")); }
}

auto
zpt::python::bridge::to_object(zpt::json _to_convert) -> object_type {
    object_type _ret = nullptr;
    this->initialize();
    switch (_to_convert->type()) {
        case zpt::JSObject: {
            _ret = PyDict_New();
            for (auto [_, _json_key, _json_value] : _to_convert) {
                object_type _key = this->to_object(zpt::json::string(_json_key));
                object_type _value = this->to_object(_json_value);
                PyDict_SetItem(_ret, _key, _value);
            }
            break;
        }
        case zpt::JSArray: {
            _ret = PyList_New(_to_convert->size());
            for (auto [_idx, _, _json_value] : _to_convert) {
                object_type _value = this->to_object(_json_value);
                PyList_SET_ITEM(_ret.get(), _idx, _value.get());
            }
            break;
        }
        case zpt::JSString: {
            if (_to_convert->string().find("ref(") == 0) { _ret = this->from_ref(_to_convert); }
            else { _ret = PyUnicode_DecodeLocale(_to_convert->string().data(), nullptr); }
            break;
        }
        case zpt::JSDate: {
            object_type _timestamp = PyLong_FromLong((zpt::timestamp_t)_to_convert);
            _ret = PyDateTime_FromTimestamp(_timestamp);
            break;
        }
        case zpt::JSBoolean: {
            _ret = (bool(_to_convert) ? Py_True : Py_False);
            break;
        }
        case zpt::JSInteger: {
            _ret = PyLong_FromLong((long long)_to_convert);
            break;
        }
        case zpt::JSDouble: {
            _ret = PyFloat_FromDouble((double)_to_convert);
            break;
        }
        case zpt::JSLambda: {
            unsigned long _ref = 0;
            std::istringstream _iss;
            _iss.str(_to_convert->lambda()->name());
            _iss >> std::hex >> _ref;
            _ret = (underlying_type)_ref;
            break;
        }
        case zpt::JSNil: {
            _ret = Py_None;
            break;
        }
        default: {
            break;
        }
    }
    return _ret;
}

auto
zpt::python::bridge::from_ref(zpt::json _to_convert) -> object_type {
    this->initialize();
    if (_to_convert->is_lambda()) {
        unsigned long _ref = 0;
        std::istringstream _iss;
        _iss.str(_to_convert->lambda()->name());
        _iss >> std::hex >> _ref;
        return (PyObject*)_ref;
    }
    else {
        std::string _s_ref = std::string(_to_convert);
        zpt::replace(_s_ref, "ref(", "");
        zpt::replace(_s_ref, ")", "");
        unsigned long _ref = 0;
        std::istringstream _iss;
        _iss.str(_s_ref);
        _iss >> std::hex >> _ref;
        return (PyObject*)_ref;
    }
    return nullptr;
}

auto
zpt::python::bridge::execute(zpt::json _func_name, zpt::json _args)
  -> zpt::python::bridge::object_type {
    this->initialize();

    expect(_func_name->is_object(), "Python: cannot call a null function", 500, 0);

    object_type _func = this->locate(_func_name);
    object_type _tuple = PyTuple_New(_args->size());
    for (auto [_k, __, _arg] : _args) {
        auto _value = this->to_object(_arg);
        PyTuple_SET_ITEM(_tuple.get(), _k, _value.get());
    }

    return this->execute(_func, _tuple);
}

auto
zpt::python::bridge::execute(object_type _func, object_type _args)
  -> zpt::python::bridge::object_type {
    this->initialize();

    expect(_func.get() != nullptr, "Python: cannot call a null function", 500, 0);
    expect(PyCallable_Check(_func.get()) == 1,
           "Python: first argument isnnot a callable python object",
           500,
           0);
    expect(PyTuple_Check(_args.get()) == 1, "Python: arguments must be a tuple", 500, 0);

    PyErr_Clear();
    object_type _ret = PyObject_CallObject(_func.get(), _args.get());

    PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
    PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);
    expect(_py_error_type == nullptr,
           "Python: error invoking callable object: " << this->to_json(_py_error),
           500,
           0);
    return _ret;
}

auto
zpt::python::bridge::execute(zpt::json _self, std::string _func_name, std::nullptr_t)
  -> zpt::python::bridge::object_type {
    this->initialize();
    expect(_self->ok(), "Python: cannot call a function over a null instance", 500, 0);
    return this->execute(this->to_object(_self), _func_name, nullptr);
}

auto
zpt::python::bridge::execute(object_type _self, std::string _func_name, std::nullptr_t)
  -> zpt::python::bridge::object_type {
    this->initialize();

    expect(_self != nullptr, "Python: cannot call a function over a null instance", 500, 0);
    auto _func = this->to_object(_func_name);

    PyErr_Clear();
    object_type _ret = PyObject_CallMethodObjArgs(_self, _func, nullptr);

    PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
    PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);

    expect(_py_error_type == nullptr,
           "Python: error invoking callable object: " << this->to_json(_py_error),
           500,
           0);
    return _ret;
}

auto
zpt::python::bridge::initialize() -> zpt::python::bridge& {
    auto _not_initialized{ false };
    if (this->__initialized.compare_exchange_strong(_not_initialized, true)) {
        Py_SetProgramName((wchar_t*)("zpt"));

        struct _inittab _initt[this->__builtin_to_load.size() + 1];
        size_t _k{ 0 };
        for (auto [_name, _value] : this->__builtin_to_load) {
            auto [_init_callback, _conf] = _value;
            _initt[_k].name = _name.data();
            _initt[_k].initfunc = _init_callback;
            ++_k;
        }
        _initt[_k].name = nullptr;
        _initt[_k].initfunc = nullptr;
        expect(PyImport_ExtendInittab(_initt) != -1, "Python: could not import modules", 500, 0);

        Py_Initialize();

        for (auto [_name, _] : this->__builtin_to_load) {
            object_type _module = PyImport_ImportModule(_name.data());
            expect(_module != nullptr, "Python: unable to load module '" << _name << "'", 500, 0);
            zlog("Python: loaded module '" << _name << "'", zpt::notice);
            this->__modules.insert(std::make_pair(_name, _module));
        }

        if (this->options()["sys_path"]->ok() && this->options()["sys_path"]->is_array()) {
            zpt::json _new_sys_path =
              this->to_json(PySys_GetObject("path")) + this->options()["sys_path"];
            std::string _sys_path = zpt::join(_new_sys_path, ":");
            PySys_SetPath(zpt::utf8::utf8_to_wstring(_sys_path));
        }

        for (auto [_name, _value] : this->__external_to_load) {
            object_type _module = PyImport_ImportModule(_name.data());
            expect(_module != nullptr,
                   "Python: unable to load module '" << _value["name"]->string() << "'",
                   500,
                   0);
            zlog("Python: loaded module '" << _value["name"]->string() << "'", zpt::notice);
            this->__modules.insert(std::make_pair(_value["name"]->string(), _module));
        }
    }
    return (*this);
}

auto
zpt::python::bridge::is_initialized() const -> bool {
    return this->__initialized.load();
}
