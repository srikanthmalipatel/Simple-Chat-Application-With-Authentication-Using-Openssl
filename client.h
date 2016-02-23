#ifndef CLIENT_H
#define CLIENT_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "base.h"
using namespace std;


class Client {
private:
	int m_nListenPort;
	int m_nListenSd;
    
    int m_nClientSd;
	bool m_bisRegistered;

	char m_srvIpAddress[32];

	int m_nServerSd;		// socket used for connecting to the server, this is used when server sends client list updates.
	int m_nMaxFd;  
	fd_set m_masterSet;
	fd_set m_readSet;

public:
	// constructor and destructor
    Client(int port);
    ~Client();

private:
	void startListener();
	void eventHandler();
	void commandShell();
	void newConnectionHandler();
	void displayUsage();

    void command_help();
    int command_register(char *ipAddr, char *port);
    void command_connect(char *ipAddr, char *port);
    void command_msg(char *msg);
    int sendall(int sockFd, char *buf, int *length);
    void startChat();
	int getArgCount(char *line, const char *delim);
    CommandID getCommandID(char *comnd);
    SSL_CTX* InitCTX(void);
	void ShowCerts(SSL *);
};

#endif
