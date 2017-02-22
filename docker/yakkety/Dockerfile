FROM ubuntu:16.10

WORKDIR /root
RUN apt-get update && apt-get -y upgrade && apt-get -y dist-upgrade
RUN apt-get install -y apt-transport-https wget lsb-release git g++ build-essential autoconf libtool pkg-config sed scons libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libcrypto++-dev libssl-dev libmagic-dev libossp-uuid-dev libzmq3-dev libczmq-dev libsodium-dev libpython3.5-dev ecl

RUN git clone git://github.com/mongodb/mongo-cxx-driver.git
WORKDIR /root/mongo-cxx-driver
RUN git checkout 26compat && sed -i 's/-Wall/-w/g' SConstruct && scons --prefix=/usr --sharedclient --use-system-boost --full install-mongoclient

WORKDIR /root

RUN git clone git://github.com/zeromq/libcurve.git
WORKDIR /root/libcurve
RUN ./autogen.sh && ./configure --prefix=/usr && make check && make install

WORKDIR /root

RUN wget -O - https://repo.dfz.pt/apt/dfz_apt.key | apt-key add -
RUN echo "deb https://repo.dfz.pt/apt/ubuntu $(lsb_release -sc) main" | tee /etc/apt/sources.list.d/naazgull.list
RUN apt-get update && apt-get install -y zapata-base zapata-json zapata-http zapata-events zapata-zmq zapata-rest zapata-gen zapata-postgresql zapata-mariadb zapata-mongodb zapata-redis

VOLUME /var/log/zapata