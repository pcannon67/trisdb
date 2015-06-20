/* 
 * File:   Shell.cpp
 * Author: tiepologian <tiepolo.gian@gmail.com>
 * 
 * Created on 28 maggio 2014, 12.27
 */

#include "Shell.h"
#include "UnixSocketClient.h"
#include "TcpClient.h"

Shell::Shell(std::string port, std::string socket) {
    if(socket != "nosocket") {
        this->useSocket = true;
        this->_socketPath = socket;
    }
    else {
        this->useSocket = false;
        this->_port = port;
    }
}

Shell::Shell(const Shell& orig) {
    //
}

Shell::~Shell() {
    //
}

void Shell::run() {
    GenericClient* client;
    if(this->useSocket) client = new UnixSocketClient(this->_socketPath);
    else client = new TcpClient(this->_port);    
    LogManager::getSingleton()->log(LogManager::LINFO, "Shell ready");
    std::cout << std::endl;
#ifdef __linux__
    char *buf;
    rl_bind_key('\t', rl_abort); //disable auto-complete
    while ((buf = readline(">> ")) != NULL) {
        std::string input(buf);
        try {
            if (input != "") {
                if (boost::to_upper_copy(input) == "QUIT") break;
		std::string cmd = boost::to_upper_copy(input.substr(0, input.find(" ")));
		if(Utils::ValidCommands.find(cmd) == Utils::ValidCommands.end()) std::cout << "INVALID COMMAND" << std::endl;
                else {
		    RequestPointer req(new QueryRequest);
                    req->add_query(input);
                    req->set_timestamp(std::to_string(TimeUtils::getCurrentTimestamp()));
                    printQueryResult(client->connect(req), boost::to_upper_copy(input.substr(0, input.find(" "))));
                    req.reset();
		}
            }
        } catch (Utils::CustomException& e) {
            LogManager::getSingleton()->log(LogManager::LERROR, e.what());
        }

        if (buf[0] != 0)
            add_history(buf);
    }
    free(buf);
#else   
    while (true) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        try {
            if (input != "") {
                if (boost::to_upper_copy(input) == "QUIT") break;
                RequestPointer req(new QueryRequest);
                req->add_query(input);
                req->set_timestamp(std::to_string(TimeUtils::getCurrentTimestamp()));
                printQueryResult(client->connect(req), boost::to_upper_copy(input.substr(0, input.find(" "))));
                req.reset();
            }
        } catch (Utils::CustomException& e) {
            LogManager::getSingleton()->log(LogManager::LERROR, e.what());
        }
    }
#endif
    // if exited loop, shutdown
    quit();
    exit(0);
    delete client;
}

void Shell::quit() {
    // log shell closed?
}

void Shell::printQueryResult(QueryResponse res, std::string cmd) {
    std::cout << std::endl;
    bprinter::TablePrinter tp(&std::cout);
    if (cmd == "GETS") tp.AddColumn("Subject", 20);
    else if (cmd == "GETP") tp.AddColumn("Predicate", 20);
    else if (cmd == "GETO") tp.AddColumn("Object", 20);
    else if (cmd == "GET") {
        tp.AddColumn("Subject", 20);
        tp.AddColumn("Predicate", 20);
        tp.AddColumn("Object", 20);
    } else if (cmd == "COUNT") tp.AddColumn("Count", 20);
    else if (cmd == "STATUS") {
        tp.AddColumn("Property", 20);
        tp.AddColumn("Value", 20);
    }
    if ((cmd.substr(0, 2) == "GE") || (cmd == "COUNT") || (cmd == "STATUS")) tp.PrintHeader();
    for (int i = 0; i < res.data_size(); i++) {
        if (cmd == "GETS") tp << res.data(i).subject();
        else if (cmd == "GETP") tp << res.data(i).predicate();
        else if (cmd == "GETO") tp << res.data(i).object();
        else if (cmd == "GET") tp << res.data(i).subject() << res.data(i).predicate() << res.data(i).object();
        else if (cmd == "COUNT") tp << res.data(i).object();
        else if (cmd == "STATUS") tp << res.data(i).subject() << res.data(i).object();
    }
    if ((cmd.substr(0, 2) == "GE") || (cmd == "COUNT") || (cmd == "STATUS")) tp.PrintFooter();
    //double now = TimeUtils::getCurrentTimestamp();
    int queryTime = atoi(res.timestamp().c_str());
    if (queryTime == 0) queryTime = 1;
    if (cmd.substr(0, 2) == "GE") std::cout << "\nRead " << res.data_size() << " rows in " << queryTime << "ms" << std::endl << std::endl;
    else std::cout << "Query executed in " << queryTime << "ms" << std::endl << std::endl;
}