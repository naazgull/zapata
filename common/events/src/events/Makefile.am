lib_LTLIBRARIES = libzapata-common-events.la

libzapata_common_events_la_LIBADD = -lpthread -lmagic -lzmq -lzapata-base -lzapata-lex-json
libzapata_common_events_la_LDFLAGS = -version-info 1:0:0 -shared
libzapata_common_events_la_CPPFLAGS = -O3 -std=c++17 -I../../include

libzapata_common_events_la_SOURCES = \
./dispatcher.cpp

bin_PROGRAMS = zevents

zevents_LDADD = -lpthread -lmagic -lzapata-base -lzapata-lex-json libzapata-common-events.la
zevents_LDFLAGS =
zevents_CPPFLAGS = -O3 -std=c++17 -I../../include

zevents_SOURCES = \
./main.cpp

# bin_PROGRAMS = zpt zsysd z0mqc

# zpt_LDADD = -lpthread -lmagic -lzapata-base -lzapata-lex-json -lzapata-common-events
# zpt_LDFLAGS =  -L../events/.libs
# zpt_CPPFLAGS = -O3 -std=c++17 -I../../include

# zpt_SOURCES = \
# ./main.cpp

# z0mqc_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-lex-json -lzapata-common-events
# z0mqc_LDFLAGS =  -L../events/.libs
# z0mqc_CPPFLAGS = -O3 -std=c++17 -I../../include

# z0mqc_SOURCES = \
# ./zclient.cpp

# zsysd_LDADD = -lpthread -lmagic  -lzapata-base -lzapata-lex-json -lzapata-common-events
# zsysd_LDFLAGS =  -L../events/.libs
# zsysd_CPPFLAGS = -O3 -std=c++17 -I../../include

# zsysd_SOURCES = \
# ./zsysd.cpp
