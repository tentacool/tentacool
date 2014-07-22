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
#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Util/Validator.h>
#include <Poco/Util/IntValidator.h>
#include <Poco/Util/RegExpValidator.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerParams.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <unistd.h>
#include <Poco/Path.h>
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


TCPConnectionFactory::TCPConnectionFactory(DataManager* data_m) : _data_m(data_m)
{
}

TCPConnectionFactory::~TCPConnectionFactory()
{
}

Net::TCPServerConnection* TCPConnectionFactory::createConnection(
    const Net::StreamSocket& socket)
{
    return new BrokerConnection(socket, _data_m);
}

BrokerApplication::BrokerApplication() :
        debugTag(""), m_helpRequested(false), _debug_mode(false),
        logger(Logger::get("HF_Broker")), port(10000), num_threads(20),
        queuelen(46), idletime(100), _data_mode(false), _stdout_logging(false),
        _filename_spec(false), _exe_path("./"), _log_file("tentacool.log"),
        _filename("auth_keys.dat"), _mongo_ip("127.0.0.1"), _mongo_port("27017"),
        _mongo_db("hpfeeds"), _mongo_collection("auth_key"), _data_manager(NULL)
{
    // Default broker name
    BrokerConnection::Broker_name = "@hp1";
    // Broker is not stopped at the begin
    BrokerConnection::isStopped = false;
    // Set default priority of the logger messages
    logger.setLevel(Message::PRIO_INFORMATION);
}

BrokerApplication::~BrokerApplication()
{
    if (!m_helpRequested)
        logger.information("Tentacool shutting down");
}

int BrokerApplication::getPath(char* pBuf)
{
    //! Retrieve the path of the Broker executable
    int bytes = readlink("/proc/self/exe", pBuf, PATH_MAX);
    if (bytes >= 0) {
        pBuf[bytes] = '\0';
        string line(pBuf, bytes);
        line = line.substr(0, line.find_last_of("\\/"));
        line += '/';
        size_t length = line.copy(pBuf, line.length());
        pBuf[length] = '\0';
        if(length == line.length())
            return line.length();
    };
    return -1;
}

void BrokerApplication::initialize(Poco::Util::Application& self)
{
    //! Load configuration file, if present, and initialize the application
    loadConfiguration();

    if (m_helpRequested) {
        displayHelp();
    } else {
        try {
            // Get the path of the executable
            char pBuf[PATH_MAX];

            int path_len = getPath(pBuf);
            if (path_len > 0) {
                _exe_path = string(pBuf, path_len);
                if (!_filename_spec)
                    _filename = _exe_path + _filename;
                _log_file = _exe_path + _log_file;
            }

            // Setting Logger
            AutoPtr<SimpleFileChannel> sfChannel(new SimpleFileChannel);
            sfChannel->setProperty("path", _log_file);
            // Overwrite file at 2KB
            sfChannel->setProperty("rotation", "2 K");
            AutoPtr<PatternFormatter> sfPF(new PatternFormatter);
            sfPF->setProperty("pattern","[Th. %I] %Y-%m-%d %H:%M:%S [%p] %s: %t");
            sfPF->setProperty("times", "local");

            AutoPtr<FormattingChannel> sfFC(new FormattingChannel(sfPF, sfChannel));
            AutoPtr<ConsoleChannel> cc(new ConsoleChannel());

            AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

            pSplitter->addChannel(sfFC); //on file by default
            if (_stdout_logging)
                pSplitter->addChannel(cc);

            logger.setChannel(pSplitter);

            // Set priority at least for INFO messages
            if(_debug_mode) {
                debugTag = "[DEBUG_LOGGING_MODE]";
            } else {
                logger.setLevel(Message::PRIO_INFORMATION);
            }

            //Create File manager
            #ifdef __WITH_MONGO__
            if(!_data_mode)
                _data_manager = new DataManager(_filename);
            else
                _data_manager = new DataManager(_mongo_ip,_mongo_port, _mongo_db,
                    _mongo_collection);
            #else
                _data_manager = new DataManager(_filename);
            #endif
        } catch(Poco::Exception& exc) {
            logger.error(exc.displayText());
            _data_manager = NULL;
            return;
        }
    }
    Poco::Util::ServerApplication::initialize(self);
}

