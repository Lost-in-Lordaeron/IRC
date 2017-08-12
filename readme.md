-------------------------------------------------------------------------------

Project Name: “Text ChatRoullette”

-------------------------------------------------------------------------------

Version 5.1
Release date: 02/25/2014

-------------------------------------------------------------------------------

Credits
	Yang Cao (ycao2@wpi.edu)
-------------------------------------------------------------------------------

Project description



Implement an IRC like chat server and a chat client that enables a client to 
chat 
with "random" people and supports multiple such sessions. 
The project has been 
completed in C and require the use of Unix Socket programming.

-------------------------------------------------------------------------------

Documentation



server:
server.h	server.c	makefile



Client:
Client.c	makefile

-------------------------------------------------------------------------------

How to compile Server:


1. Open the Command Line(SHELL or Terminal), and switch to this directory.

2. Type in "make all" to compile the exe file.

3. Type in "./Server" to run the this server.



How to run the Server:


1. Type in "START" to start the server as the admin.

2. Then all commands will be available for admin to use.

Admin command list:



command			notes

STATS			to show almost all information of current users

THROWOUT		to tear down a chatting channel.(by typing one user's socket_fd later)

BLOCK			to BLOCK an user from chatting.(by typing one's socket_fd later)

UNBLOCK			to UNBLOCK an user.(by typing one's socket_fd later)

END			to shut down the server gracefully.


-------------------------------------------------------------------------------

How to compile Client:


1. Open the Command Line(SHELL or Terminal), and switch to this directory

2. Type in "make all" to compile the exe file



How to run Client:

3. Open the exe file which has been compiled

4. Type CONNECT Command then type the hostname of the Server

5. When receive ACKN Command from the Server, type NICK Command then type nickname

6. When receive 'nickname has been set.' from the Server, type CHAT Command

7. When receive 'Waiting for a partner.' from the Server, wait for another one to chat
8. When another log in, the Client will receive 'Begin chatting with other’s nickname’

9. When type TRANSFER Command, the Client will type the file_path of file which the Client
 wants to send

10.When the Command lines show the “File: file_path Transfer Finished!”, 
it tells that
 the file has been sent to another one successfully

11.When type FLAG Command, the another one will be blocked by the Server

12.When type HELP Command, the Client will receive the information about HELP

13.When type QUIT Command, the Client will be put in the Connect Queue

14.When type HALT Command, the Client shut down

-------------------------------------------------------------------------------

Additional Notes



TRS port number: 8080

The most size of file that a Client tries to send: 100MB