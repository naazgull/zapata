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
#include <zapata/postgresql/convert_sql.h>

namespace zpt {
	namespace pgsql {

		std::map<std::string, std::string> OPS = { { "gt", ">" }, { "gte", ">=" }, { "lt", "<" }, { "lte", "<=" }, { "ne", "<>" }, { "exists", "EXISTS" }, { "in", "IN" } };
		
	}
}

auto zpt::pgsql::fromsql(pqxx::tuple _in, zpt::json _out) -> void {
	for (auto _f : _in) {
		_out << std::string(_f.name());
		switch(_f.type()) {
			// T_bool            
			case 16 : {
				_out << _f.as<bool>();
				break;
			} 
			// T_bytea           
			case 17 : {
				_out << _f.as<short>();
				break;
			} 
			// T_char
			case 18 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_name            
			case 19 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_int8            
			case 20 : {
				_out << _f.as<long long>();
				break;
			} 
			// T_int2            
			case 21 : {
				_out << _f.as<short>();
				break;
			} 
			// T_int2vector      
			case 22 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_int4
			case 23 : {
				_out << _f.as<int>();
				break;
			} 
			// T_regproc         
			case 24 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T_text            
			case 25 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_oid             
			case 26 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_tid             
			case 27 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_xid             
			case 28 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_cid             
			case 29 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_oidvector       
			case 30 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_pg_type         
			case 71 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_pg_attribute    
			case 75 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_pg_proc         
			case 81 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T_pg_class        
			case 83 : {
				_out << _f.as<std::string>();
				break;
			}
			// T_json            
			case 114 : {
				_out << zpt::json(_f.as<std::string>());
				break;
			} 
			// T_xml             
			case 142 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__xml            
			case 143 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_pg_node_tree    
			case 194 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__json           
			case 199 : {
				_out << zpt::json(_f.as<std::string>());
				break;
			} 
			// T_smgr            
			case 210 : {
				_out << _f.as<int>();
				break;
			} 
			// T_point           
			case 600 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_lseg            
			case 601 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_path            
			case 602 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_box             
			case 603 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_polygon         
			case 604 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_line            
			case 628 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__line           
			case 629 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_cidr            
			case 650 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__cidr           
			case 651 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_float4          
			case 700 : {
				_out << _f.as<double>();
				break;
			} 
			// T_float8          
			case 701 : {
				_out << _f.as<double>();
				break;
			} 
			// T_abstime         
			case 702 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_reltime         
			case 703 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_tinterval       
			case 704 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_unknown         
			case 705 : {
				_out << zpt::undefined;
				break;
			} 
			// T_circle          
			case 718 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ">", "]"), "<", "["), ")", "]"), "(", "["));
				break;
			} 
			// T__circle         
			case 719 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ">", "]"), "<", "["), ")", "]"), "(", "["));
				break;
			} 
			// T_money           
			case 790 : {
				_out << _f.as<double>();
				break;
			} 
			// T__money          
			case 791 : {
				_out << _f.as<double>();
				break;
			} 
			// T_macaddr         
			case 829 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_inet            
			case 869 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__bool           
			case 1000 : {
				_out << _f.as<bool>();
				break;
			} 
			// T__bytea          
			case 1001 : {
				_out << _f.as<short>();
				break;
			} 
			// T__char           
			case 1002 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__name           
			case 1003 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__int2           
			case 1005 : {
				_out << _f.as<int>();
				break;
			} 
			// T__int2vector     
			case 1006 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__int4           
			case 1007 : {
				_out << _f.as<int>();
				break;
			} 
			// T__regproc        
			case 1008 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T__text           
			case 1009 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__tid            
			case 1010 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T__xid            
			case 1011 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T__cid            
			case 1012 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T__oidvector      
			case 1013 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__bpchar         
			case 1014 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__varchar        
			case 1015 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__int8           
			case 1016 : {
				_out << _f.as<int>();
				break;
			} 
			// T__point          
			case 1017 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__lseg           
			case 1018 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__path           
			case 1019 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__box            
			case 1020 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__float4         
			case 1021 : {
				_out << _f.as<double>();
				break;
			} 
			// T__float8         
			case 1022 : {
				_out << _f.as<double>();
				break;
			} 
			// T__abstime        
			case 1023 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__reltime        
			case 1024 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__tinterval      
			case 1025 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__polygon        
			case 1027 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__oid            
			case 1028 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_aclitem         
			case 1033 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__aclitem        
			case 1034 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__macaddr        
			case 1040 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__inet           
			case 1041 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_bpchar          
			case 1042 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_varchar         
			case 1043 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_date            
			case 1082 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_time            
			case 1083 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_timestamp       
			case 1114 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__timestamp      
			case 1115 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__date           
			case 1182 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__time           
			case 1183 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_timestamptz     
			case 1184 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__timestamptz    
			case 1185 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_interval        
			case 1186 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__interval       
			case 1187 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__numeric        
			case 1231 : {
				_out << _f.as<double>();
				break;
			} 
			// T_pg_database     
			case 1248 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__cstring        
			case 1263 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_timetz          
			case 1266 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T__timetz         
			case 1270 : {
				_out << zpt::json::date(_f.as<zpt::timestamp_t>());
				break;
			} 
			// T_bit             
			case 1560 : {
				_out << _f.as<short>();
				break;
			} 
			// T__bit            
			case 1561 : {
				_out << _f.as<short>();
				break;
			} 
			// T_varbit          
			case 1562 : {
				_out << _f.as<short>();
				break;
			} 
			// T__varbit         
			case 1563 : {
				_out << _f.as<short>();
				break;
			} 
			// T_numeric         
			case 1700 : {
				_out << _f.as<double>();
				break;
			} 
			// T_refcursor       
			case 1790 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T__refcursor      
			case 2201 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_regprocedure    
			case 2202 : {
				std::string _functor = _f.as<std::string>();
				size_t _lparen = _functor.find("(");
				_functor = _functor.substr(0, _lparen);
				std::string _args = _functor.substr(_lparen, _functor.length() - _lparen - 1);
				_out << zpt::json::lambda(_functor, zpt::split(_args, ",")->arr()->size());
				break;
			} 
			// T_regoper         
			case 2203 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T_regoperator     
			case 2204 : {
				std::string _functor = _f.as<std::string>();
				size_t _lparen = _functor.find("(");
				_functor = _functor.substr(0, _lparen);
				std::string _args = _functor.substr(_lparen, _functor.length() - _lparen - 1);
				_out << zpt::json::lambda(_functor, zpt::split(_args, ",")->arr()->size());
				break;
			} 
			// T_regclass        
			case 2205 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T_regtype         
			case 2206 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T__regprocedure   
			case 2207 : {
				std::string _functor = _f.as<std::string>();
				size_t _lparen = _functor.find("(");
				_functor = _functor.substr(0, _lparen);
				std::string _args = _functor.substr(_lparen, _functor.length() - _lparen - 1);
				_out << zpt::json::lambda(_functor, zpt::split(_args, ",")->arr()->size());
				break;
			} 
			// T__regoper        
			case 2208 : {
				_out << zpt::json::lambda(_f.as<std::string>(), 0);
				break;
			} 
			// T__regoperator    
			case 2209 : {
				std::string _functor = _f.as<std::string>();
				size_t _lparen = _functor.find("(");
				_functor = _functor.substr(0, _lparen);
				std::string _args = _functor.substr(_lparen, _functor.length() - _lparen - 1);
				_out << zpt::json::lambda(_functor, zpt::split(_args, ",")->arr()->size());
				break;
			} 
			// T__regclass       
			case 2210 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__regtype        
			case 2211 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_record          
			case 2249 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_cstring         
			case 2275 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_any             
			case 2276 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_anyarray        
			case 2277 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_void            
			case 2278 : {
				_out << zpt::undefined;
				break;
			} 
			// T_trigger         
			case 2279 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_language_handler
			case 2280 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_internal        
			case 2281 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_opaque          
			case 2282 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_anyelement      
			case 2283 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__record         
			case 2287 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_anynonarray     
			case 2776 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_pg_authid       
			case 2842 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_pg_auth_members 
			case 2843 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__txid_snapshot  
			case 2949 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_uuid            
			case 2950 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__uuid           
			case 2951 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_txid_snapshot   
			case 2970 : {
				_out << _f.as<unsigned int>();
				break;
			} 
			// T_fdw_handler     
			case 3115 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_anyenum         
			case 3500 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_tsvector        
			case 3614 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_tsquery         
			case 3615 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_gtsvector       
			case 3642 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__tsvector       
			case 3643 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__gtsvector      
			case 3644 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__tsquery        
			case 3645 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_regconfig       
			case 3734 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__regconfig      
			case 3735 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_regdictionary   
			case 3769 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T__regdictionary  
			case 3770 : {
				_out << _f.as<std::string>();
				break;
			} 
			// T_anyrange        
			case 3831 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_event_trigger   
			case 3838 : {
				break;
			} 
			// T_int4range       
			case 3904 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__int4range      
			case 3905 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_numrange        
			case 3906 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__numrange       
			case 3907 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_tsrange         
			case 3908 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__tsrange        
			case 3909 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_tstzrange       
			case 3910 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__tstzrange      
			case 3911 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_daterange       
			case 3912 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__daterange      
			case 3913 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T_int8range       
			case 3926 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			} 
			// T__int8range Oid =
			case 3927 : {
				_out << zpt::json(zpt::r_replace(zpt::r_replace(_f.as<std::string>(), ")", "]"), "(", "["));
				break;
			}
			default : {
				_out << zpt::undefined;
				break;
			}
		}
	}
}