void BrokerApplication::uninitialize()
{
    //! Uninitialize the application
    Poco::Util::ServerApplication::uninitialize();
}

void BrokerApplication::defineOptions(Poco::Util::OptionSet& options)
{
    // Define the application options
    // options is a Poco::Util::OptionSet&.
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
    Poco::Util::Option("version", "ver", "Show the version")
        .required(false)
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("log_stdout", "v", "write broker output on the stdout")
        .required(false)
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("name", "n", "give a name to the broker (@hp1 default)")
        .required(false)
        .argument("<name of the broker>")
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("port", "p", "define the port Tentacool will listen to")
        .required(false)
        .argument("<port number>")
        .validator(new Util::IntValidator(1024, 65535))
        .callback(Poco::Util::OptionCallback<BrokerApplication>
                      (this, &BrokerApplication::handlePort)));

    options.addOption(
    Poco::Util::Option("file", "f", "filename where fetch the authentication data")
        .required(false)
        .argument("<filename>")
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("max_thread", "max_th", "define the maximum number of "
            "simultaneous threads available. The default value is 10")
        .required(false)
        .argument("<Max threads>")
        .validator(new Util::IntValidator(10, 64))
        .callback(Poco::Util::OptionCallback<BrokerApplication>
            (this, &BrokerApplication::handleServerParams)));

    options.addOption(
    Poco::Util::Option("idletime", "idle", "define the maximum idle time for a "
            "thread before it is terminated. The default value is 100")
        .required(false)
        .argument("<Idle time>")
        .validator(new Util::IntValidator(1, 200))
        .callback(Poco::Util::OptionCallback<BrokerApplication>
            (this, &BrokerApplication::handleServerParams)));

#ifdef __WITH_MONGO__
    options.addOption(
    Poco::Util::Option("mode", "m", "set the way to fetch authentication data")
        .required(false)
        .argument("<file|mongodb>")
        .validator(new Util::RegExpValidator("file|mongodb"))
        .callback(Poco::Util::OptionCallback<BrokerApplication>
                    (this, &BrokerApplication::handleMode)));

    options.addOption(
    Poco::Util::Option("mongoip", "m_ip", "The IP address of the mongodb")
        .required(false)
        .argument("<IP of mongodb>")
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("mongoport", "m_port", "The port where mongodb is listening to")
        .required(false)
        .argument("<Port of mongodb>")
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("mongodb", "m_db", "The name of the db")
        .required(false)
        .argument("<Name of mongo DB>")
        .repeatable(false));

    options.addOption(
    Poco::Util::Option("mongocoll", "m_coll", "The the name of the collection "
            "in 'mongodb' database")
        .required(false)
        .argument("<Mongo Collection>")
        .repeatable(false));
#endif
}

void BrokerApplication::handleOption(const std::string& name,
                                                    const std::string& value)
{
    //! Handle the option values
    //\param name is a string with the option name.
    //\param value is a string with the value.
    Poco::Util::ServerApplication::handleOption(name, value);
    if (name == "debug") {
        //Set priority at least for DEBUG messages if the option is setted
        _debug_mode = true;
        logger.setLevel(Message::PRIO_DEBUG);
    } else if ( name =="help") {
        m_helpRequested = true;
    } else if (name == "log_stdout") {
        _stdout_logging = true;
    } else if (name == "file") {
        _filename = value;
        _filename_spec = true;
        logger.debug("Filename: " + value);
    } else if (name == "name") {
        BrokerConnection::Broker_name = value;
        logger.debug("Broker name: " + value);
    } else if (name == "version") {
        cout << PACKAGE_VERSION << endl;
        exit(0);
    }
#ifdef __WITH_MONGO__
    else if(name=="mongoip") {
        _mongo_ip = value;
        logger.debug("Mongo IP: " + value);
    } else if(name=="mongoport") {
        _mongo_port = value;
        logger.debug("Mongo Port: " + value);
    } else if(name=="mongodb") {
        _mongo_db = value;
        logger.debug("Mongo db name: " + value);
    } else if(name=="mongocoll") {
        _mongo_collection = value;
        logger.debug("Mongo collection name: " + value);
    }
#endif
}

