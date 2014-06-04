AC_INIT([tentacool_tests], [0.1.0], [lomato@gmail.com, abes975@gmail.com], [tentacool_tests], [http://github.com/abes975/tentacool])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 no-define foreign])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AM_PROG_CC_C_O
AC_CONFIG_FILES([Makefile])



#check pthread library
AC_CHECK_LIB([pthread],[pthread_create], [], AC_MSG_ERROR([unable to find the pthread_create() function]))

#check POCO
AC_ARG_WITH(poco,
  AS_HELP_STRING([--with-poco=DIR],[location of Poco]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include/Poco"
        CPPFLAGS="$CPPFLAGS -I${withval}/include/Poco/Net"
        CPPFLAGS="$CPPFLAGS -I${withval}/include/Poco/Util"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[
        #Default lib and headers folder
	CPPFLAGS="$CPPFLAGS -I/usr/local/include/Poco"
        CPPFLAGS="$CPPFLAGS -I/usr/local/include/Poco/Net"
        CPPFLAGS="$CPPFLAGS -I/usr/local/include/Poco/Util"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
])

AC_CHECK_LIB([PocoNet],[main],[],AC_MSG_ERROR([unable to find PocoNet]))
AC_CHECK_LIB([PocoFoundation],[main],[],AC_MSG_ERROR([unable to find PocoFoundation]))
AC_CHECK_LIB([PocoUtil],[main],[],AC_MSG_ERROR([unable to find PocoUtil]))

#check if you want mongodb support 
AC_ARG_WITH(mongoclient,
  AS_HELP_STRING([--with-mongoclient=DIR],[location of mongoclient]),
[
        if test "${withval}" != yes; then
	        CPPFLAGS="$CPPFLAGS -I${withval}/include/mongo/client"
	        LDFLAGS="$LDFLAGS -L${withval}/lib"
        else
                #no path 
	        CPPFLAGS="$CPPFLAGS -I/usr/include/mongo/client"
	        LDFLAGS="$LDFLAGS -L/usr/lib"
        fi
],[with_mongoclient=no])

#if you want mongodb support, I have to check it and the boost libraries
if test "x${with_mongoclient}" != xno; then
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

#        AC_MSG_CHECKING(for MongoClient headers)
#	        AC_TRY_COMPILE([
#		        #include "/mongo/client/dbclient.h"
#		        ], using mongo;,
#		        [AC_MSG_RESULT(yes)], [AC_MSG_RESULT(no)
#                                               AC_MSG_ERROR("unable to find MongoClient headers")])

#        AC_MSG_CHECKING(for MongoClient headers)
#        AC_CHECK_HEADER (/usr/include/mongo/client/dbclient.h, [], AC_MSG_ERROR("unable to find MongoClient headers"), [] )


        AC_CHECK_LIB([mongoclient],[main],[],AC_MSG_ERROR([unable to find MongoClient]))
        
        #Macro I check in the code
        AC_DEFINE(__WITH_MONGO__,1,[MongoDB support])
fi


#check for CPPUNIT
AC_ARG_WITH(cppunit,
  AS_HELP_STRING([--with-cppunit=DIR],[location of CppUnit]),
[
	CPPFLAGS="$CPPFLAGS -I${withval}/include/cppunit"
	LDFLAGS="$LDFLAGS -L${withval}/lib"
],[
        #If not specified, get lib and headers folder

        CPPUNIT_CFLAGS=`cppunit-config --cflags`
	CPPFLAGS="$CPPFLAGS $CPPUNIT_CFLAGS"
        if test x$CPPUNIT_CFLAGS != x; then
              echo ""
        else
              AC_MSG_ERROR([unable to find the CppUnit])
        fi

        CPPUNIT_LIBS=`cppunit-config --libs`
	LIBS="$LIBS $CPPUNIT_LIBS"
        #LD= echo $CPPUNIT_LIBS | awk '{print $1;}'
        #LDFLAGS="$LDFLAGS $LD"
        if test x"$CPPUNIT_LIBS" != x; then
              echo "checking for -lcppunit... yes"
        else
              AC_MSG_ERROR([unable to find the CppUnit])
        fi
        #cppunit_version='$cppunit-config --version'

])

#AC_CHECK_LIB([cppunit],[main], [], AC_MSG_ERROR([unable to find the CppUnit]))
#AC_TRY_LINK([#include <cppunit/TestResult.h>],[main],[],AC_MSG_ERROR([unable to find the CppUnit]))

#AC_MSG_CHECKING(for MongoClient)
#	AC_TRY_COMPILE([
#		#include <mongo/client/dbclient.h>
#		],[main],
#		have_mongo=yes, have_mongo=no)
#	if test "$have_mongo" = yes; then
#	    AC_MSG_RESULT(yes)
#	    AC_DEFINE(HAS_MONGO, 1, [Define to 1 if MongoClient available])
#	else
#	    AC_MSG_RESULT(no)
#		 echo
#		 echo "ERROR: Unable to find mongo::client header"
#		 echo
#		 exit 1
#	fi





#AC_MSG_CHECKING(for Poco)
#	AC_TRY_COMPILE([
#		#include <Poco/Net/ServerSocket.h>
#		#include <Poco/Net/StreamSocket.h>
#		#include <Poco/Timestamp.h>
#		#include <Poco/DateTime.h>
#		#include <Poco/DateTimeParser.h>
#		#include <Poco/DateTimeFormatter.h>
#		#include <Poco/Net/NetException.h>
#                #include <Poco/AutoPtr.h>
#                #include <Poco/Util/ServerApplication.h>
#		],,
#		have_poco=yes, have_poco=no)
#	if test "$have_poco" = yes; then
#	    AC_MSG_RESULT(yes)
#	    AC_DEFINE(HAS_POCO, 1, [Define to 1 if Poco available])
#	else
#	    AC_MSG_RESULT(no)
#		 echo
#		 echo "ERROR: Poco development libraries are required. Install and try again (see wiki:prerequisites)."
#		 echo
#		 exit 1
#	fi

AC_OUTPUT
