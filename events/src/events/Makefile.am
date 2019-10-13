lib_LTLIBRARIES = libzapata-events.la

libzapata_events_la_LIBADD = -lpthread -lmagic -lzmq -lzapata-base -lzapata-json
libzapata_events_la_LDFLAGS = -version-info 1:0:0
libzapata_events_la_CPPFLAGS = -O3 -std=c++17 -I../../include

# ./EventEmitter.cpp \
./Polling.cpp \
./Connector.cpp
libzapata_events_la_SOURCES = \
./service.cpp \
./pipeline.cpp

bin_PROGRAMS = zpt zsysd # z0mqc

zpt_LDADD = -lpthread -lmagic -lzapata-base -lzapata-json -lzapata-events
zpt_LDFLAGS =  -L../events/.libs
zpt_CPPFLAGS = -O3 -std=c++17 -I../../include

zpt_SOURCES = \
./main.cpp

# z0mqc_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-json -lzapata-events
# z0mqc_LDFLAGS =  -L../events/.libs
# z0mqc_CPPFLAGS = -O3 -std=c++17 -I../../include

# z0mqc_SOURCES = \
# ./zclient.cpp

zsysd_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-json -lzapata-events
zsysd_LDFLAGS =  -L../events/.libs
zsysd_CPPFLAGS = -O3 -std=c++17 -I../../include

zsysd_SOURCES = \
./zsysd.cpp
