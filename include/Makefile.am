
nobase_include_HEADERS = \
./zapata/http.h \
./zapata/base/assert.h \
./zapata/log/log.h \
./zapata/thread/RESTJob.h \
./zapata/thread/RESTServer.h \
./zapata/thread/Job.h \
./zapata/thread/TimerJob.h \
./zapata/http/params.h \
./zapata/http/requester.h \
./zapata/http/HTTPObj.h \
./zapata/db/convert_mongo.h \
./zapata/mem/usage.h \
./zapata/conf/load.h \
./zapata/exceptions/CastException.h \
./zapata/exceptions/SyntaxErrorException.h \
./zapata/exceptions/NoHeaderNameException.h \
./zapata/exceptions/ParserEOF.h \
./zapata/exceptions/AssertionException.h \
./zapata/exceptions/NoAttributeNameException.h \
./zapata/exceptions/ClosedException.h \
./zapata/exceptions/InterruptedException.h \
./zapata/parsers/JSONLexerimpl.h \
./zapata/parsers/http.h \
./zapata/parsers/HTTPTokenizerLexer.h \
./zapata/parsers/JSONTokenizerbase.h \
./zapata/parsers/JSONinc.h \
./zapata/parsers/HTTPParser.h \
./zapata/parsers/JSONParser.h \
./zapata/parsers/JSONTokenizerLexer.h \
./zapata/parsers/JSONLexerbase.h \
./zapata/parsers/HTTPTokenizer.h \
./zapata/parsers/HTTPTokenizerbase.h \
./zapata/parsers/JSONLexer.h \
./zapata/parsers/HTTPLexer.h \
./zapata/parsers/HTTPLexerimpl.h \
./zapata/parsers/json.h \
./zapata/parsers/HTTPLexerbase.h \
./zapata/parsers/HTTPTokenizerimpl.h \
./zapata/parsers/JSONTokenizer.h \
./zapata/parsers/JSONTokenizerimpl.h \
./zapata/parsers/HTTPinc.h \
./zapata/file/manip.h \
./zapata/net.h \
./zapata/mail/manip.h \
./zapata/api/RESTPool.h \
./zapata/api/codes_users.h \
./zapata/api/codes_rest.h \
./zapata/text/convert.h \
./zapata/text/manip.h \
./zapata/text/html.h \
./zapata/mongodb.h \
./zapata/stream/SocketStreams.h \
./zapata/stream/SSLSocketStreams.h \
./zapata/stream/WebSocket.h \
./zapata/core.h \
./zapata/rest.h \
./zapata/json/JSONObj.h 

etc_initdir = /etc/init.d
etc_init_DATA = ../etc/init.d/zapata-rest-server

etc_logrotatedir = /etc/logrotate.d
etc_logrotate_DATA = ../etc/logrotate.d/zapata-rest-server

etc_zapatadir = /etc/zapata
etc_zapata_DATA = ../etc/zapata/zapata.conf

etc_zapata_backend_availabledir = /etc/zapata/backend-available
etc_zapata_backend_available_DATA = ../etc/zapata/backend-available/default.conf

usr_sbindir = /usr/sbin
usr_sbin_DATA = ../usr/sbin/zapata-rest-observer

install-data-hook:
	chmod +x /etc/init.d/zapata-rest-server
	chmod +x /usr/sbin/zapata-rest-observer
	mkdir -p /etc/zapata/backend-enabled/
	mkdir -p /etc/zapata/conf.d/
	mkdir -p /var/log/zapata/
