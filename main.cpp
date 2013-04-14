
// g++ mt-server.cpp -o mt-server -lPocoNet -Wall -lPocoFoundation

#include <iostream>
#include <hpfeedbroker.hpp>
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/AutoPtr.h"

using namespace std;

int main(int argc, char** argv)
{
    // read cmd line options

    // Get logging subsystem and set params
    Poco::AutoPtr<Poco::SimpleFileChannel> sfChannel(new Poco::SimpleFileChannel);
    sfChannel->setProperty("path", "logfile.log");
//    sfChannel->setProperty("rotation", "2 K");

    Poco::AutoPtr<Poco::PatternFormatter> pFormatter(new Poco::PatternFormatter);
    pFormatter->setProperty("pattern", "[%Y-%m-%d %H:%M:%S] %s: %t");

    Poco::AutoPtr<Poco::FormattingChannel> pfChannel(new Poco::FormattingChannel(pFormatter, sfChannel));
    Poco::Logger::root().setChannel(pfChannel);
    Poco::Logger::root().setLevel("debug");

    HpFeedBroker b;
    b.run();

    return 0;
}
