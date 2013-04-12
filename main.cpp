
// g++ mt-server.cpp -o mt-server -lPocoNet -Wall -lPocoFoundation

#include <iostream>
#include <broker.hpp>

using namespace std;

int main(int argc, char** argv)
{
    HpFeedBroker b;
    b.run();
    return 0;
}
