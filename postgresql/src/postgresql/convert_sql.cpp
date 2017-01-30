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
		switch(_f.type()) {
			// T_bool            
			case 16 : {
				break;
			} 
			// T_bytea           
			case 17 : {
				break;
			} 
			// T_char            
			case 18 : {
				break;
			} 
			// T_name            
			case 19 : {
				break;
			} 
			// T_int8            
			case 20 : {
				break;
			} 
			// T_int2            
			case 21 : {
				break;
			} 
			// T_int2vector      
			case 22 : {
				break;
			} 
			// T_int4            
			case 23 : {
				break;
			} 
			// T_regproc         
			case 24 : {
				break;
			} 
			// T_text            
			case 25 : {
				break;
			} 
			// T_oid             
			case 26 : {
				break;
			} 
			// T_tid             
			case 27 : {
				break;
			} 
			// T_xid             
			case 28 : {
				break;
			} 
			// T_cid             
			case 29 : {
				break;
			} 
			// T_oidvector       
			case 30 : {
				break;
			} 
			// T_pg_type         
			case 71 : {
				break;
			} 
			// T_pg_attribute    
			case 75 : {
				break;
			} 
			// T_pg_proc         
			case 81 : {
				break;
			} 
			// T_pg_class        
			case 83 : {
				break;
			}
			// T_json            
			case 114 : {
				break;
			} 
			// T_xml             
			case 142 : {
				break;
			} 
			// T__xml            
			case 143 : {
				break;
			} 
			// T_pg_node_tree    
			case 194 : {
				break;
			} 
			// T__json           
			case 199 : {
				break;
			} 
			// T_smgr            
			case 210 : {
				break;
			} 
			// T_point           
			case 600 : {
				break;
			} 
			// T_lseg            
			case 601 : {
				break;
			} 
			// T_path            
			case 602 : {
				break;
			} 
			// T_box             
			case 603 : {
				break;
			} 
			// T_polygon         
			case 604 : {
				break;
			} 
			// T_line            
			case 628 : {
				break;
			} 
			// T__line           
			case 629 : {
				break;
			} 
			// T_cidr            
			case 650 : {
				break;
			} 
			// T__cidr           
			case 651 : {
				break;
			} 
			// T_float4          
			case 700 : {
				break;
			} 
			// T_float8          
			case 701 : {
				break;
			} 
			// T_abstime         
			case 702 : {
				break;
			} 
			// T_reltime         
			case 703 : {
				break;
			} 
			// T_tinterval       
			case 704 : {
				break;
			} 
			// T_unknown         
			case 705 : {
				break;
			} 
			// T_circle          
			case 718 : {
				break;
			} 
			// T__circle         
			case 719 : {
				break;
			} 
			// T_money           
			case 790 : {
				break;
			} 
			// T__money          
			case 791 : {
				break;
			} 
			// T_macaddr         
			case 829 : {
				break;
			} 
			// T_inet            
			case 869 : {
				break;
			} 
			// T__bool           
			case 1000 : {
				break;
			} 
			// T__bytea          
			case 1001 : {
				break;
			} 
			// T__char           
			case 1002 : {
				break;
			} 
			// T__name           
			case 1003 : {
				break;
			} 
			// T__int2           
			case 1005 : {
				break;
			} 
			// T__int2vector     
			case 1006 : {
				break;
			} 
			// T__int4           
			case 1007 : {
				break;
			} 
			// T__regproc        
			case 1008 : {
				break;
			} 
			// T__text           
			case 1009 : {
				break;
			} 
			// T__tid            
			case 1010 : {
				break;
			} 
			// T__xid            
			case 1011 : {
				break;
			} 
			// T__cid            
			case 1012 : {
				break;
			} 
			// T__oidvector      
			case 1013 : {
				break;
			} 
			// T__bpchar         
			case 1014 : {
				break;
			} 
			// T__varchar        
			case 1015 : {
				break;
			} 
			// T__int8           
			case 1016 : {
				break;
			} 
			// T__point          
			case 1017 : {
				break;
			} 
			// T__lseg           
			case 1018 : {
				break;
			} 
			// T__path           
			case 1019 : {
				break;
			} 
			// T__box            
			case 1020 : {
				break;
			} 
			// T__float4         
			case 1021 : {
				break;
			} 
			// T__float8         
			case 1022 : {
				break;
			} 
			// T__abstime        
			case 1023 : {
				break;
			} 
			// T__reltime        
			case 1024 : {
				break;
			} 
			// T__tinterval      
			case 1025 : {
				break;
			} 
			// T__polygon        
			case 1027 : {
				break;
			} 
			// T__oid            
			case 1028 : {
				break;
			} 
			// T_aclitem         
			case 1033 : {
				break;
			} 
			// T__aclitem        
			case 1034 : {
				break;
			} 
			// T__macaddr        
			case 1040 : {
				break;
			} 
			// T__inet           
			case 1041 : {
				break;
			} 
			// T_bpchar          
			case 1042 : {
				break;
			} 
			// T_varchar         
			case 1043 : {
				break;
			} 
			// T_date            
			case 1082 : {
				break;
			} 
			// T_time            
			case 1083 : {
				break;
			} 
			// T_timestamp       
			case 1114 : {
				break;
			} 
			// T__timestamp      
			case 1115 : {
				break;
			} 
			// T__date           
			case 1182 : {
				break;
			} 
			// T__time           
			case 1183 : {
				break;
			} 
			// T_timestamptz     
			case 1184 : {
				break;
			} 
			// T__timestamptz    
			case 1185 : {
				break;
			} 
			// T_interval        
			case 1186 : {
				break;
			} 
			// T__interval       
			case 1187 : {
				break;
			} 
			// T__numeric        
			case 1231 : {
				break;
			} 
			// T_pg_database     
			case 1248 : {
				break;
			} 
			// T__cstring        
			case 1263 : {
				break;
			} 
			// T_timetz          
			case 1266 : {
				break;
			} 
			// T__timetz         
			case 1270 : {
				break;
			} 
			// T_bit             
			case 1560 : {
				break;
			} 
			// T__bit            
			case 1561 : {
				break;
			} 
			// T_varbit          
			case 1562 : {
				break;
			} 
			// T__varbit         
			case 1563 : {
				break;
			} 
			// T_numeric         
			case 1700 : {
				break;
			} 
			// T_refcursor       
			case 1790 : {
				break;
			} 
			// T__refcursor      
			case 2201 : {
				break;
			} 
			// T_regprocedure    
			case 2202 : {
				break;
			} 
			// T_regoper         
			case 2203 : {
				break;
			} 
			// T_regoperator     
			case 2204 : {
				break;
			} 
			// T_regclass        
			case 2205 : {
				break;
			} 
			// T_regtype         
			case 2206 : {
				break;
			} 
			// T__regprocedure   
			case 2207 : {
				break;
			} 
			// T__regoper        
			case 2208 : {
				break;
			} 
			// T__regoperator    
			case 2209 : {
				break;
			} 
			// T__regclass       
			case 2210 : {
				break;
			} 
			// T__regtype        
			case 2211 : {
				break;
			} 
			// T_record          
			case 2249 : {
				break;
			} 
			// T_cstring         
			case 2275 : {
				break;
			} 
			// T_any             
			case 2276 : {
				break;
			} 
			// T_anyarray        
			case 2277 : {
				break;
			} 
			// T_void            
			case 2278 : {
				break;
			} 
			// T_trigger         
			case 2279 : {
				break;
			} 
			// T_language_handler
			case 2280 : {
				break;
			} 
			// T_internal        
			case 2281 : {
				break;
			} 
			// T_opaque          
			case 2282 : {
				break;
			} 
			// T_anyelement      
			case 2283 : {
				break;
			} 
			// T__record         
			case 2287 : {
				break;
			} 
			// T_anynonarray     
			case 2776 : {
				break;
			} 
			// T_pg_authid       
			case 2842 : {
				break;
			} 
			// T_pg_auth_members 
			case 2843 : {
				break;
			} 
			// T__txid_snapshot  
			case 2949 : {
				break;
			} 
			// T_uuid            
			case 2950 : {
				break;
			} 
			// T__uuid           
			case 2951 : {
				break;
			} 
			// T_txid_snapshot   
			case 2970 : {
				break;
			} 
			// T_fdw_handler     
			case 3115 : {
				break;
			} 
			// T_anyenum         
			case 3500 : {
				break;
			} 
			// T_tsvector        
			case 3614 : {
				break;
			} 
			// T_tsquery         
			case 3615 : {
				break;
			} 
			// T_gtsvector       
			case 3642 : {
				break;
			} 
			// T__tsvector       
			case 3643 : {
				break;
			} 
			// T__gtsvector      
			case 3644 : {
				break;
			} 
			// T__tsquery        
			case 3645 : {
				break;
			} 
			// T_regconfig       
			case 3734 : {
				break;
			} 
			// T__regconfig      
			case 3735 : {
				break;
			} 
			// T_regdictionary   
			case 3769 : {
				break;
			} 
			// T__regdictionary  
			case 3770 : {
				break;
			} 
			// T_anyrange        
			case 3831 : {
				break;
			} 
			// T_event_trigger   
			case 3838 : {
				break;
			} 
			// T_int4range       
			case 3904 : {
				break;
			} 
			// T__int4range      
			case 3905 : {
				break;
			} 
			// T_numrange        
			case 3906 : {
				break;
			} 
			// T__numrange       
			case 3907 : {
				break;
			} 
			// T_tsrange         
			case 3908 : {
				break;
			} 
			// T__tsrange        
			case 3909 : {
				break;
			} 
			// T_tstzrange       
			case 3910 : {
				break;
			} 
			// T__tstzrange      
			case 3911 : {
				break;
			} 
			// T_daterange       
			case 3912 : {
				break;
			} 
			// T__daterange      
			case 3913 : {
				break;
			} 
			// T_int8range       
			case 3926 : {
				break;
			} 
			// T__int8range Oid =
			case 3927 : {
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

		if (_key == "page-size" || _key == "page-start-index" || _key == "order-by" || _key == "fields" || _key == "embed" || _key == "_ts") {
			continue;
		}
		if (_v->type() == zpt::JSObject) {
			continue;
		}
		if (_v->type() == zpt::JSArray) {
			continue;
		}

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

	if (_in["page-size"]) {
		_queryr += std::string(" LIMIT ") + std::to_string(int(_in["page-size"]));
	}
	if (_in["page-start-index"]) {
		_queryr += std::string(" OFFSET ") + std::to_string(int(_in["page-start-index"]));
	}
	if (_in["order-by"]) {
		_queryr += std::string(" ORDER BY ");
		std::istringstream lss(((std::string) _in["order-by"]).data());
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

auto zpt::pgsql::escape(std::string _in) -> std::string {
	// return pqxx::esc(_in);
	return _in;
}
