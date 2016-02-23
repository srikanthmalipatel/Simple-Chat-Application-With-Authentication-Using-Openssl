#include "server.h"
#include "client.h"

using namespace std;

void printUsage(int argc, char* argv[]) {
    printf("Usage: ./%s <program type> <port> \n", argv[0]);
    printf("<program type>: 's' for server and 'c' for client \n");
    printf("<port>: listening port of server/client \n");
}

int main(int argc, char **argv ) {
	int port = atoi(argv[2]);

    // check the parameters passed to this program
    if(argc != 3) {
        printUsage(argc, argv);
        return -1;
    }
    // check if it client or server
    if(strcmp("s", argv[1]) == 0 && port > 1024) {
        // server initalization
        Server lServer(port);
    }
    else if(strcmp("c", argv[1]) == 0 && port > 1024) {
        // execute client code
        Client lClient(port);
    }
    else {
        printUsage(argc, argv);
        return -1;
    }
    // also do a valid port number check
    // check the program type and based on it initalize corresponding objects
    
    return 0;
	
}