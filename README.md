I have used openssl for authentication. For which we need libssl-dev package installed on the machine
First we need to generate a key for authentication using
 openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem
Then start the server program which is used for authentication.
    Command: ./app s <port>  // 's' here implies server
Then start two clients.
    Command: ./app c <port> // 'c' here implies client
The first message a client sends is a Register message
    Usage: register <ip> <server-port>

Then it should connect to a client using connect command
    Usage: connect <ip> <client-port>

Now both clients can chat by entering messages. 

