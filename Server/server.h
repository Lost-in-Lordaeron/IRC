/*
** server.h -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PORT "8080"  // the port users will be connecting to
#define MAXDATASIZE 1024
#define BACKLOG 10     // how many pending connections queue will hold
#define STATS           0
#define THROWOUT        1
#define BLOCK           2
#define UNBLOCK         3
#define START           4
#define END             5

#define CONNECTED 		0
#define WAITING 		1
#define CHATTING 		2

#define TRUE	        1
#define FALSE			0

#define MAX_CLIENT		100


typedef struct user
{
	int sock_fd;
	char nickname[20];
	struct user* partner;
	long dataflow;
	char* status;
	int isBlocked;
}User,*USER;
// connected queue, stored sock_fds
// -1 stands for end of queue
USER connected[MAX_CLIENT] ;
int length_connected = 0;

// stored sock_fds that is waiting for chat
// -1 stands for end of queue
USER waiting[MAX_CLIENT] ;
int length_waiting = 0;

// stored chatting pairs, 
// the 2i'th is chatting with 2i+1'th 
// -1 stands for end of queue
USER chatting[MAX_CLIENT] ;
int pairs = 0;
int total_flaged = 0;
char* help_msg = "\ncommands		usage			notes\nCHAT			CHAT			begin chatting\nTRANSFER		TRANSFER		transfer file\nFLAG			FLAG			flag your partner\nNICK			NICK			set your nickname\nHELP			HELP			print help message\nQUIT			QUIT			quit to connected status\nHALT			HALT			get offline\n";
