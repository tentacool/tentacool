#include <iostream>
#include <Poco/String.h>
#include <Poco/Message.h>
#include <Poco/Logger.h>
#include <Poco/Channel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/Formatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include <Poco/Util/Validator.h>
#include <Poco/Util/IntValidator.h>
#include <Poco/Util/RegExpValidator.h>
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "broker.hpp"
#include "broker_connection.hpp"
#include "data_manager.hpp"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

using namespace std;
using namespace Poco;
using Poco::AutoPtr;
using Poco::Channel;
using Poco::PatternFormatter;

TCPConnectionFactory::TCPConnectionFactory(DataManager* data_m): _data_m(data_m)
{}
TCPConnectionFactory::~TCPConnectionFactory()
{}
Net::TCPServerConnection* TCPConnectionFactory::createConnection(const Poco::Net::StreamSocket& socket)
{
    return new BrokerConnection(socket, _data_m);
}

BrokerApplication::BrokerApplication() :
        m_helpRequested(false), _debug_mode(false)
        , logger(Logger::get("HF_Broker"))
        , port(10000), num_threads(10), queuelen(20), idletime(100)
        , _data_mode(false), _stdout_logging(false)
        , _log_file("hpfeedsBroker.log"), _filename("auth_keys.dat")
        , _mongo_ip("127.0.0.1"), _mongo_port("27017")
        , _mongo_db("hpfeeds"), _mongo_collection("auth_key")
        , _data_manager(NULL)
{
    //Default broker name
    BrokerConnection::Broker_name="@hp1";
}

BrokerApplication::~BrokerApplication()
{   //! Destructor
    if(!m_helpRequested)logger.information("HpfeedsBroker shutting down");
}


void BrokerApplication::initialize(Poco::Util::Application& self)
    {
        //! Load configuration file, if present, and initialize the application
        loadConfiguration(); // load default configuration files, if present
        Poco::Util::ServerApplication::initialize(self);
    }


void BrokerApplication::uninitialize()
    {   //! Uninitialize the application
        Poco::Util::ServerApplication::uninitialize();
    }

void BrokerApplication::defineOptions(Poco::Util::OptionSet& options)
    {
        //! Define the application options
        //\param options is a Poco::Util::OptionSet&.
        Poco::Util::ServerApplication::defineOptions(options);

        options.addOption(
        Poco::Util::Option("help", "h", "display help information on command line arguments")
            .required(false)
            .repeatable(false));

        options.addOption(
        Poco::Util::Option("debug", "d", "active the debug informations printing")
            .required(false)
            .repeatable(false));

        options.addOption(
        Poco::Util::Option("log_stdout", "v", "write broker output on the stdout")
            .required(false)
            .repeatable(false));

        options.addOption(
        Poco::Util::Option("broker_name", "n", "give a name to the broker (@hp1 default)")
            .required(false)
            .argument("broker_name")
            .repeatable(false));

        options.addOption(
        Poco::Util::Option("port", "p", "define the port HpfeedsBroker will listen to")
            .required(false)
            .argument("port")
            .validator(new Util::IntValidator(1024, 65535))
            .callback(Poco::Util::OptionCallback<BrokerApplication>
                (this, &BrokerApplication::handlePort)));

        options.addOption(
        Poco::Util::Option("file", "f", "filename where fetch the authentication data")
            .required(false)
            .argument("file")
            .repeatable(false));

#ifdef __WITH_MONGO__
        options.addOption(
        Poco::Util::Option("mode", "m", "set the way to fetch authentication data")
            .required(false)
            .argument("mode")
            .validator(new Util::RegExpValidator("file|mongodb"))
            .callback(Poco::Util::OptionCallback<BrokerApplication>
                (this, &BrokerApplication::handleMode)));

        options.addOption(
        Poco::Util::Option("mongoip", "m_ip", "The IP address of the mongodb")
            .required(false)
            .argument("mongoip")
            .repeatable(false));
        options.addOption(
        Poco::Util::Option("mongoport", "m_port", "The port where mongodb is listening to")
            .required(false)
            .argument("mongoport")
            .repeatable(false));
        options.addOption(
        Poco::Util::Option("mongodb", "m_db", "The name of the db")
            .required(false)
                .argument("mongodb")
                .repeatable(false));
        options.addOption(
        Poco::Util::Option("mongocoll", "m_coll", "The the name of the collection in 'mongodb' database")
            .required(false)
            .argument("mongocoll")
            .repeatable(false));
#endif
    }

void BrokerApplication::handleOption(const std::string& name, const std::string& value)
    {
        //! Handle the option values
        //\param name is a string with the option name.
        //\param value is a string with the value.
        Poco::Util::ServerApplication::handleOption(name, value);
        if(name=="debug"){
            //Set priority at least for DEBUG messages if the option is setted
            _debug_mode = true;
            logger.setLevel(Message::PRIO_DEBUG);
        }else if(name=="help"){
            m_helpRequested = true;
        }else if(name=="log_stdout"){
            _stdout_logging = true;
        }else if(name=="file"){
            _filename = value;
            logger.debug("Filename: "+value);
        }else if(name=="broker_name"){
            BrokerConnection::Broker_name = value;
            logger.debug("Broker name: "+value);
        }
#ifdef __WITH_MONGO__
        else if(name=="mongoip"){
            _mongo_ip = value;
            logger.debug("Mongo IP: "+value);
        }else if(name=="mongoport"){
            _mongo_port = value;
            logger.debug("Mongo Port: "+value);
        }else if(name=="mongodb"){
            _mongo_db = value;
            logger.debug("Mongo db name: "+value);
        }else if(name=="mongocoll"){
            _mongo_collection = value;
            logger.debug("Mongo collection name: "+value);
        }
#endif
    }

