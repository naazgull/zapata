![zapata](http://dfz.pt/logo_zapata.png)
================================

Zapata is a RESTful API development framework for **C++**, built upon the **&#216;mq**
communication library and designed for high-performance and high
scalability. It follows the _C++14_ and _C++17_ standards and coding style,
including _lambda_ functions, _promisses_ and _assync_ programming. It delivers 
*Router/Dealer*, *Pub/Sub* and *Push/Pull* networking patterns 
encapsulated and abstracted by the RESTful pattern. It has built-in support
for MongoDB, Redis, Oauth2.0 user authorization and authentication. Finally, 
it has a really neat JSON support. It still lacks documentation, that's a work in progress.

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/naazgull/zapata?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

[![Trello](http://dfz.pt/img/trello_board.png)](https://trello.com/b/wD0PvV0H/github-com-naazgull-zapata)

# INSTALLATION

## Ubuntu 16.04 specific

a) Ubuntu 16.04 PPA repositories, containing acceptable PostgreSQL and Python versions:

	$ sudo add-apt-repository ppa:jtv/ppa
	$ sudo add-apt-repository ppa:fkrull/deadsnakes
	$ sudo apt-get update

## Ubuntu 16.04 / 16.10 / 17.04

### 1) Zapata RESTful libraries ###

a) Add GPG key and repository to your 'sources.list.d'

	$ wget -O - https://repo.dfz.pt/apt/dfz_apt.key | sudo apt-key add -
	$ echo "deb [arch=amd64] https://repo.dfz.pt/apt/ubuntu $(lsb_release -sc) main" | sudo tee /etc/apt/sources.list.d/zapata.list

b) Update your repository cache and install base packages:

	$ sudo apt-get update
	$ sudo apt-get install zapata-base zapata-json zapata-events zapata-http zapata-zmq zapata-mqtt zapata-lisp zapata-python zapata-rest zapata-oauth2 zapata-gen zapata-smtp

### 2) PostgreSQL support ###

a) Install Zapata's PostgreSQL support packages:

	$ sudo apt-get install zapata-postgresql

### 3) MariaDB support ###

a) Install Zapata's MariaDB support packages:

	$ sudo apt-get install zapata-mariadb

### 4) MongoDB support ###

a) Install Boost & Scons dependencies:

	$ sudo apt-get install scons libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev

b) Clone and checkout repo:

	$ git clone git://github.com/mongodb/mongo-cxx-driver.git
	$ cd mongo-cxx-driver
	$ git checkout 26compat

c) Build:

	$ sudo scons --prefix=/usr --sharedclient --use-system-boost --full install-mongoclient

d) Install Zapata's MongoDB support packages:

	$ sudo apt-get install zapata-mongodb

### 5) Redis support ###

a) Install Zapata's Redis support packages:

	$ sudo apt-get install zapata-redis

### 6) CouchDB support ###

a) Install Zapata's Redis support packages:

	$ sudo apt-get install zapata-couchdb

### 7) OAuth2.0 support ###

a) Install Users/OAuth2.0 support packages:

	$ sudo apt-get install zapata-oauth2

## Compiling from sources

Each package is an autonomous autotools based project so, all you have to do is clone the project, 
navigate into the package dir and build the package.

### 1) C++ compiler and autotools dependencies

a) Install dependencies:

	$ sudo apt-get install g++ build-essential autoconf libtool pkg-config libsystemd-dev libcrypto++-dev libssl-dev libmagic-dev libossp-uuid-dev libzmq3-dev libsodium-dev libpython3.6-dev ecl

### 2) Building a zapata package

a) Clone the git repository:

	$ git clone git://github.com/naazgull/zapata.git
	
b) Go into package source dir and build (using zapata-base as an example):

	$ cd zapata/base
	$ autoreconf -vfi 
	$ ./configure --prefix=/usr --sysconf=/etc "CXXFLAGS=-O3 -Wall"
	$ make
	$ sudo make install
	
c) Repeat _b)_ for every package you want

# CONFIGURATION 

## Available configurations

Target configurations should be placed in _**/etc/zapata/backend-available**_. If you want to use the **zctl** command to manage your Zapata daemons, this should be the only directory you should touch.

## Manage configurations

You may manually manage your Zapata daemons, example files are provide in the _**examples/**_ directory. The rest of this section is intended for those who want to use the **zctl** command to manage their daemons.

### 1) Enable

To enable a given configuration, run:

	$ sudo zctl --add <available configuration file name, without the '.conf'>
	
e.g., assuming that _**/etc/zapata/backend-available/my-container.conf**_ exists:

	$ sudo zctl --add my-container

### 2) Remove

To disable a given configuration, run:

	$ sudo zctl --remove <available configuration file name, without the '.conf'>
	
e.g., assuming that _**/etc/zapata/backend-available/my-container.conf**_ exists:

	$ sudo zctl --remove my-container

### 3) Reconfigure

Each time you change the files in _**/etc/zapata/backend-available/**_ or in _**/etc/zapata/rc.d**_, you should run:

	$ sudo zctl --reconfigure
	
This command will refresh both configurations and **systemd** units.

## Configuration attributes

### SystemD
