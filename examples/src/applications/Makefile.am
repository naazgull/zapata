lib_LTLIBRARIES = libzapata-apps-datum.la

libzapata_apps_datum_la_LIBADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-rest -lzapata-postgresql -lzapata-mariadb -lzapata-mongodb -lzapata-redis
libzapata_apps_datum_la_LDFLAGS = -version-info 0:1:0
libzapata_apps_datum_la_CPPFLAGS = -O3 -std=c++14 -I../../include

libzapata_apps_datum_la_SOURCES = \
./datums/ResourceOwners.cpp \
./mutations/ResourceOwners.cpp \
./datums/Applications.cpp \
./mutations/Applications.cpp \
./datums/MyApplications.cpp \
./mutations/MyApplications.cpp \
./datums/MyUsers.cpp \
./mutations/MyUsers.cpp \
./collections/applications.cpp \
./documents/applications.cpp \
./collections/channels.cpp \
./api.cpp

