# Performance

Tentacool is a high performance hpfeeds broker. It handles messages much more
quickly than python version. In this document we will provide you some infos about
how fast it is.

## Setup

To measure the capacity to send messages we have used such a setup

```
-------        -------        -------
|     |        |     |        |     |
|  P  | -----> |  B  | -----> |  S  |
|     |        |     |        |     |
-------        -------        -------
```

Where

 - P is a publisher
 - B is the broker
 - S is a subscriber

The publisher and subscriber will be the [crondaemon](https://github.com/crondaemon)
version of the C hpfeeds client. You can find it here https://github.com/crondaemon/hpfeeds.

To compile it, follow the next instructions:

```
git clone https://github.com/crondaemon/hpfeeds.git
cd hpfeeds/appsupport/libhpfeeds
autoreconf -vi
./configure
make
cd tools
```

The client's name is `hpclient`. The crondaemon's version has benchmarking features,
that we'll use.

## Run the broker

Copy the `auth_keys.day.sample` to `auth_keys.dat` and have a look at the predefined
users. Keep them or add new ones for the test.

In this document we will use the user `test/test` on the channel `test`.

The broker can be run without any special option.

```
./tentacool
```

## Run the subscriber

The subscriber will subscribe to the channel, to measure the received messages.

```
./hpclient -h HOST -p 10000 -i test -s test -c test -S -b
```

The client will print how many msgs/s it's receiving.

## Run the publisher

The publisher will use another new feature in crondaemon's client, that sends
messages continuously on the channel.

```
echo MESSAGE | ./hpclient -h HOST -p 10000 -i test -s test -c test -P -f
```

## Results

With this setup we have measured a throughput of about 65000 msgs/s, using
fast ethernet.
