#ifndef BASE_H
#define BASE_H

#define STDIN 0

typedef enum {
    COMMAND_HELP=1,
	COMMAND_REGISTER,
	COMMAND_CONNECT,
	COMMAND_MSG,
    COMMAND_NONE,
}CommandID;

#endif