auto zpt::pgsql::fromsql_r(pqxx::tuple _in) -> zpt::json {
	zpt::json _return = zpt::json::object();
	zpt::pgsql::fromsql(_in, _return);
	return _return;
}

auto zpt::pgsql::get_query(zpt::json _in, std::string&  _queryr) -> void {
	if (_in->ok() && _in->type() == zpt::JSObject) {
		return;
	}
	for (auto _i : _in->obj()) {
		std::string _key = _i.first;
		zpt::json _v = _i.second;

		if (_queryr.length() != 0) {
			_queryr += std::string(" AND ");
		}

		std::string _value = (std::string) _v;
		if (_value.length() > 3 && _value.find('/') != std::string::npos) {
			int _bar_count = 0;
			std::istringstream _lss(_value);
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

					if (_expression.length() == 0 || _expression[_expression.length() - 1] != '\\') {
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
				_queryr += zpt::pgsql::escape(_key) + std::string("=") + zpt::pgsql::escape(_expression);
				continue;
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
							std::string _bexpr(_expression.data());
							std::transform(_bexpr.begin(), _bexpr.end(), _bexpr.begin(), ::tolower);
							if (_bexpr != "true" && _bexpr != "false") {
								_queryr += zpt::pgsql::escape(_key) + std::string("=") + zpt::pgsql::escape(_expression);
							}
							else {
								_queryr += zpt::pgsql::escape(_key) + std::string("=") + _bexpr;
							}
						}
						else {
							_queryr += zpt::pgsql::escape(_key) + std::string("=") + std::to_string(d);
						}
					}
					else {
						_queryr += zpt::pgsql::escape(_key) + std::string("=") + std::to_string(i);
					}
					continue;
				}
			}
			else {
				std::map<std::string, std::string>::iterator _found = zpt::pgsql::OPS.find(_command);
				if (_found != zpt::pgsql::OPS.end()) {
					if (_bar_count == 2) {
						_queryr += zpt::pgsql::escape(_key) + _found->second + zpt::pgsql::escape(_expression);
					}
					else if (_options == "n") {
						std::istringstream iss(_expression);
						int i = 0;
						iss >> i;
						if (!iss.eof()) {
							iss.clear();
							double d = 0;
							iss >> d;
							if (!iss.eof()) {
								std::string _bexpr(_expression.data());
								std::transform(_bexpr.begin(), _bexpr.end(), _bexpr.begin(), ::tolower);
								if (_bexpr != "true" && _bexpr != "false") {
									_queryr += zpt::pgsql::escape(_key) + _found->second + zpt::pgsql::escape(_expression);
								}
								else {
									_queryr += zpt::pgsql::escape(_key) + _found->second + _bexpr;
								}
							}
							else {
								_queryr += zpt::pgsql::escape(_key) + _found->second + std::to_string(d);
							}
						}
						else {
							_queryr += zpt::pgsql::escape(_key) + _found->second + std::to_string(i);
						}
					}
					else if (_options == "j") {
					}
					else if (_options == "d") {
						_queryr += zpt::pgsql::escape(_key) + _found->second + std::string("TIMESTAMP('") + zpt::pgsql::escape(_expression) + std::string("')");
					}
					continue;
				}

			}
		}

		_queryr += zpt::pgsql::escape(_key) + std::string("=") + zpt::pgsql::escape(_value);
	}
}

