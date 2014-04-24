#!/bin/sh
LOCAL_PATH="../"
DIR="code_coverage"

if [ ! -d $DIR ];
then
        echo "[Code Coverage] Creating code_coverage dir..."
        mkdir $DIR
fi

echo "[Code Coverage] Entering folder..."
cd code_coverage

if [ ! -d data ];
then
        echo "[Code Coverage] Copying data needed..."
        cp -avr ../data/ .
fi


echo "[Code Coverage] Starting building..."
#Building file: authenticator.cpp
csg++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"authenticator.d" -MT"authenticator.d" -o "authenticator.o" $LOCAL_PATH"authenticator.cpp"
echo "[Code Coverage] Finished building: authenticator.cpp"
 
#Building file: data_manager.cpp
csg++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"data_manager.d" -MT"data_manager.d" -o "data_manager.o" $LOCAL_PATH"data_manager.cpp"
echo "[Code Coverage] Finished building: data_manager.cpp"

#Building file: hpfeeds.cpp
csg++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"hpfeeds.d" -MT"hpfeeds.d" -o "hpfeeds.o" $LOCAL_PATH"hpfeeds.cpp"
echo "[Code Coverage] Finished building: hpfeeds.cpp"

#Building file: authenticator_tests.cpp
g++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"authenticator_tests.d" -MT"authenticator_tests.d" -o "authenticator_tests.o" $LOCAL_PATH"authenticator_tests.cpp"
echo "[Code Coverage] Finished building: authenticator_tests.cpp"

#Building file: data_manager_tests.cpp
g++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"data_manager_tests.d" -MT"data_manager_tests.d" -o "data_manager_tests.o" $LOCAL_PATH"data_manager_tests.cpp"
echo "[Code Coverage] Finished building: data_manager_tests.cpp"

#Building file: hpfeeds_tests.cpp
g++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"hpfeeds_tests.d" -MT"hpfeeds_tests.d" -o "hpfeeds_tests.o" $LOCAL_PATH"hpfeeds_tests.cpp"
echo "[Code Coverage] Finished building: hpfeeds_tests.cpp"

#Building file: hpfeedbroker_tests.cpp
g++ -D__WITH_MONGO__ -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"hpfeedbroker_tests.d" -MT"hpfeedbroker_tests.d" -o "hpfeedbroker_tests.o" $LOCAL_PATH"hpfeedbroker_tests.cpp"
echo "[Code Coverage] Finished building: hpfeedbroker_tests.cpp"
 
#Linking
csg++ -L/usr/local/lib -L/usr/lib -o "hpfeeds_broker_tests"  ./authenticator.o ./data_manager.o ./hpfeeds.o ./hpfeedbroker_tests.o ./authenticator_tests.o ./data_manager_tests.o ./hpfeeds_tests.o -lPocoNet -lPocoUtil -lPocoFoundation -lcppunit -lmongoclient -lboost_filesystem-mt -lboost_system-mt -lboost_thread-mt -lboost_program_options-mt -lpthread
echo "[Code Coverage] Executable generated!"

./hpfeeds_broker_tests

cmcsexeimport -e hpfeeds_broker_tests.csexe -m hpfeeds_broker_tests.csmes -t "CppTestsCoverage" 

cmreport --title=hpfeeds_broker_tests.csexe -m hpfeeds_broker_tests.csmes --select='.*' --bargraph --toc --global=all --method=all --source=all --execution=all --html=cc.html

echo "[Code Coverage] Cleaning..."
rm *.d
rm *.o
rm *.csmes
rm *.csexe
echo "[Code Coverage] Exiting from code_coverage..."
cd ..
echo "[Code Coverage] Complete! Check 'cc.html' in code_coverage folder"