void BrokerApplication::handleMode(const std::string& name, const std::string& value)
    {
        //!\param name is a string with the option name.
        //!\param value is a string with the value.
        if(!value.compare("file")) _data_mode = false;
        else _data_mode = true; //0->file 1->mongodb
        logger.information("Data fetching mode: "+value);
    }

void BrokerApplication::handlePort(const std::string& name, const std::string& value)
    {
        //!
        //!\param name is a string with the option name.
        //!\param value is a string with the value.
        port = (unsigned short) config().getInt("BrokerApplication.port",
                NumberParser::parseUnsigned(value));
        logger.information("HpfeedsBroker port setted to " +
                NumberFormatter::format(port));
    }
void BrokerApplication::displayHelp()
    {
        //! Display the result of -h option
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
#ifdef __WITH_MONGO__
        helpFormatter.setUsage("HpfeedsBroker can run in two modes: \n"
            "-n name    [Give a specific name to the broker - default '@hp1']\n"
            "-m file    [Fetch users authentication datas from a structured file]\n"
            "-m mongodb [Fetch users authentication datas from mongodb collection]\n\n"
            "If not specified the broker fetch data from a file named 'auth_keys.dat'\n");
#else
        helpFormatter.setUsage("HpfeedsBroker is running in file mode: \n"
            " broker fetch users authentication datas from a structured file\n\n"
            "If not specified the broker fetch data from a file named 'auth_keys.dat'\n"
            "-n name    [Give a specific name to the broker - default '@hp1']\n");
#endif
        helpFormatter.setHeader("HpfeedsBroker is a hpfeeds messages broker.");
        helpFormatter.format(std::cout);
    }

int BrokerApplication::main(const std::vector<std::string>& args)
    {
        try{
        //! Main
        if(m_helpRequested){
            displayHelp();
        }else{
            string debugTag("");

            //Setting Logger
            AutoPtr<SimpleFileChannel> sfChannel(new SimpleFileChannel);
            sfChannel->setProperty("path", _log_file);
            sfChannel->setProperty("rotation", "2 K"); //Overwrite file at 2KB
            AutoPtr<PatternFormatter> sfPF(new PatternFormatter);
            sfPF->setProperty("pattern", "[Th. %I] %Y-%m-%d %H:%M:%S [%p] %s: %t");

            AutoPtr<FormattingChannel> sfFC(new FormattingChannel(sfPF, sfChannel));
            AutoPtr<ConsoleChannel> cc (new ConsoleChannel());

            AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

            pSplitter->addChannel(sfFC); //on file by default
            if(_stdout_logging) pSplitter->addChannel(cc);

            logger.setChannel(pSplitter);

            //Set priority at least for INFO messages
            if(_debug_mode){
                debugTag="[DEBUG_LOGGING_MODE]";
            }else{
                logger.setLevel(Message::PRIO_INFORMATION);
            }

            //Create File manager
            try{
            #ifdef __WITH_MONGO__
                if(!_data_mode) _data_manager = new DataManager(_filename);
                else _data_manager = new DataManager(_mongo_ip,_mongo_port,
                                                    _mongo_db,_mongo_collection);
            #else
                _data_manager = new DataManager(_filename);
            #endif
            }catch(Poco::Exception& exc){
                logger.error(exc.displayText());
                return Poco::Util::ServerApplication::EXIT_IOERR;
            }
            logger.information("HpfeedsBroker started "+debugTag);

            // Create a server socket in order to listen to the port
            Net::ServerSocket svs(port);
            logger.information("HpfeedsBroker server socket in listening.");

            // Configure some server parameters.
            Net::TCPServerParams* pParams = new Net::TCPServerParams();
            logger.information("Setting max threads number: " +
                    NumberFormatter::format(num_threads));
            pParams->setMaxThreads(num_threads);
            logger.information("Setting queue length: " +
                    NumberFormatter::format(queuelen));
            pParams->setMaxQueued(queuelen);
            logger.information("Setting idle time: " +
                    NumberFormatter::format(idletime));
            pParams->setThreadIdleTime(idletime);

            Net::TCPServer server(
                    new TCPConnectionFactory(_data_manager),svs, pParams);
            server.start();

            // wait for CTRL-C or kill
            waitForTerminationRequest();

            server.stop();
        }
        }catch(exception& e){
            logger.information(e.what());
            return Poco::Util::Application::EXIT_IOERR;
        }
        return Poco::Util::Application::EXIT_OK;
    }

//----------------------------------------
//	FeedsBrokerApplication main
//----------------------------------------
//POCO_SERVER_MAIN(BrokerApplication)
