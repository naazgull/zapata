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
#include <zapata/python/Bridge.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>
#include <datetime.h>

auto zpt::python::from_python(PyObject* _in) -> zpt::json {
	zpt::json _parent;
	zpt::python::from_python(_in, _parent);
	return _parent;
}

/**
  s (str) [const char *]
  s* (str or bytes-like object) [Py_buffer]
  s# (str, read-only bytes-like object) [const char *, int or Py_ssize_t]
  z (str or None) [const char *]
  z* (str, bytes-like object or None) [Py_buffer]
  z# (str, read-only bytes-like object or None) [const char *, int]
  y (read-only bytes-like object) [const char *]
  y* (bytes-like object) [Py_buffer]
  y# (read-only bytes-like object) [const char *, int]
  S (bytes) [PyBytesObject *]
  Y (bytearray) [PyByteArrayObject *]
  u (str) [Py_UNICODE *]
  u# (str) [Py_UNICODE *, int]
  Z (str or None) [Py_UNICODE *]
  Z# (str or None) [Py_UNICODE *, int]
  U (str) [PyObject *]
  w* (read-write bytes-like object) [Py_buffer]
  es (str) [const char *encoding, char **buffer]
  et (str, bytes or bytearray) [const char *encoding, char **buffer]
  es# (str) [const char *encoding, char **buffer, int *buffer_length]
  et# (str, bytes or bytearray) [const char *encoding, char **buffer, int *buffer_length]
  b (int) [unsigned char]
  B (int) [unsigned char]
  h (int) [short int]
  H (int) [unsigned short int]
  i (int) [int]
  I (int) [unsigned int]
  l (int) [long int]
  k (int) [unsigned long]
  L (int) [long long]
  K (int) [unsigned long long]
  n (int) [Py_ssize_t]
  c (bytes or bytearray of length 1) [char]
  C (str of length 1) [int]
  f (float) [float]
  d (float) [double]
  D (complex) [Py_complex]
  O (object) [PyObject *]
  O! (object) [typeobject, PyObject *]
  O& (object) [converter, anything]
  p (bool) [int]
**/

