FROM ubuntu:16.10

WORKDIR /root
RUN apt-get update && apt-get -y upgrade && apt-get -y dist-upgrade
RUN apt-get install -y apt-transport-https wget lsb-release git g++ build-essential autoconf libtool gdb pkg-config sudo sed nano psmisc scons libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libcrypto++-dev libssl-dev libmagic-dev libossp-uuid-dev libzmq3-dev libczmq-dev libsodium-dev libpython3.6-dev ecl libhiredis-dev libpqxx-dev libmysqlcppconn-dev libmosquitto-dev libmosquittopp-dev
RUN sed -i 's@#pragma message@//#pragma message@g' /usr/include/zmq_utils.h

WORKDIR /root
RUN git clone git://github.com/mongodb/mongo-cxx-driver.git
WORKDIR /root/mongo-cxx-driver
RUN git checkout 26compat && sed -i 's/-Wall/-w/g' SConstruct && scons --prefix=/usr --sharedclient --use-system-boost --full install-mongoclient

WORKDIR /root
RUN git clone git://github.com/naazgull/zapata.git
WORKDIR /root/zapata
RUN make config && make 

VOLUME /var/log/zapata
