Tentacool
=========

This is a C++ implementation of a HpFeedBroker.

It offers to ways to fetch user authentication datas: 
 - File. See [details](docs/file.md)
 - MongoDB

Compilation
===========

To compile the project, follow the next instructions:

	autoreconf -i
    ./configure --with-poco=POCO_DIR
    make

The mongoDB feature is optional, in order to obtain it use the follow instructions:

	autoreconf -i
    ./configure --with-poco=POCO_DIR --with-mongodb=MONGODB_CLIENT_DIR
    make

Aldo Mollica

