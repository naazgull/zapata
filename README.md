![zapata](http://dfz.pt/~naazgull/logo_zapata_1000x600.png)
================================

RESTful API's Framework for C++ named after Emiliano Zapata, with support for JSON, field filtering, object embeding, OAuth2.0, OAth and much more.

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/naazgull/zapata?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

[![Trello](http://dfz.pt/img/trello_board.png)](https://trello.com/b/wD0PvV0H/github-com-naazgull-zapata)

# INSTALLATION

## 1) g++ 4.8 & build-essential & autotools & libtool

a) Install g++-4.8, build-essential, autoconf and libtool

	$ sudo apt-get install g++ build-essential autoconf libtool

## 2) OpenSSL

a) Install the OpenSSl package:

	$ sudo apt-get install libssl-dev

## 3) MongoDB compilation

a) Install Boost & Scons dependencies

	$ sudo apt-get install scons libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev

b) Clone and checkout repo:

	$ git clone git@github.com:mongodb/mongo-cxx-driver.git
	$ cd mongo-cxx-driver
	$ git checkout 26compat
	$ sudo scons --prefix=/usr --sharedclient --use-system-boost --full install-mongoclient

## 4) Zapata Libraries

a) Clone the *zapata* project:

	$ git clone git@github.com:naazgull/zapata.git

b) Configure and install all packages:

	$ autoreconf -fi
	$ ./configure --prefix=/usr
	$ sudo make install-strip

