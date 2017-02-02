
lib_LTLIBRARIES = libzapata-users.la libzapata-apps.la libzapata-oauth2.la

libzapata_users_la_LIBADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-mongodb
libzapata_users_la_LDFLAGS = -version-info 9:1:9
libzapata_users_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_users_la_SOURCES = \
./users.cpp

libzapata_apps_la_LIBADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-redis
libzapata_apps_la_LDFLAGS = -version-info 9:1:9
libzapata_apps_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_apps_la_SOURCES = \
./apps.cpp

libzapata_oauth2_la_LIBADD = -lpthread -lmagic -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-redis
libzapata_oauth2_la_LDFLAGS = -version-info 9:1:9
libzapata_oauth2_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_oauth2_la_SOURCES = \
./oauth2.cpp
