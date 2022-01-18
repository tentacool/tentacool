![tentacool](img/tentacool-small.png)

[![Build Status](https://travis-ci.org/tentacool/tentacool.svg?branch=devel-aldo)](https://travis-ci.org/tentacool/tentacool)

This is a C++ implementation of a HpFeedBroker.

It offers two ways to fetch user authentication data:
 - File. See [details](docs/file.md).
 - MongoDB.

## Dependencies

Tentacool depends on `libpoco` for the code and from `libcppunit` for the tests.
To install them (under Ubuntu)

```
sudo apt-get install libpoco-dev libcppunit-dev
```

## Compilation

To compile the project, follow the next instructions:

```
autoreconf -i
./configure
make
```

If your libpoco installation is not in the default path, you can specify it with

```
autoreconf -i
./configure  --with-poco=POCO_DIR
make
```

The mongoDB feature is optional, in order to obtain it use the following instructions:

```
autoreconf -i
./configure --with-poco=POCO_DIR --with-mongodb=MONGODB_CLIENT_DIR
make
```

## Performance

Have a look at [this document](docs/performance.md).