auto zpt::pgsql::get_opts(zpt::json _in, std::string&  _queryr) -> void {
	if (_in->ok() && _in->type() == zpt::JSObject) {
		return;
	}

	if (_in["page_size"]) {
		_queryr += std::string(" LIMIT ") + std::to_string(int(_in["page_size"]));
	}
	if (_in["page_start_index"]) {
		_queryr += std::string(" OFFSET ") + std::to_string(int(_in["page_start_index"]));
	}
	if (_in["order_by"]) {
		_queryr += std::string(" ORDER BY ");
		std::istringstream lss(((std::string) _in["order_by"]).data());
		std::string _part;
		bool _first = true;
		while (std::getline(lss, _part, ',')) {
			if (_part.length() > 0) {
				std::string _dir = "ASC";
				
				if (_part[0] == '-') {
					_dir = "DESC";
				}
				_part.erase(0, 1);
				_queryr += (!_first ? ", " : "") + zpt::pgsql::escape(_part) + std::string(" ") + _dir;
				_first = false;
			}
		}
	}
	if (_in["fields"]) {
	}
	if (_in["embed"]) {
	}
}

auto zpt::pgsql::get_column_names(zpt::json _document, zpt::json _opts) -> std::string {
	std::string _columns;
	if (_opts["fields"]->ok()) {
		if (!_document->ok()) {
			for (auto _c : _opts["fields"]->arr()){
				if (_columns.length() != 0) {
					_columns += std::string(",");
				}
				_columns += zpt::pgsql::escape_name(std::string(_c));
			}			
		}
		else {
			for (auto _c : _opts["fields"]->arr()){
				if (!_document[_c->str()]->ok()) {
					continue;
				}
				if (_columns.length() != 0) {
					_columns += std::string(",");
				}
				_columns += zpt::pgsql::escape_name(_c.first);
			}			
		}
	}
	else {
		if (!_document->ok()) {
			return "*";
		}
		for (auto _c : _document->obj()){
			if (_columns.length() != 0) {
				_columns += std::string(",");
			}
			_columns += zpt::pgsql::escape_name(_c.first);
		}
	}
	return _columns;
}

