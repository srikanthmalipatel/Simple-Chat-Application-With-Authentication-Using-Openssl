#include "client.h"


Client::Client(int port) {
	this->m_nListenPort = port;
	this->m_bisRegistered = false;
	this->m_nClientSd = -1;
	FD_ZERO(&m_masterSet);
	FD_ZERO(&m_readSet);

	eventHandler();
}

Client::~Client() {

}

void Client::startListener() {
	int sd;
    struct sockaddr_in m_cliListenAddr;

    FD_SET(STDIN, &m_masterSet);
 
    // populate server address structure
    m_cliListenAddr.sin_family = AF_INET;
    m_cliListenAddr.sin_port = htons(m_nListenPort);
    m_cliListenAddr.sin_addr.s_addr = INADDR_ANY;
    // create a TCP listening socket
    if((m_nListenSd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Listening socket creation failed");
        exit(EXIT_FAILURE);
    }
    // reuse the socket in case of crash
    int optval = 1;
    if(setsockopt(m_nListenSd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    // bind m_srvAddr to the listening socket
    if(bind(m_nListenSd, (struct sockaddr *)&m_cliListenAddr, sizeof(m_cliListenAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Client is now listening on port: %d \n", m_nListenPort);
    if(listen(m_nListenSd, 3)) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    FD_SET(m_nListenSd, &m_masterSet);
	    m_nMaxFd = m_nListenSd;
}

void Client::commandShell() {
	int nArgs, connId;
	char arg1[32], arg2[32], arg3[32];
	char *commandLine = NULL;
	size_t size;
	// read the command from stdin
	ssize_t linelen = getline(&commandLine, &size, stdin);
	commandLine[linelen-1] = '\0';
	if(strlen(commandLine) == 0)
		return;

	char *argLine = (char *)malloc(strlen(commandLine));
	strcpy(argLine, commandLine);
	nArgs = getArgCount(argLine, " ");

	CommandID command_id = getCommandID(strtok(commandLine, " "));
	switch(command_id) {
		case COMMAND_HELP:
			//command_help();
			break;
		case COMMAND_REGISTER:
			if(nArgs != 3) {
				displayUsage();
				break;
			}
			// if the server is not yet registered and if the number of arguments is same as expected.
			if(m_bisRegistered) {
				cout << "Register Error: Client has already been registered to the Server. Please do not try again!" << endl;
				break;
			}

			strcpy(arg1, strtok(NULL, " "));
			strcpy(arg2, strtok(NULL, " "));
			command_register(arg1, arg2);
			break;
		case COMMAND_CONNECT:
			if(nArgs != 3) {
				displayUsage();
				break;
			}
			if(!m_bisRegistered) {
				cout << "Connect Error: Please register to the server first!" << endl;
				break;
			}
			strcpy(arg1, strtok(NULL, " "));
			strcpy(arg2, strtok(NULL, " "));
			command_connect(arg1, arg2);
			break;
		case COMMAND_MSG:
		    if(!m_bisRegistered) {
                cout << "MSG Error: Please register to the client first!" << endl;
                break;
            }
            if(nArgs != 2) {
                displayUsage();
                break;
            }
            cout << strtok(NULL, " ");
            command_msg(commandLine+4);
            break;
		default:
			displayUsage();
			break;
	}
	memset(arg1, 0, sizeof(arg1));
	memset(arg2, 0, sizeof(arg2));
}

void Client::eventHandler() {
	char buffer[1024];
    startListener();

	for(;;) {
		// copy master set to read set
        m_readSet = m_masterSet;
        // start polling on read set, which is blocking indefinitely
        if(select(m_nMaxFd+1, &m_readSet, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        // check if there is data on any of the sockets
        int bytesRead;
        char recvBuff[1024];
        for(int i=0; i<=m_nMaxFd; i++) {
            if(FD_ISSET(i, &m_readSet)) {
                if(i == 0) {
                        commandShell();
                    }
                else if(i == m_nListenSd) {
                    // new connection
                	if(m_bisRegistered) {
                        cout << "New Connection" << endl;
                        newConnectionHandler();
                    }
                }
            }
            else {
                    //cout << "data from connected client" << endl;
                    // handle data from connected clientfter registration is successful
            	if(m_bisRegistered) {
                    char recvBuff[1024] = {0};
                    int bytesRead;
                    if( (bytesRead = recv(i, recvBuff, sizeof(recvBuff), 0)) > 0) {
                        recvBuff[strlen(recvBuff)] = 0;
                        cout << "Remote Msg: " << recvBuff << endl;
                    }
            	}
            }
		}
	}
}

void Client::command_connect(char *ip, char *port) {
    struct sockaddr_in peerAddr;
    char peerIpAddr[32];
    int peerPort = atoi(port);
    int peerSd;
    cout << ip << " " << peerPort <<endl;
    strcpy(peerIpAddr, ip);
    
    memset(&peerAddr, 0, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(peerPort);
    
    if(inet_pton(AF_INET, peerIpAddr, &peerAddr.sin_addr) != 1) {
            perror("[command_register] inet_pton");
            displayUsage();
        }
        if((peerSd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Connect Error: socket");
            return;
        }
        if( connect(peerSd, (struct sockaddr *)&peerAddr, sizeof(peerAddr)) < 0) {
            perror("Connect Error: connect");
            return;
        }
        m_nClientSd = peerSd;
        FD_SET(m_nClientSd, &m_masterSet);
        if(m_nClientSd > m_nMaxFd)
            m_nMaxFd = m_nClientSd;
        startChat();
        close(m_nClientSd);
        cout << "connection established with client" << endl;
}

void Client::startChat() {
    cout << "starting chat" << endl;
    for(;;) {
        m_readSet = m_masterSet;
        if(select(m_nMaxFd+1, &m_readSet, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        int bytesRead;
        char recvBuff[1024];
        for(int i=0; i<=m_nMaxFd; i++) {
            if(FD_ISSET(i, &m_readSet)) {
                if(i == 0) {
                    string s;
                    getline(cin, s);
                    send(m_nClientSd,s.c_str(),s.length(),0);
                }
                else {
                    if( (bytesRead = recv(i, recvBuff, sizeof(recvBuff), 0)) > 0) {
                        recvBuff[strlen(recvBuff)] = 0;
                        cout << "Remote Msg: " << recvBuff << endl;
                    }
                    memset(recvBuff, 0, sizeof(recvBuff));
                }
            }
        }
    }
}

void Client::newConnectionHandler() {
    int newConnSd;
    char remoteIP[32];
    struct sockaddr_in remoteaddr;
    memset(&remoteaddr, 0, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    int addrlen = sizeof(remoteaddr);
    if((newConnSd = accept(m_nListenSd, (struct sockaddr*)&remoteaddr, (socklen_t*)&addrlen)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    m_nClientSd = newConnSd;
    FD_SET(m_nClientSd, &m_masterSet);
    if(m_nClientSd > m_nMaxFd)
        m_nMaxFd = m_nClientSd;
    printf("new connection from %s on ""socket %d\n", inet_ntop(AF_INET, &remoteaddr.sin_addr, remoteIP, 32), newConnSd);
    startChat();
}

int Client::command_register(char *ipAddr, char *port) {
	struct sockaddr_in srvAddr;
	int srvPort = atoi(port);
	strcpy(m_srvIpAddress, ipAddr);

	SSL_library_init();
	SSL_CTX *ctx = InitCTX();


	// populate server address structure and also check if it is a valid ip address
	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(srvPort);
	if(inet_pton(AF_INET, m_srvIpAddress, &srvAddr.sin_addr) != 1) {
		perror("[command_register] inet_pton");
		displayUsage();
	}

	if((m_nServerSd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return 0;
	}

	if( connect(m_nServerSd, (struct sockaddr *)&srvAddr, sizeof(srvAddr)) < 0)
	{
		perror("connect");
		return 0;
	}
    char buf[1024];
	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, m_nServerSd);
	if ( SSL_connect(ssl) == 0 )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {   char msg[] = "Hello???";
 
        //printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        //ShowCerts(ssl);        /* get any certs */
        SSL_write(ssl, msg, strlen(msg));   /* encrypt & send message */
        int bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
        buf[bytes] = 0;
        //printf("Received: \"%s\"\n", buf);
        SSL_free(ssl);        /* release connection state */
    }
    cout << "Authentication Successfull" << endl;
    m_bisRegistered = true;
    close(m_nServerSd);
    SSL_CTX_free(ctx);
}

void Client::command_msg(char *msg) {
    //char buff[1024] = {0};
    cout << msg << endl;
    int len = strlen(msg);
    if(sendall(m_nClientSd, msg, &len) == -1) {
        cout << "MSG send Error: Message sending failed. Please retry" << endl;
        return;
    }
}

int Client::sendall(int sockFd, char *buf, int *length)
{
    int btotal = 0;             // total number of bytes to send
    int bleft = *length;    // number of bytes remaining to be sent
    int n;
    while(btotal < *length)
    {
        n = send(sockFd, buf+btotal, bleft, 0);
        if (n == -1)
        {
            break;
        }
        btotal += n;
        bleft -= n;
    }
    *length = btotal; // actual number of bytes sent
    if(n == -1)
        return -1;  // failed
    return 0;   // success
}

CommandID Client::getCommandID(char comnd[]) {
    if(strcasecmp(comnd, "HELP") == 0)
        return COMMAND_HELP;
    else if(strcasecmp(comnd, "REGISTER") == 0)
    	return COMMAND_REGISTER;
    else if(strcasecmp(comnd, "CONNECT") == 0)
    	return COMMAND_CONNECT;
    else if(strcasecmp(comnd, "MSG") == 0)
        return COMMAND_MSG;
    else
        return COMMAND_NONE;
}


void Client::displayUsage() {
	printf("Please enter a valid command \n");
	printf("Type Help - to display supported commands \n");
}

int Client::getArgCount(char *line, const char *delim) {
        char *tok = strtok(line, delim);
        int nArgs = 0;
        while(tok!=NULL) {
             nArgs++;
             tok = strtok(NULL, delim);
        }
        return nArgs;
}

SSL_CTX* Client::InitCTX(void) {   
    const SSL_METHOD *method;
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = SSLv3_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

