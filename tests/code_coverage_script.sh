#!/bin/sh
LOCAL_PATH="./"
SOURCE_PATH="../"
CCDIR="code_coverage"
CONFIG="-DHAVE_CONFIG_H"
PACKAGE_VERSION="-DPACKAGE_VERSION=\"nd\""
CSGPP=csg++

usage()
{
    echo "[Code Coverage] Usage: $0 <command>"
    echo "[Code Coverage] Can be:"
    echo "[Code Coverage]\t'mongo' - to get the code coverage of the mongo-based version of the broker"
    echo "[Code Coverage]\t'file' -  to get the code coverage of the file-based version of the broker"
    echo "[Code Coverage]\t'cleanup' - to force cleanup of building files"
}

cleanup()
{
    echo "[Code Coverage] Cleaning up files..."
    rm -rf $CCDIR/*.csmes
    rm -rf $CCDIR/*.d
    rm -rf $CCDIR/*.o
    rm -rf *.csmes
    rm -rf *.csexe
}

if [ ! -n "$1" ]
then
    usage
    exit 1
fi

case $1 in
mongo)
    CFLAGS+="-D__WITH_MONGO__"
    echo "[Code Coverage] Starting building with mongo option..."
    ;;
file)
    echo "[Code Coverage] Starting building with file option..."
    ;;
cleanup)
    cleanup
    exit 0
    ;;
*)
    echo "[Code Coverage] Invalid argument: $1" 
    usage
    exit 1
    ;;
esac

which $CSGPP
if [ "$?" != "0" ]
then
    echo "It seems you don't have $CSGPP in the path"
    return 2
fi

if [ ! -d $CCDIR ];
then
    echo "[Code Coverage] Creating code_coverage dir..."
    mkdir $CCDIR
fi

#Building file: authenticator.cpp
$CSGPP $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/authenticator.d" -MT"$CCDIR/authenticator.d" -o "$CCDIR/authenticator.o" $SOURCE_PATH"authenticator.cpp"
echo "[Code Coverage] Finished building: authenticator.cpp"
 
#Building file: data_manager.cpp
$CSGPP $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/data_manager.d" -MT"$CCDIR/data_manager.d" -o "$CCDIR/data_manager.o" $SOURCE_PATH"data_manager.cpp"
echo "[Code Coverage] Finished building: data_manager.cpp"

#Building file: hpfeeds.cpp
$CSGPP $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/hpfeeds.d" -MT"$CCDIR/hpfeeds.d" -o "$CCDIR/hpfeeds.o" $SOURCE_PATH"hpfeeds.cpp"
echo "[Code Coverage] Finished building: hpfeeds.cpp"

#Building file: rw_lock_t.cpp
$CSGPP $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/rw_lock_t.d" -MT"$CCDIR/rw_lock_t.d" -o "$CCDIR/rw_lock_t.o" $SOURCE_PATH"rw_lock_t.cpp"
echo "[Code Coverage] Finished building: rw_lock_t.cpp"

#Building file: message_router.cpp
$CSGPP $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/message_router.d" -MT"$CCDIR/message_router.d" -o "$CCDIR/message_router.o" $SOURCE_PATH"message_router.cpp"
echo "[Code Coverage] Finished building: message_router.cpp"

#Building file: broker_connection.cpp
$CSGPP $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/broker_connection.d" -MT"$CCDIR/broker_connection.d" -o "$CCDIR/broker_connection.o" $SOURCE_PATH"broker_connection.cpp"
echo "[Code Coverage] Finished building: broker_connection.cpp"

#Building file: broker.cpp
$CSGPP $CFLAGS $PACKAGE_VERSION -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/broker.d" -MT"$CCDIR/broker.d" -o "$CCDIR/broker.o" $SOURCE_PATH"broker.cpp"
echo "[Code Coverage] Finished building: broker.cpp"

#Building file: hpfeeds_client.cpp [NO cc]
g++ $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/hpfeeds_client.d" -MT"$CCDIR/hpfeeds_client.d" -o "$CCDIR/hpfeeds_client.o" $LOCAL_PATH"hpfeeds_client.cpp"
echo "[Code Coverage] Finished building: hpfeeds_client.cpp [NO cc]"

#Building file: authenticator_tests.cpp
g++ $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/authenticator_tests.d" -MT"$CCDIR/authenticator_tests.d" -o "$CCDIR/authenticator_tests.o" $LOCAL_PATH"authenticator_tests.cpp"
echo "[Code Coverage] Finished building: authenticator_tests.cpp"

#Building file: data_manager_tests.cpp
g++ $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/data_manager_tests.d" -MT"$CCDIR/data_manager_tests.d" -o "$CCDIR/data_manager_tests.o" $LOCAL_PATH"data_manager_tests.cpp"
echo "[Code Coverage] Finished building: data_manager_tests.cpp"

#Building file: hpfeeds_tests.cpp
g++ $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/hpfeeds_tests.d" -MT"$CCDIR/hpfeeds_tests.d" -o "$CCDIR/hpfeeds_tests.o" $LOCAL_PATH"hpfeeds_tests.cpp"
echo "[Code Coverage] Finished building: hpfeeds_tests.cpp"

#Building file: integration_tests.cpp
g++ $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/integration_tests.d" -MT"$CCDIR/integration_tests.d" -o "$CCDIR/integration_tests.o" $LOCAL_PATH"integration_tests.cpp"
echo "[Code Coverage] Finished building: integration_tests.cpp"

#Building file: hpfeedbroker_tests.cpp
g++ $CFLAGS -I/usr/local/include/Poco -I/usr/local/include/Poco/Net -I/usr/local/include/Poco/Util -I/usr/local/include/cppunit -I/usr/include/mongo/client -O3 -g -std=c++0x -Wall -Werror -c -fmessage-length=0 -MMD -MP -MF"$CCDIR/hpfeedbroker_tests.d" -MT"$CCDIR/hpfeedbroker_tests.d" -o "$CCDIR/hpfeedbroker_tests.o" $LOCAL_PATH"hpfeedbroker_tests.cpp"
echo "[Code Coverage] Finished building: hpfeedbroker_tests.cpp"
 
echo "[Code Coverage] Entering code_coverage folder and linking..."
cd $CCDIR

#Linking
$CSGPP -L/usr/local/lib -L/usr/lib -o "../tentacool_tests_cc"  ./authenticator.o ./data_manager.o ./hpfeeds.o ./rw_lock_t.o ./message_router.o ./broker_connection.o ./broker.o ./hpfeeds_client.o ./hpfeedbroker_tests.o ./authenticator_tests.o ./data_manager_tests.o ./hpfeeds_tests.o ./integration_tests.o -lPocoNet -lPocoUtil -lPocoFoundation -lcppunit -lmongoclient -lboost_filesystem-mt -lboost_system-mt -lboost_thread-mt -lboost_program_options-mt -lpthread
echo "[Code Coverage] Executable generated!"

echo "[Code Coverage] Exiting from code_coverage folder and runnings tests"
cd ..

./tentacool_tests_cc

rc=$?
if [ $rc != 0 ] ; then
    echo "[Code Coverage] An error was occurred during tests execution."
    exit $rc
fi

echo "[Code Coverage] Entering code_coverage folder and creating HTML report..."
cd $CCDIR

cmcsexeimport -e ../tentacool_tests_cc.csexe -m ../tentacool_tests_cc.csmes -t "CppTestsCoverage" 

cmreport --title=../tentacool_tests_cc.csexe -m ../tentacool_tests_cc.csmes --select='.*' --bargraph --toc -global=all --method=all --source=all --execution=all --html=tentacool_cc.html

cd ..
cleanup
echo "[Code Coverage] Exiting"
