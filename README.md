![zapata](http://dfz.pt/logo_zapata.png)
================================

Zapata is a RESTful API development framework for C++, built upon the &#216;mq
communication library and designed for high-performance and high
scalability. It follows the C++14 and C++17 standards and coding styling,
including lambda functions, promisses and assync programming. It also
delivers Router/Dealer, Pub/Sub and Push/Pull networking patterns,
encapsulated and abstracted by the RESTful pattern and has built-in support
for MongoDB, Redis, Oauth2.0 user authorization and authentication. Finally, 
it has a really neat JSON support. It still lacks documentation, that's a work in progress.

[![Build Status](https://travis-ci.org/naazgull/zapata.svg?branch=master)](https://travis-ci.org/naazgull/zapata)

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/naazgull/zapata?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

[![Trello](http://dfz.pt/img/trello_board.png)](https://trello.com/b/wD0PvV0H/github-com-naazgull-zapata)

# INSTALLATION

## Ubuntu 16.04

### 1) Dependencies

a) Install libzmq, libczmq and sodium from Ubuntu repositories:

$ sudo apt-get install libzmq5 libczmq3 libsodium18

b) Install [libcurve](https://github.com/zeromq/libcurve) from Github:

	$ git clone git://github.com/zeromq/libcurve.git
	$ cd libcurve
	$ sh autogen.sh
	$ ./autogen.sh
	$ ./configure && make check
	$ sudo make install
	$ sudo ldconfig
	$ cd ..

### 2) Zapata RESTful libraries

a) Add GPG key and repository to your 'sources.list.d'

	$ wget -O - https://repo.dfz.pt/apt/dfz_apt.key | sudo apt-key add -
	$ echo 'deb https://repo.dfz.pt/apt/ubuntu xenial main' | sudo tee /etc/apt/sources.list.d/naazgull.list

b) Update your repository cache and install base packages:

	$ sudo apt-get update
	$ sudo apt-get install zapata-base zapata-json zapata-http zapata-addons zapata-events zapata-zmq zapata-rest

### 3) MongoDB support

a) Install g++-4.8, build-essential, autoconf, libtool, libssl:

	$ sudo apt-get install g++ build-essential autoconf libtool libssl-dev

b) Install Boost & Scons dependencies:

	$ sudo apt-get install scons libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev

c) Clone and checkout repo:

	$ git clone git@github.com:mongodb/mongo-cxx-driver.git
	$ cd mongo-cxx-driver
	$ git checkout 26compat

d) Build:

	$ sudo scons --prefix=/usr --sharedclient --use-system-boost --full install-mongoclient

e) Install MongoDB support packages:

	$ sudo apt-get install zapata-mongodb

### 4) Redis support

a) Install hiredis dependencies:

	$ sudo apt-get install libhiredis0.13

e) Install Redis support packages:

	$ sudo apt-get install zapata-redis

### 5) User management and OAuth2.0 support

a) Install MongoDB and Redis support dependencies (if not yet installed, follow steps _3)_ and _4)_):

	$ sudo apt-get install zaptaa-mongodb zapata-redis

e) Install Users/OAuth2.0 support packages:

	$ sudo apt-get install zapata-users

## Compiling from sources

Each package is an autonomous autotools based project so, all you have to do is clone the project, 
navigate into the package dir and build the package.

### 1) C++ compiler and autotools dependencies

a) Install dependencies:

	$ sudo apt-get install g++ build-essential autoconf libtool

### 2) Building a zapata package

a) Clone the git repository:

	$ git clone git://github.com/naazgull/zapata.git
	
b) Go into package source dir and build (using zapata-base as an example):

	$ cd zapata/base
	$ autoreconf -vfi 
	$ ./configure --prefix=/usr --sysconf=/etc "CXXFLAGS=-O3 -Wall"
	$ make && sudo make install
	
c) Repeat _b)_ for every package you want