auto zpt::python::from_python(PyObject* _exp, zpt::json& _parent) -> void {
	try {
		if (_exp == nullptr || _exp == Py_None) {
			// zdbg("Py_None");
			if (_parent->is_object() || _parent->is_array()) {
				_parent << zpt::undefined;
			}
			else {
				_parent = zpt::undefined;
			}
		}
		else if (PyBool_Check(_exp)) {
			// zdbg("Py_Bool");
			zpt::json _value = zpt::json::boolean(_exp == Py_True ? true : false);
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (PyByteArray_CheckExact(_exp)) {
			// zdbg("Py_ByteArray");
			zpt::json _value = zpt::json::string(std::string(PyByteArray_AsString(_exp), PyByteArray_Size(_exp))); 
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (PyBytes_CheckExact(_exp)) {
			// zdbg("Py_Bytes");
			zpt::json _value = zpt::json::string(std::string(PyBytes_AsString(_exp), PyBytes_Size(_exp))); 
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (PyComplex_CheckExact(_exp)) {
			// zdbg("Py_Complex");
			zpt::json _value = zpt::json({ "real", PyComplex_RealAsDouble(_exp), "imaginary", PyComplex_ImagAsDouble(_exp) });
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (PyDict_CheckExact(_exp)) {
			// zdbg("Py_Dict");
			zpt::json _object = zpt::json::object();
			PyObject* _key = nullptr;
			PyObject* _value = nullptr;
			Py_ssize_t _pos = 0;
			while (PyDict_Next(_exp, &_pos, &_key, &_value)) {
				_object << std::string(zpt::python::from_python(_key)) << zpt::python::from_python(_value);
			}
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _object;
			}
			else {
				_parent = _object;
			}
		}
		else if (PyFloat_CheckExact(_exp)) {
			// zdbg("Py_Float");
			zpt::json _value = zpt::json::floating(PyFloat_AsDouble(_exp));
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (PyList_CheckExact(_exp)) {
			// zdbg("Py_List");
			zpt::json _array = zpt::json::array();
			for (long _idx = 0; _idx != PyList_GET_SIZE(_exp); _idx++) {
				_array << zpt::python::from_python(PyList_GET_ITEM(_exp, _idx));
			}
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _array;
			}
			else {
				_parent = _array;
			}
		}
		else if (PyLong_CheckExact(_exp)) {
			// zdbg("Py_Long");
			zpt::json _value = zpt::json::integer((long long) PyLong_AsLong(_exp));
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (PyODict_CheckExact(_exp)) {
			// zdbg("Py_ODict");
			zpt::json _object = zpt::json::object();
			PyObject* _key = nullptr;
			PyObject* _value = nullptr;
			Py_ssize_t _pos = 0;
			while (PyDict_Next(_exp, &_pos, &_key, &_value)) {
				_object << std::string(zpt::python::from_python(_key)) << zpt::python::from_python(_value);
			}
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _object;
			}
			else {
				_parent = _object;
			}
		}
		else if (PyTuple_CheckExact(_exp)) {
			// zdbg("Py_Tuple");
			zpt::json _array = zpt::json::array();
			for (long _idx = 0; _idx != PyTuple_GET_SIZE(_exp); _idx++) {
				_array << zpt::python::from_python(PyTuple_GET_ITEM(_exp, _idx));
			}
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _array;
			}
			else {
				_parent = _array;
			}
		}
		else if (PyUnicode_CheckExact(_exp)) {
			// zdbg("Py_Unicode");
			PyObject* _bytes = PyUnicode_EncodeLocale(_exp, nullptr);
			std::string _s = std::string(PyBytes_AsString(_bytes), PyBytes_Size(_bytes));
			zpt::json _value = zpt::json::string(_s);
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		/*else if (PyDate_CheckExact(_exp)) {
		  zpt::json _value = zpt::json::date(std::to_string(PyDateTime_GET_YEAR(_exp)) + std::string("-") + std::to_string(PyDateTime_GET_MONTH(_exp)) + std::string("-") + std::to_string(PyDateTime_GET_DAY(_exp)) + std::string("T00:00:00.000") + zpt::tostr(0, "%z"));
		  if (_parent->is_object() || _parent->is_array()) {
		  _parent << _value;
		  }
		  else {
		  _parent = _value;
		  }
		  }
		  else if (PyDateTime_CheckExact(_exp)) {
		  zpt::json _value = zpt::json::date(std::to_string(PyDateTime_GET_YEAR(_exp)) + std::string("-") + std::to_string(PyDateTime_GET_MONTH(_exp)) + std::string("-") + std::to_string(PyDateTime_GET_DAY(_exp)) + std::string("T") + std::to_string(PyDateTime_DATE_GET_HOUR(_exp)) + std::string(":") + std::to_string(PyDateTime_DATE_GET_MINUTE(_exp)) + std::string(":") + std::to_string(PyDateTime_DATE_GET_SECOND(_exp)) + std::string(".") + std::to_string((unsigned long long) (PyDateTime_DATE_GET_MICROSECOND(_exp) / 1000)) + zpt::tostr(0, "%z"));
		  if (_parent->is_object() || _parent->is_array()) {
		  _parent << _value;
		  }
		  else {
		  _parent = _value;
		  }
		  }
		  else if (PyTime_CheckExact(_exp)) {
		  zpt::json _value = zpt::json::date(std::string("1970-01-01T") + std::to_string(PyDateTime_TIME_GET_HOUR(_exp)) + std::string(":") + std::to_string(PyDateTime_TIME_GET_MINUTE(_exp)) + std::string(":") + std::to_string(PyDateTime_TIME_GET_SECOND(_exp)) + std::string(".") + std::to_string((unsigned long long) (PyDateTime_TIME_GET_MICROSECOND(_exp) / 1000)) + zpt::tostr(0, "%z"));
		  if (_parent->is_object() || _parent->is_array()) {
		  _parent << _value;
		  }
		  else {
		  _parent = _value;
		  }
		  }
		  else if (PyDelta_CheckExact(_exp)) {
		  zpt::json _value = zpt::json::integer((PyDateTime_DELTA_GET_DAYS(_exp) * 24 * 3600 * 1000 + PyDateTime_DELTA_GET_SECONDS(_exp) * 1000 + ((unsigned long long) (PyDateTime_DELTA_GET_MICROSECONDS(_exp) / 1000))));
		  if (_parent->is_object() || _parent->is_array()) {
		  _parent << _value;
		  }
		  else {
		  _parent = _value;
		  }
		  }*/
		else if (Py_TYPE(_exp) == &PyFilter_Type) {
			assertz(Py_TYPE(_exp) != &PyFilter_Type, std::string("unmanaged python type PyFilter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyMap_Type) {
			assertz(Py_TYPE(_exp) != &PyMap_Type, std::string("unmanaged python type PyMap"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyZip_Type) {
			assertz(Py_TYPE(_exp) != &PyZip_Type, std::string("unmanaged python type PyZip"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyByteArrayIter_Type) {
			assertz(Py_TYPE(_exp) != &PyByteArrayIter_Type, std::string("unmanaged python type PyByteArrayIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyBytesIter_Type) {
			assertz(Py_TYPE(_exp) != &PyBytesIter_Type, std::string("unmanaged python type PyBytesIter"), 500, 0);
		}
		else if (PyCell_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyCell_Type, std::string("unmanaged python type PyCell"), 500, 0);
		}
		else if (PyMethod_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyMethod_Type, std::string("unmanaged python type PyMethod"), 500, 0);
		}
		else if (PyInstanceMethod_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyInstanceMethod_Type, std::string("unmanaged python type PyInstanceMethod"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyClassMethodDescr_Type) {
			assertz(Py_TYPE(_exp) != &PyClassMethodDescr_Type, std::string("unmanaged python type PyClassMethodDescr"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyGetSetDescr_Type) {
			assertz(Py_TYPE(_exp) != &PyGetSetDescr_Type, std::string("unmanaged python type PyGetSetDescr"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyMemberDescr_Type) {
			assertz(Py_TYPE(_exp) != &PyMemberDescr_Type, std::string("unmanaged python type PyMemberDescr"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyMethodDescr_Type) {
			assertz(Py_TYPE(_exp) != &PyClassMethodDescr_Type, std::string("unmanaged python type PyClassMethodDescr"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyMemberDescr_Type) {
			assertz(Py_TYPE(_exp) != &PyMemberDescr_Type, std::string("unmanaged python type PyMemberDescr"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyDictProxy_Type) {
			assertz(Py_TYPE(_exp) != &PyDictProxy_Type, std::string("unmanaged python type PyDictProxy"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyMethodWrapper_Type) {
			assertz(Py_TYPE(_exp) != &_PyMethodWrapper_Type, std::string("unmanaged python type _PyMethodWrapper"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyProperty_Type) {
			assertz(Py_TYPE(_exp) != &PyProperty_Type, std::string("unmanaged python type PyProperty"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyDictIterKey_Type) {
			assertz(Py_TYPE(_exp) != &PyDictIterKey_Type, std::string("unmanaged python type PyDictIterKey"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyDictIterValue_Type) {
			assertz(Py_TYPE(_exp) != &PyDictIterValue_Type, std::string("unmanaged python type PyDictIterValue"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyDictIterItem_Type) {
			assertz(Py_TYPE(_exp) != &PyDictIterItem_Type, std::string("unmanaged python type PyDictIterItem"), 500, 0);
		}
		else if (PyDictKeys_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyDictKeys_Type, std::string("unmanaged python type PyDictKeys"), 500, 0);
		}
		else if (PyDictItems_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyDictItems_Type, std::string("unmanaged python type PyDictItems"), 500, 0);
		}
		else if (PyDictValues_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyDictValues_Type, std::string("unmanaged python type PyDictValues"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyEnum_Type) {
			assertz(Py_TYPE(_exp) != &PyEnum_Type, std::string("unmanaged python type PyEnum"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyReversed_Type) {
			assertz(Py_TYPE(_exp) != &PyReversed_Type, std::string("unmanaged python type PyReversed"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyStdPrinter_Type) {
			assertz(Py_TYPE(_exp) != &PyStdPrinter_Type, std::string("unmanaged python type PyStdPrinter"), 500, 0);
		}
		else if (PyCode_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyCode_Type, std::string("unmanaged python type PyCode"), 500, 0);
		}
		else if (PyFunction_Check(_exp)) {
			// zdbg("Py_Function");
			std::ostringstream _oss;
			_oss << _exp << flush;
			zpt::json _value = zpt::json::lambda(_oss.str(), 0);
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}		
		}
		else if (Py_TYPE(_exp) == &PyClassMethod_Type) {
			assertz(Py_TYPE(_exp) != &PyClassMethod_Type, std::string("unmanaged python type PyClassMethod"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyStaticMethod_Type) {
			assertz(Py_TYPE(_exp) != &PyStaticMethod_Type, std::string("unmanaged python type PyStaticMethod"), 500, 0);
		}
		else if (PyGen_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyGen_Type, std::string("unmanaged python type PyGen"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyCoro_Type) {
			assertz(Py_TYPE(_exp) != &PyCoro_Type, std::string("unmanaged python type PyCoro"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyCoroWrapper_Type) {
			assertz(Py_TYPE(_exp) != &_PyCoroWrapper_Type, std::string("unmanaged python type _PyCoroWrapper"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyAIterWrapper_Type) {
			assertz(Py_TYPE(_exp) != &_PyAIterWrapper_Type, std::string("unmanaged python type _PyAIterWrapper"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyAsyncGen_Type) {
			assertz(Py_TYPE(_exp) != &PyAsyncGen_Type, std::string("unmanaged python type PyAsyncGen"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyAsyncGenASend_Type) {
			assertz(Py_TYPE(_exp) != &_PyAsyncGenASend_Type, std::string("unmanaged python type _PyAsyncGenASend"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyAsyncGenWrappedValue_Type) {
			assertz(Py_TYPE(_exp) != &_PyAsyncGenWrappedValue_Type, std::string("unmanaged python type _PyAsyncGenWrappedValue"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyAsyncGenAThrow_Type) {
			assertz(Py_TYPE(_exp) != &_PyAsyncGenAThrow_Type, std::string("unmanaged python type _PyAsyncGenAThrow"), 500, 0);
		}
		else if (PySeqIter_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PySeqIter_Type, std::string("unmanaged python type PySeqIter"), 500, 0);
		}
		else if (PyCallIter_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyCallIter_Type, std::string("unmanaged python type PyCallIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyListIter_Type) {
			assertz(Py_TYPE(_exp) != &PyListIter_Type, std::string("unmanaged python type PyListIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyListRevIter_Type) {
			assertz(Py_TYPE(_exp) != &PyListRevIter_Type, std::string("unmanaged python type PyListRevIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyManagedBuffer_Type) {
			assertz(Py_TYPE(_exp) != &_PyManagedBuffer_Type, std::string("unmanaged python type _PyManagedBuffer"), 500, 0);
		}
		else if (PyMemoryView_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyMemoryView_Type, std::string("unmanaged python type PyMemoryView"), 500, 0);
		}
		else if (PyCFunction_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyCFunction_Type, std::string("unmanaged python type PyCFunction"), 500, 0);
		}
		else if (PyModule_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyModule_Type, std::string("unmanaged python type PyModule"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyModuleDef_Type) {
			assertz(Py_TYPE(_exp) != &PyModuleDef_Type, std::string("unmanaged python type PyModuleDef"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyNamespace_Type) {
			assertz(Py_TYPE(_exp) != &_PyNamespace_Type, std::string("unmanaged python type _PyNamespace"), 500, 0);
		}
		else if (PyType_Check(_exp)) {
			zpt::json _value = zpt::json::string(((PyTypeObject*)_exp)->tp_name);
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}
		}
		else if (Py_TYPE(_exp) == &PyBaseObject_Type) {
			assertz(Py_TYPE(_exp) != &PyBaseObject_Type, std::string("unmanaged python type PyBaseObject"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PySuper_Type) {
			assertz(Py_TYPE(_exp) != &PySuper_Type, std::string("unmanaged python type PySuper"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyNone_Type) {
			assertz(Py_TYPE(_exp) != &_PyNone_Type, std::string("unmanaged python type _PyNone"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &_PyNotImplemented_Type) {
			assertz(Py_TYPE(_exp) != &_PyNotImplemented_Type, std::string("unmanaged python type _PyNotImplemented"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyODictIter_Type) {
			assertz(Py_TYPE(_exp) != &PyODictIter_Type, std::string("unmanaged python type PyODictIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyODictKeys_Type) {
			assertz(Py_TYPE(_exp) != &PyODictKeys_Type, std::string("unmanaged python type PyODictKeys"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyODictItems_Type) {
			assertz(Py_TYPE(_exp) != &PyODictItems_Type, std::string("unmanaged python type PyODictItems"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyODictValues_Type) {
			assertz(Py_TYPE(_exp) != &PyODictValues_Type, std::string("unmanaged python type PyODictValues"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyCapsule_Type) {
			assertz(Py_TYPE(_exp) != &PyCapsule_Type, std::string("unmanaged python type PyCapsule"), 500, 0);
		}
		else if (PyRange_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyRange_Type, std::string("unmanaged python type PyRange"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyRangeIter_Type) {
			assertz(Py_TYPE(_exp) != &PyRangeIter_Type, std::string("unmanaged python type PyRangeIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyLongRangeIter_Type) {
			assertz(Py_TYPE(_exp) != &PyLongRangeIter_Type, std::string("unmanaged python type PyLongRangeIter"), 500, 0);
		}
		else if (PySet_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PySet_Type, std::string("unmanaged python type PySet"), 500, 0);
		}
		else if (PyFrozenSet_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyFrozenSet_Type, std::string("unmanaged python type PyFrozenSet"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PySetIter_Type) {
			assertz(Py_TYPE(_exp) != &PySetIter_Type, std::string("unmanaged python type PySetIter"), 500, 0);
		}
		else if (PySlice_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PySlice_Type, std::string("unmanaged python type PySlice"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyEllipsis_Type) {
			assertz(Py_TYPE(_exp) != &PyEllipsis_Type, std::string("unmanaged python type PyEllipsis"), 500, 0);
		}
		else if (PyTraceBack_Check(_exp)) {
			assertz(Py_TYPE(_exp) != &PyTraceBack_Type, std::string("unmanaged python type PyTraceBack"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyTupleIter_Type) {
			assertz(Py_TYPE(_exp) != &PyTupleIter_Type, std::string("unmanaged python type PyTupleIter"), 500, 0);
		}
		else if (Py_TYPE(_exp) == &PyUnicodeIter_Type) {
			assertz(Py_TYPE(_exp) != &PyUnicodeIter_Type, std::string("unmanaged python type PyUnicodeIter"), 500, 0);
		}	 
		/*else if (PyTZInfo_CheckExact(_exp)) {
		  assertz(!PyTZInfo_Check(_exp), std::string("unmanaged python type PyTZInfo"), 500, 0);
		  }*/
		else {
			// zdbg("SOME OTHER Py THINGY");
			zpt::json _value = zpt::python::to_ref(_exp);
			if (_parent->is_object() || _parent->is_array()) {
				_parent << _value;
			}
			else {
				_parent = _value;
			}		
		}
	}
	catch (zpt::assertion& _e) {
		zlog(_e.description(), zpt::error);
		zlog(std::string("\n") + _e.backtrace(), zpt::trace);
	}
	catch (std::exception& _e) {
		zlog(_e.what(), zpt::error);
	}
}

auto zpt::python::to_python(zpt::json _in, zpt::python::bridge* _bridge) -> zpt::python::object {
	return zpt::python::object(zpt::python::to_python(_in));
}

auto zpt::python::from_ref(zpt::json _in) -> PyObject* {
	if (_in->is_lambda()) {
		unsigned long _ref = 0;
		std::istringstream _iss;
		_iss.str(_in->lbd()->name());
		_iss >> std::hex >> _ref;
		return (PyObject*) _ref;
	}
	else {
		std::string _s_ref = std::string(_in);
		zpt::replace(_s_ref, "ref(", "");
		zpt::replace(_s_ref, ")", "");
		unsigned long _ref = 0;
		std::istringstream _iss;
		_iss.str(_s_ref);
		_iss >> std::hex >> _ref;
		return (PyObject*) _ref;
	}
}

auto zpt::python::to_ref(PyObject* _in) -> zpt::json {
	std::ostringstream _oss;
	_oss << _in << flush;
	Py_INCREF(_in);
	if (PyFunction_Check(_in)) {
		return zpt::json::lambda(_oss.str(), 0);
	}
	else {
		return zpt::json::string(std::string("ref(") + _oss.str() + std::string(")"));
	}
}

auto zpt::python::to_python(zpt::json _in) -> PyObject* {
	PyObject* _ret = nullptr;
	switch (_in->type()) {
		case zpt::JSObject : {
			_ret = PyDict_New();
			Py_INCREF(_ret);
			for (auto _o : _in->obj()) {
				PyObject* _key = zpt::python::to_python(zpt::json::string(_o.first));
				PyObject* _value = zpt::python::to_python(_o.second);
				PyDict_SetItem(_ret, _key, _value);
			}
			break;
		}
		case zpt::JSArray : {
			_ret = PyList_New(_in->arr()->size());
			Py_INCREF(_ret);
			size_t _idx = 0;
			for (auto _o : _in->arr()) {
				PyObject* _value = zpt::python::to_python(_o);
				PyList_SET_ITEM(_ret, _idx, _value);
				_idx++;
			}
			break;
		}
		case zpt::JSString: {
			//_ret = PyUnicode_DecodeFSDefault(std::string(_in).data());
			if (std::string(_in).find("ref(") == 0) {
				_ret = zpt::python::from_ref(_in);
			}
			else {
				_ret = PyUnicode_DecodeLocale(std::string(_in).data(), nullptr);
				Py_INCREF(_ret);
			}
			break;
		}
		case zpt::JSDate: {
			PyObject* _timestamp = PyLong_FromLong((zpt::timestamp_t) _in);
			Py_INCREF(_timestamp);
			_ret = PyDate_FromTimestamp(_timestamp);
			Py_DECREF(_timestamp);
			Py_INCREF(_ret);
			break;
		}
		case zpt::JSBoolean: {
			_ret = (bool(_in) ? Py_True : Py_False);
			Py_INCREF(_ret);
			break;
		}
		case zpt::JSInteger: {
			_ret = PyLong_FromLong((long long) _in);
			Py_INCREF(_ret);
			break;
		}
		case zpt::JSDouble: {
			_ret = PyFloat_FromDouble((double) _in);
			Py_INCREF(_ret);
			break;
		}
		case zpt::JSLambda : {
			unsigned long _ref = 0;
			std::istringstream _iss;
			_iss.str(_in->lbd()->name());
			_iss >> std::hex >> _ref;
			_ret = (PyObject*) _ref;
			break;
		}
		case zpt::JSNil: {
			Py_RETURN_NONE;
		}
		default : {
			break;
		}
	}
	return _ret;
}

