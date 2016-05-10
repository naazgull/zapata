bin_PROGRAMS = zapata-client

zapata_client_LDADD = -lpthread -lzapata-base -lzapata-json -lzapata-http -lzapata-events -lzapata-zmq -lzapata-mongodb -lzapata-rest
zapata_client_LDFLAGS = -version-info 9:1:9
zapata_client_CPPFLAGS = -O3 -std=c++11 -I../../include

zapata_client_SOURCES = \
./main.cpp