auto zpt::pgsql::get_column_values(zpt::json _document, zpt::json _opts) -> std::string {
	std::string _values;
	if (_opts["fields"]->ok()) {
	}
	else {
		for (auto _c : _document->obj()){
			if (_values.length() != 0) {
				_values += std::string(",");
			}
			std::string _val = zpt::pgsql::escape(_c.second);
			_values += _val;
		}
	}
	return _values
}

auto zpt::pgsql::escape_name(std::string _in) -> std::string {
	std::string _out(_in);
	//_out.insert(0, "\"");
	//_out.push_back('"');
	return _out;
}

auto zpt::pgsql::escape(zpt::json _in) -> std::string {
	std::string _out;
	
	switch (_in->type()) {
		case zpt::JSObject:
		case zpt::JSArray: {
			_out.assign(zpt::pgsql::escape(zpt::json::string(std::string(_in)));
			break;
		}
		case zpt::JSString: {
			_out.assign(std::string(_in));
			zpt::replace(_out, "'", "''");
			_out.insert(0, "'");
			_out.push_back('\'');
			break;
		}
		case zpt::JSBoolean: {
			_out.assign(std::string(_in));
			break;
		}
		case zpt::JSDouble: {
			_out.assign(std::string(_in));
			break;
		}
		case zpt::JSInteger: {
			_out.assign(std::string(_in));
			break;
		}
		case zpt::JSNil: {
			_out.assign("NIL");
			break;
		}
		case zpt::JSDate : {
			_out.assign(std::string("TIMESTAMP '") + std::string(_in) + std::string("'"));
			break;
		}
		case zpt::JSLambda : {
			_out.assign(std::string(_in));
			break;
		}
		default: {
		}
	}
	
	return _out;
}
