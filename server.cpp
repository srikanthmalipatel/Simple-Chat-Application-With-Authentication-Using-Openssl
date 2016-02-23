#include "server.h"

Server::Server(int port) {
	cout << "Running program as server" << endl;
    this->m_nListenPort = port;

    // initalize server specific parameters
    FD_ZERO(&m_masterSet);
    FD_ZERO(&m_readSet);
   	
    // starting event handler
    eventHandler();
}

Server::~Server() {

}

void Server::startListener() {
		struct sockaddr_in m_srvAddr;
	// add stdin to master set
	    FD_SET(STDIN, &m_masterSet);

	    // populate server address structure
	    m_srvAddr.sin_family = AF_INET;
	    m_srvAddr.sin_port = htons(m_nListenPort);
	    m_srvAddr.sin_addr.s_addr = INADDR_ANY;
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
	    if(bind(m_nListenSd, (struct sockaddr *)&m_srvAddr, sizeof(m_srvAddr)) < 0) {
	        perror("bind failed");
	        exit(EXIT_FAILURE);
	    }
	    printf("Server is now listening on port: %d \n", m_nListenPort);
	    // maximum of 5 pending connections for m_nListenSd socket
	    if(listen(m_nListenSd, 5)) {
	        perror("Listen failed");
	        exit(EXIT_FAILURE);
	    }
	    // add listening socket to master set and update m_nMaxFd
	    FD_SET(m_nListenSd, &m_masterSet);
	    m_nMaxFd = m_nListenSd;
}

void Server::eventHandler() {
	char buffer[1024];
    SSL_library_init();
    SSL_CTX *ctx = InitServerCTX();
    LoadCertificates(ctx, "mycert.pem", "mycert.pem");
    startListener();
    while (1)
    {   struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;
 
        int client = accept(m_nListenSd, (struct sockaddr*)&addr, &len);  /* accept connection as usual */
        printf("Connection: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx);              /* get new SSL state with context */
        SSL_set_fd(ssl, client);      /* set connection socket to SSL state */
        Servlet(ssl);         /* service connection */
    }
    close(m_nListenSd);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
}

void Server::Servlet(SSL* ssl) /* Serve the connection -- threadable */
{   char buf[1024];
    char reply[1024];
    int sd, bytes;
    const char* HTMLecho="<html><body><pre>%s</pre></body></html>\n\n";
 
    if ( SSL_accept(ssl) == 0 )     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    else
    {
        //ShowCerts(ssl);        /* get any certificates */
        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
        if ( bytes > 0 )
        {
            buf[bytes] = 0;
            //printf("Client msg: \"%s\"\n", buf);
            sprintf(reply, HTMLecho, buf);   /* construct reply */
            SSL_write(ssl, reply, strlen(reply)); /* send reply */
        }
        else
            ERR_print_errors_fp(stderr);
    }
    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */
}

void Server::LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

SSL_CTX* Server::InitServerCTX(void)
{   const SSL_METHOD *method;
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv3_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}
