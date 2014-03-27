AC_INIT([tentacool], [0.1.0], [lomato@gmail.com, abes975@gmail.com], [tentacool], [http://github.com/abes975/tentacool])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 no-define foreign])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AM_PROG_CC_C_O
AC_CONFIG_FILES([Makefile])
AC_CHECK_LIB([PocoFoundation],[main],[],AC_MSG_ERROR([unable to find PocoFoundation]))
AC_CHECK_LIB([PocoUtil],[main],[],AC_MSG_ERROR([unable to find PocoUtil]))
AC_CHECK_LIB([PocoNet],[main],[],AC_MSG_ERROR([unable to find PocoNet]))
AC_CHECK_LIB([mongoclient],[main],[],AC_MSG_ERROR([unable to find MongoClient]))

#check for boost
AC_CHECK_LIB(boost_thread-mt, main, , [
        AC_CHECK_LIB(boost_thread, main, , [
                AC_MSG_ERROR("unable to find boost_thread")
        ])
])

AC_CHECK_LIB(boost_program_options-mt, main, , [
        AC_CHECK_LIB(boost_program_options, main, , [
                AC_MSG_ERROR("unable to find boost_options")
        ])
])

AC_CHECK_LIB(boost_filesystem-mt, main, , [
        AC_CHECK_LIB(boost_filesystem, main, , [
                AC_MSG_ERROR("unable to find boost_filesystem")
        ])
])

AC_CHECK_LIB(boost_system-mt, main, , [
        AC_CHECK_LIB(boost_system, main, , [
                AC_MSG_ERROR("unable to find boost_system")
        ])
])

# check for mongo::client
#AC_CHECK_HEADERS([client/dbclient.h], , [AC_MSG_ERROR("unable to find mongo::client")])

AC_ARG_WITH(mongoclient,
  AS_HELP_STRING([--with-mongoclient=DIR],[location of mongoclient]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include/mongo/"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_MSG_CHECKING(for MongoClient)
	AC_TRY_COMPILE([
		#include <mongo/client/dbclient.h>
		],[main],
		have_mongo=yes, have_mongo=no)
	if test "$have_mongo" = yes; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAS_MONGO, 1, [Define to 1 if MongoClient available])
	else
	    AC_MSG_RESULT(no)
		 echo
		 echo "ERROR: Unable to find mongo::client header"
		 echo
		 exit 1
	fi




AC_ARG_WITH(poco,
  AS_HELP_STRING([--with-poco=DIR],[location of Poco]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include/Poco"
        CPPFLAGS="$CPPFLAGS -I${withval}/include/Poco/Net"
        CPPFLAGS="$CPPFLAGS -I${withval}/include/Poco/Util"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[])

AC_MSG_CHECKING(for Poco)
	AC_TRY_COMPILE([
		#include <Poco/Net/ServerSocket.h>
		#include <Poco/Net/StreamSocket.h>
		#include <Poco/Timestamp.h>
		#include <Poco/DateTime.h>
		#include <Poco/DateTimeParser.h>
		#include <Poco/DateTimeFormatter.h>
		#include <Poco/Net/NetException.h>
                #include <Poco/AutoPtr.h>
                #include <Poco/Util/ServerApplication.h>
		],,
		have_poco=yes, have_poco=no)
	if test "$have_poco" = yes; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(HAS_POCO, 1, [Define to 1 if Poco available])
	else
	    AC_MSG_RESULT(no)
		 echo
		 echo "ERROR: Poco development libraries are required. Install and try again (see wiki:prerequisites)."
		 echo
		 exit 1
	fi

AC_CHECK_LIB([pthread],[pthread_create], [], AC_MSG_ERROR([unable to find the pthread_create() function]))



AC_OUTPUT
