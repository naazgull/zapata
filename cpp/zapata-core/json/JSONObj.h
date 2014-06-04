#pragma once

#include <ostream>
#include <vector>
#include <base/smart_ptr.h>
#include <base/str_map.h>

using namespace std;
using namespace __gnu_cxx;

#define JSON(z) ((zapata::JSONObj()) << z)
#define JSON_ARRAY(z) ((zapata::JSONArr()) << z)
#define JSON_NIL zapata::nil;

namespace zapata {

	enum JSONType {
		JSObject, JSArray, JSString, JSInteger, JSDouble, JSNumber, JSBoolean, JSNil
	};

	class JSONIntRef;
	typedef smart_ptr<JSONIntRef> JSONInt;

	class JSONBoolRef;
	typedef smart_ptr<JSONBoolRef> JSONBool;

	class JSONDblRef;
	typedef smart_ptr<JSONDblRef> JSONDbl;

	class JSONStrRef;
	typedef smart_ptr<JSONStrRef> JSONStr;

	class JSONElement;
	#define JSONNilRef JSONElement
	typedef smart_ptr<JSONNilRef> JSONNil;

	class JSONArrRef;
	typedef smart_ptr<JSONArrRef> JSONArr;
	typedef vector<smart_ptr<JSONElement>*>::iterator JSONArrIterator;

	class JSONObjRef;
	typedef smart_ptr<JSONObjRef> JSONObj;
	typedef str_map<smart_ptr<JSONElement>*>::iterator JSONObjIterator;

	class JSONElement {
		public:
			JSONElement();
			virtual ~JSONElement();

			virtual JSONType type();
			virtual short flags();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

			bool operator==(JSONElement& _in);
			bool operator==(string _in);
			bool operator==(bool _in);
			bool operator==(int _in);
			bool operator==(long _in);
			bool operator==(long long _in);
			bool operator==(double _in);

			bool operator!=(JSONElement& _in);
			bool operator!=(string _in);
			bool operator!=(bool _in);
			bool operator!=(int _in);
			bool operator!=(long _in);
			bool operator!=(long long _in);
			bool operator!=(double _in);

			operator string();
			operator bool();
			operator int();
			operator long();
			operator long long();
#ifdef __LP64__
			operator unsigned int();
#endif
			operator size_t();
			operator double();
			operator JSONObjRef&();
			operator JSONArrRef&();
			operator JSONElement&();

			JSONElement& operator<<(const char* _in);
			JSONElement& operator<<(bool _in);
			JSONElement& operator<<(int _in);
			JSONElement& operator<<(long _in);
			JSONElement& operator<<(long long _in);
			JSONElement& operator<<(double _in);
			JSONElement& operator<<(string _in);
			JSONElement& operator<<(ObjectOp _in);
			JSONElement& operator<<(JSONObj& _in);
			JSONElement& operator<<(JSONArr& _in);
			JSONElement& operator<<(JSONInt& _in);
			JSONElement& operator<<(JSONBool& _in);
			JSONElement& operator<<(JSONDbl& _in);
			JSONElement& operator<<(JSONStr& _in);
			JSONElement& operator<<(JSONNil& _in);

			JSONElement& operator>>(const char* _in);
			JSONElement& operator>>(long long _in);
			JSONElement& operator>>(string _in);
			JSONElement& operator>>(ObjectOp _in);

			JSONElement& operator[](int _idx);
			JSONElement& operator[](size_t _idx);
			JSONElement& operator[](const char* _idx);
			JSONElement& operator[](string& _idx);

			friend ostream& operator<<(ostream& _out, JSONElement& _in) {
				_in.stringify(_out, _in.__flags, "");
				return _out;
			}

		protected:
			short __flags;

			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(ObjectOp _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual void unset(long long _in);
			virtual void unset(string _in);
			virtual void unset(ObjectOp _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();
			virtual JSONElement& getJSONElement();
	};

	class JSONBoolRef: public JSONElement {
		private:
			bool __value;

		public:
			JSONBoolRef(bool _in = false);
			virtual ~JSONBoolRef();

			JSONType type();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();
	};

	class JSONIntRef: public JSONElement {
		private:
			long long __value;

		public:
			JSONIntRef(long long _in = 0);
			virtual ~JSONIntRef();

			JSONType type();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();
	};

	class JSONDblRef: public JSONElement {
		private:
			double __value;

		public:
			JSONDblRef(double _in = 0);
			virtual ~JSONDblRef();

			JSONType type();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();
	};

	class JSONStrRef: public JSONElement {
		private:
			string __value;
			public:
			JSONStrRef(string _in = "");
			virtual ~JSONStrRef();

			JSONType type();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();
	};

	class JSONArrRef: public JSONElement, public vector<smart_ptr<JSONElement>*> {
		public:
			JSONArrRef();
			virtual ~JSONArrRef();

			JSONType type();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

			JSONElement& operator[](int _idx);
			JSONElement& operator[](size_t _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual void unset(long long _in);
			virtual void unset(string _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();
	};

	class JSONObjRef: public JSONElement, public str_map<smart_ptr<JSONElement>*> {
		private:
			string* __name;

		public:
			JSONObjRef();
			virtual ~JSONObjRef();

			JSONType type();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out,short _flags = 0,  string _tabs = "");

			virtual JSONElement& get(size_t _idx);
			virtual JSONElement& get(const char* _idx);

			JSONElement& operator[](int _idx);
			JSONElement& operator[](size_t _idx);
			JSONElement& operator[](const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(unsigned int _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);
			virtual void put(JSONObj& _in);
			virtual void put(JSONArr& _in);
			virtual void put(JSONBool& _in);
			virtual void put(JSONInt& _in);
			virtual void put(JSONDbl& _in);
			virtual void put(JSONStr& _in);
			virtual void put(JSONNil& _in);

			virtual void unset(long long _in);
			virtual void unset(string _in);

			virtual bool compare(JSONElement& _in);

			virtual int getInt();
			virtual long getLong();
			virtual long getLongLong();
			virtual unsigned int getUnsignedInt();
			virtual double getDouble();
			virtual bool getBool();
			virtual string getString();
			virtual JSONObjRef& getJSONObj();
			virtual JSONArrRef& getJSONArr();

	};

	extern JSONElement& nil;

}
