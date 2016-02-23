#ifndef SERVER_H
#define SERVER_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "base.h"
using namespace std;


class Server {
private:
	int m_nListenPort;

	// socket specific members
    int m_nListenSd;        // listen socket descriptor
    int m_nMaxFd;           // Maximum file descriptor number
	fd_set m_masterSet;
	fd_set m_readSet;

public:
	// constructor and destructor
    Server(int port);
    ~Server();

private:
	void startListener();
	void eventHandler();
	void commandShell();
	void displayUsage();

	SSL_CTX* InitServerCTX(void);
	void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
    void ShowCerts(SSL* ssl);
    void Servlet(SSL* ssl);
};

#endif