#ifdef __WITH_MONGO__
void BrokerApplication::handleMode(const std::string& name,
                                    const std::string& value)
{
    // name is a string with the option name.
    // value is a string with the value.
    if (!value.compare("file"))
        _data_mode = false;
    else
        _data_mode = true; // 0->file 1->mongodb

    logger.debug("Data fetching mode: " + value);
}
#endif

void BrokerApplication::handlePort(const std::string& name,
                                        const std::string& value)
{
    // name is a string with the option name.
    // value is a string with the value.
    port = (unsigned short) config().getInt("BrokerApplication.port",
        NumberParser::parseUnsigned(value));
    logger.debug("Tentacool port setted to " + NumberFormatter::format(port));
}

// Handle Server Parameters
// is a string with the option name.
// value is a string with the value.
void BrokerApplication::handleServerParams(const std::string& name,
    const std::string& value)
{
    if (name == "max_thread") {
        num_threads = NumberParser::parseUnsigned(value);
    } else if (name == "idletime") {
        idletime = NumberParser::parseUnsigned(value);
    }
}

void BrokerApplication::displayHelp()
{
    //! Display the result of -h option
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setWidth(120);
    helpFormatter.setCommand(commandName());
#ifdef __WITH_MONGO__
    helpFormatter.setUsage(string(PACKAGE_VERSION) +
    " can run in two modes: \n"
    "-n name    [Give a specific name to the broker - default '@hp1']\n"
    "-m file    [Fetch users authentication datas from a structured file]\n"
    "-m mongodb [Fetch users authentication datas from mongodb collection]"
    "\n\n"
    "If not specified the broker fetch data from the file: 'auth_keys.dat'"
    "\n");
#else
    helpFormatter.setUsage(string(PACKAGE_VERSION) +
    " is running in file mode: \n"
    "broker fetch users authentication datas from a structured file\n\n"
    "If not specified the broker fetch data from the file: 'auth_keys.dat'"
    "\n"
    "-n name    [Give a specific name to the broker - default '@hp1']\n"
    "-f file    [Give a specific filename for authentication datas]\n");
#endif
    helpFormatter.setHeader("Tentacool is a hpfeeds messages broker.");
    helpFormatter.format(std::cout);
}

int BrokerApplication::main(const std::vector<std::string>& args)
{
    if (!m_helpRequested) {
        try {
            // Main
            if(_data_manager == NULL)
                return Poco::Util::Application::EXIT_IOERR;

            logger.information("Tentacool started " + debugTag);

            // Create a server socket in order to listen to the port
            Net::ServerSocket svs(port);
            logger.information("Tentacool server socket in listening.");
            // Configure some server parameters.
            Net::TCPServerParams* pParams = new Net::TCPServerParams();
            logger.information("Setting max threads number: " +
                NumberFormatter::format(num_threads));
            pParams->setMaxThreads(num_threads);
            pParams->setMaxQueued(queuelen);
            logger.information("Setting idle time: " + NumberFormatter::format(idletime));
            pParams->setThreadIdleTime(idletime);

            Net::TCPServer server( new TCPConnectionFactory(_data_manager), svs, pParams);
            server.start();
            // TCPServer uses a separate thread to accept incoming connections.
            // Thus, the call to start() returns immediately, and the server
            // continues to run in the background.

            // wait for CTRL-C or kill
            waitForTerminationRequest();

            server.stop();

            BrokerConnection::isStopped = true;

            // free data_manager
            delete _data_manager;
        } catch(exception& e) {
            logger.information(e.what());
            return Poco::Util::Application::EXIT_IOERR;
        }
    }
    return Poco::Util::Application::EXIT_OK;
}
