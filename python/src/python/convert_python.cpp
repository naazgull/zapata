/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

auto zpt::python::from_python(PyObject* _in) -> zpt::json {
	zpt::json _parent;
	zpt::python::from_python(_in, _parent);
	return _parent;
}

auto zpt::python::from_python(PyObject* _exp, zpt::json& _parent) -> void {
}

auto zpt::python::to_python(zpt::json _in, zpt::python::bridge* _bridge) -> zpt::python::object {
	std::string _exp;
	return _bridge->eval(_exp);
}

auto zpt::python::to_python_string(zpt::json _json) -> std::string {
	std::string _ret;
	return _ret;
}

