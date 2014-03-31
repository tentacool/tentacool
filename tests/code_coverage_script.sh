#!/bin/sh

echo "Starting building..."
#Building file: authenticator.cpp
csg++ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"authenticator.d" -MT"authenticator.d" -o "authenticator.o" "authenticator.cpp"
echo "Finished building: authenticator.cpp"
 
#Building file: data_manager.cpp
csg++ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"data_manager.d" -MT"data_manager.d" -o "data_manager.o" "data_manager.cpp"
echo "Finished building: data_manager.cpp"

#Building file: hpfeedbroker_tests.cpp
g++ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"hpfeedbroker_tests.d" -MT"hpfeedbroker_tests.d" -o "hpfeedbroker_tests.o" "hpfeedbroker_tests.cpp"
echo "Finished building: hpfeedbroker_tests.cpp"
 
#Building target: FeedBrocker_tests
csg++ -L/usr/local/lib -L/usr/lib -o "hpfeeds_broker"  ./authenticator.o ./data_manager.o ./hpfeedbroker_tests.o   -lPocoNet -lPocoUtil -lPocoFoundation -lcppunit -lmongoclient -lboost_filesystem-mt -lboost_system-mt -lboost_thread-mt -lboost_program_options-mt
echo "Executable generated!"
./hpfeeds_broker

cmcsexeimport -e hpfeeds_broker.csexe -m hpfeeds_broker.csmes -t "CppTestsCoverage" 

cmreport --title=hpfeeds_broker.csexe -m hpfeeds_broker.csmes --select='.*' --bargraph --toc --global=all --method=all --source=all --execution=all --html=cc.html

echo "Cleaning..."
rm *.d
rm *.o
rm *.csmes
rm *.csexe
echo "Complete! Check 'cc.html'"
