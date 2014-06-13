#include "authenticator_tests.hpp"
#include "data_manager_tests.hpp"
#include "hpfeeds_tests.hpp"
#include "integration_tests.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestRunner.h>

// The test execution order follow the suite registration order

CPPUNIT_TEST_SUITE_REGISTRATION(Authenticator_test);
CPPUNIT_TEST_SUITE_REGISTRATION(DataManager_test);
CPPUNIT_TEST_SUITE_REGISTRATION(Hpfeeds_test);
CPPUNIT_TEST_SUITE_REGISTRATION(Integration_test);

class CoverageScannerListener : public TestListener
{
public:
    CoverageScannerListener():m_testFailed(false){}
    void startTest( CppUnit::Test *test )
    {
        m_testFailed=false;
        #ifdef __COVERAGESCANNER__
            int pos;
            // Adjusting the name of the test to display the tests
            // in a tree view in CoverageBrowser
            std::string testname = "CppUnit/" + test->getName();
            while ( (pos=testname.find("::",0)) != std::string::npos )
            testname.replace(pos,2,"/");

            // Reset the code coverage data to get only the code coverage
            // of the actual unit test.
            __coveragescanner_clear();
            __coveragescanner_testname(testname.c_str()) ;
       #endif
    }

    void addFailure( const CppUnit::TestFailure &failure )
    {
        m_testFailed=true;
    }

    void endTest( CppUnit::Test *test )
    {
        #ifdef __COVERAGESCANNER__
            // Recording the execution state in the coverage report
            if (m_testFailed)
                __coveragescanner_teststate("FAILED");
            else
                __coveragescanner_teststate("PASSED");

            // Saving the code coverage report of the unit test
            __coveragescanner_save();
            __coveragescanner_testname("");
        #endif
    }

private:
    bool m_testFailed;
    // Prevents the use of the copy constructor and operator.
    CoverageScannerListener( const CoverageScannerListener &copy );
    void operator =( const CoverageScannerListener &copy );
};

int main (int argc, char **argv)
{
    #ifdef __COVERAGESCANNER__
        __coveragescanner_install("hpfeeds_broker_test");
    #endif

    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // Add a listener that collects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener( &result );

    // Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener( &progress );

    // Add a listener that saves the code coverage information
    CoverageScannerListener coveragescannerlistener;
    controller.addListener(&coveragescannerlistener );

    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().
                    makeTest());
    runner.run( controller );
    if (result.wasSuccessful())
        cout<<"Successful end of tests"<<endl;
    else
        cout<<"End of tests with errors"<<endl;
    // Return error code 1 if the one of test failed.
    return result.wasSuccessful() ? 0 : 1;
}
