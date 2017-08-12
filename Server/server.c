/*
** server.c -- a stream socket server demo
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
#include "server.h"





void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int substr(char*a, char*b)
{
    int i;
    int len;
    char s[6];
    if (strlen(a)<=6)
        return -1;
    strncpy(s,a,6);
    if (strcmp(s,"$NICK ")==0)
        {
            for (i = 6; i<=strlen(a);i++)
                b[i-6] = a[i];
            return (strlen(a)-6);
        }
        
    else return -1;
}
int guaranteed_send(int sock_fd, int size, char* buf)
{
    int total_sent = 0;
    int temp_sent = 0;
    char grt_buf[MAXDATASIZE];
    char temp_buf[MAXDATASIZE];
    strcpy(grt_buf,buf);
    while (1)
    {
        if ((temp_sent = send(sock_fd, grt_buf, strlen(grt_buf),0)) < 0) 
        {
            perror("send");
            //sleep(1);
        }
        else
        {
            if (temp_sent==0)
            {
                printf("sent is 0.\n");
                return 0;
            }

            total_sent += temp_sent;
            if(temp_sent<size)
            {
                printf("temp_sent: %d\n",temp_sent);
                printf("total_sent: %d\n",total_sent);
            }
            if(total_sent >= size)
                return 1;
            strcpy(temp_buf,grt_buf+temp_sent);
            strcpy(grt_buf, temp_buf);
            //sleep(0.5);
        }
    }


}
void terminate(USER user, USER partner)
{
    int i;
    char* s = "Your channel has been tore down by the server.\nYou are now in CONNECTED status.";
    if (send(user->sock_fd, s, strlen(s), 0) == -1)
    perror("send");
    if (send(partner->sock_fd, s, strlen(s), 0) == -1)
    perror("send");
    user->status = "CONNECTED";
    partner->status = "CONNECTED";
    while (i<pairs)
    {
        if ((chatting[2*i]->sock_fd != user->sock_fd)
            &&(chatting[2*i]->sock_fd != partner->sock_fd))
            i++;
        else break;
    }
    //printf("finish.\n");
    while (i<pairs)
    {
        chatting[2*i] = chatting[2*(i+1)];
        chatting[2*i+1] = chatting[2*(i+1)+1];
        i++;
    }
    pairs--;
    connected[length_connected] = user;
    length_connected++;
    connected[length_connected] = partner;
    length_connected++;
}
void kick(USER user)
{
    char buf[1025];
    int i=0;
    strcpy(buf,"$QUIT");
    if (send(user->sock_fd, buf, 1024, 0) == -1)
        perror("send");
    while (i<length_connected)
    {
        if (connected[i]->sock_fd != user->sock_fd)
            i++;
        else break;
    }
    while (i<length_connected)
    {
        connected[i] = connected[i+1];
    }
    length_connected--;
    close(user->sock_fd);
}
int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];

    int rv;
    int numbytes;  
    int pid;
    int fd1[2],fd2[2]; // pipe between ADMIN & process_2
    char buf[1025];
    char send_buf[1025];
    char cmd[20];
    int cmd_type;
    int len;
    int iterator;



    /**
              ------------fd1[]-------->
    ADMIN                                process_2 
         <-----------fd2[]---------         

    */
    if ((pipe(fd1)<0)|(pipe(fd2)<0))
    {
        printf("pipe creation failed.\n");
        return 0;
    }
    if ((pid = fork() )<0)
    {
        printf("fork fail.\n");
        return 0;
    }
    else if (pid > 0)
    {   //main process is the ADMIN process
        int n;
        char s[10];
        close (fd1[0]);
        close (fd2[1]);
        while (1)
        {
            strcpy(cmd,"\0");
            gets(cmd);

            if (strcmp(cmd,"STATS") == 0)
                cmd_type = 0;
            else if(strcmp(cmd,"THROWOUT")==0)
                cmd_type = 1;
            else if(strcmp(cmd,"BLOCK")==0)
                cmd_type = 2;
            else if(strcmp(cmd,"UNBLOCK")==0)
                cmd_type = 3;
            else if(strcmp(cmd,"START")==0)
                cmd_type = 4;
            else if(strcmp(cmd,"END")==0)
                cmd_type = 5;
            else
                cmd_type = 6;

            switch (cmd_type)
            {
                case STATS:
                    printf("command type: STATS.\n");
                    write(fd1[1],"STATS",15);
                    n = read(fd2[0],buf,100);
                    printf("command type: STATS. finished.\n");
                    break;
                case THROWOUT:
                    printf("command type: THROWOUT.\nPlease type in the sock_fd.\n");
                    gets(s);
                    strcat(cmd,s);
                    write(fd1[1],cmd,15);
                    n = read(fd2[0],buf,100);
                    printf("command type: THROWOUT. finished.\n");
                    break;
                case BLOCK:
                    printf("command type: BLOCK.\nPlease type in the sock_fd.\n");
                    gets(s);
                    strcat(cmd,s);
                    write(fd1[1],cmd,15);
                    n = read(fd2[0],buf,100);
                    break;
                case UNBLOCK:
                    printf("command type: UNBLOCK.\nPlease type in the sock_fd.\n");
                    gets(s);
                    strcat(cmd,s);
                    write(fd1[1],cmd,15);
                    n = read(fd2[0],buf,100);
                    break;
                case START:
                    printf("command type: START.\n");
                    write(fd1[1],"START",15);
                    break;
                case END:
                    printf("command type: END.\n");
                    strcpy(s,"..");
                    strcat(cmd,s);
                    write(fd1[1],cmd,15);
                    n = read(fd2[0],buf,100);
                    return 0;
                    break;
                default:
                    printf("command not recognized.\n");
                    break;
            }
        }

    }
    else 
    {       //process_2 response for transfer message 
            int n;
            char buf[1025];
            int flags;
            close (fd1[1]);
            close (fd2[0]);
            do
            {
                n =  read(fd1[0],buf,15);
                buf[n] = '\0';
            }while (strcmp("START",buf) != 0);

            printf("starting the server...\n");

            flags = fcntl(fd1[0], F_GETFL,0);
            fcntl(fd1[0], F_SETFL, flags | O_NONBLOCK);

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE; // use my IP

            if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                return 1;
            }

            // loop through all the results and bind to the first we can
            for(p = servinfo; p != NULL; p = p->ai_next) {
                if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
                    perror("server: socket");
                    continue;
                }

                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                        sizeof(int)) == -1) {
                    perror("setsockopt");
                    exit(1);
                }

                flags = fcntl(sockfd, F_GETFL,0);
                fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

                if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                    close(sockfd);
                    perror("server: bind");
                    continue;
                }

                break;
            }

            if (p == NULL)  {
                fprintf(stderr, "server: failed to bind\n");
                return 2;
            }

            freeaddrinfo(servinfo); // all done with this structure

            if (listen(sockfd, BACKLOG) == -1) {
                perror("listen");
                exit(1);
            }

            sa.sa_handler = sigchld_handler; // reap all dead processes
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = SA_RESTART;
            if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
            }

            printf("server: waiting for connections...\n");



            //printf("test message.\n");
            while(1) 
            {  // main accept() loop
                //check commands from admin
                int usr;
                n = read(fd1[0],cmd,15);
                if (n > -1)
                {
                    char cmd_tp[6];
                    buf[n] = '\0';
                    strncpy(cmd_tp, cmd,5);
                    cmd_tp[5]= '\0';
                    if (strcmp(cmd_tp,"STATS") == 0)
                    {
                        printf("Users in connected queue:\n");
                        printf("Total: %d.\n",length_connected);
                        for (usr = 0; usr< length_connected; usr++)
                            printf("socket: %d, nickname: %s, status: %s.\n",
                                    connected[usr]->sock_fd, connected[usr]->nickname, connected[usr]->status);
                        printf("Users in waiting queue:\n");
                        printf("Total: %d.\n",length_waiting);
                        for (usr = 0; usr<length_waiting; usr++)
                            printf("socket: %d, nickname: %s.\n",
                                    waiting[usr]->sock_fd, waiting[usr]->nickname);
                        printf("Users chatting:\n");
                        printf("Total: %d.\n",2*pairs);
                        for (usr = 0; usr<2*pairs; usr++)
                            printf("socket: %d, nickname: %s, dataflow: %ld.\n",
                                    chatting[usr]->sock_fd, chatting[usr]->nickname, chatting[usr]->dataflow);
                        printf("Users that are flaged by their partners:\n");
                        for (usr = 0; usr<length_connected; usr++)
                        {
                            if(connected[usr]->isBlocked== TRUE)
                            {
                                printf("socket: %d, nickname: %s, status: ",
                                    connected[usr]->sock_fd, connected[usr]->nickname);
                                if (strcmp(connected[usr]->status,"CONNECTED")==0)
                                    printf("CONNECTED.\n");
                                else if (strcmp(connected[usr]->status,"WAITING")==0)
                                    printf("WAITING.\n");
                                else if (strcmp(connected[usr]->status,"CHATTING")==0)
                                    printf("CHATTING.\n");
                            }
                        }
                        write(fd2[1],"done",15);
                    }

                    else if(strcmp(cmd_tp,"THROW")==0)
                    {
                        int sock,i,len;
                        char bf[5];
                        len = (int)strlen(cmd);
                        //printf("%s, %d\n",cmd,len);
                        for (i=8; i<strlen(cmd);i++)
                            bf[i-8] = cmd[i];
                        bf[strlen(cmd)-8] = '\0';
                        sock = (int)atoi(bf);
                        printf("sock_id: %d.\n",sock);
                        i=0;
                        while(i<pairs)
                        {
                            if ((chatting[2*i]->sock_fd != sock)
                                &&(chatting[2*i+1]->sock_fd != sock))
                                i++;
                            else 
                                break;
                        }
                        if (i>=pairs)
                            printf("sock_id not found.\n");
                        else
                            terminate(chatting[2*i],chatting[2*i+1]);
                        write(fd2[1],"done",15);
                    }

                    else if(strcmp(cmd_tp,"BLOCK")==0)
                    {
                        int sock,i,len;
                        char bf[5];
                        len = (int)strlen(cmd);
                        //printf("%s, %d\n",cmd,len);
                        for (i=5; i<strlen(cmd);i++)
                            bf[i-5] = cmd[i];
                        bf[strlen(cmd)-5] = '\0';
                        sock = (int)atoi(bf);
                        printf("sock_id: %d.\n",sock);
                        i=0;
                        while(i<length_connected)
                        {
                            if(connected[i]->sock_fd != sock)
                                i++;
                            else
                            {
                                connected[i]->isBlocked = TRUE;
                                if (send(connected[i]->sock_fd, "You are blocked by server.", 30, 0) == -1)
                                    perror("send");
                                break;
                            }   
                        }
                        i=0;
                        while(i<length_waiting)
                        {
                            if (waiting[i]->sock_fd !=sock)
                                i++;
                            else
                            {
                                waiting[i]->isBlocked = TRUE;
                                if (send(waiting[i]->sock_fd, "You are blocked by server.", 30, 0) == -1)
                                    perror("send");
                                break;
                            }
                        }
                        if(i<length_waiting)
                        {
                            length_waiting--;
                            connected[length_connected] = waiting[i];
                            connected[length_connected]->status = "CONNECTED";
                            length_connected++;
                        }
                            
                        while (i<length_waiting)
                            waiting[i] = waiting[i+1];
                        
                        waiting[length_waiting] = NULL;
                        i = 0;
                        while (i<(2*pairs))
                        {
                            if (chatting[i]->sock_fd != sock)
                                i++;
                            else
                            {
                                chatting[i]->isBlocked = TRUE;
                                if (send(chatting[i]->sock_fd, "You are blocked by server.", 30, 0) == -1)
                                    perror("send");
                                break;
                            }
                        }
                        write(fd2[1],"done",15);
                    }
                    else if(strcmp(cmd_tp,"UNBLO")==0)
                    {
                        int sock,i,len;
                        char bf[5];
                        len = (int)strlen(cmd);
                        //printf("%s, %d\n",cmd,len);
                        for (i=7; i<strlen(cmd);i++)
                            bf[i-7] = cmd[i];
                        bf[strlen(cmd)-7] = '\0';
                        sock = (int)atoi(bf);
                        printf("sock_id: %d.\n",sock);
                        i=0;
                        while(i<length_connected)
                        {
                            if(connected[i]->sock_fd != sock)
                                i++;
                            else
                            {
                                connected[i]->isBlocked = FALSE;
                                if (send(connected[i]->sock_fd, "You are unblocked by server.", 30, 0) == -1)
                                    perror("send");
                                break;
                            }   
                        }
                        i=0;
                        while (i<(2*pairs))
                        {
                            if (chatting[i]->sock_fd != sock)
                                i++;
                            else
                            {
                                chatting[i]->isBlocked = FALSE;
                                if (send(chatting[i]->sock_fd, "You are unblocked by server.", 30, 0) == -1)
                                    perror("send");
                                break;
                            }
                        }
                        write(fd2[1],"done",15);
                    }
                    else if(strcmp(cmd_tp,"END..")==0)
                    {
                        int i;
                        i = length_waiting-1;
                        //printf("entered end cmd.\n");
                        while (i>=0)
                        {
                            //printf("1.\n");
                            if (send(waiting[i]->sock_fd, "$HALT", 5, 0) == -1)
                                    perror("send");
                            close(waiting[i]->sock_fd);
                            waiting[i] = NULL;
                            i--;
                            length_waiting--;
                        }
                        i = pairs-1;
                        while(i>=0)
                        {
                            //printf("2.\n");
                            if (send(chatting[2*i]->sock_fd, "$HALT", 5, 0) == -1)
                                perror("send");
                            if (send(chatting[2*i+1]->sock_fd, "$HALT", 5, 0) == -1)
                                perror("send");
                            close(chatting[2*i]->sock_fd);
                            close(chatting[2*i+1]->sock_fd);
                            chatting[2*i]= NULL;
                            chatting[2*i+1]= NULL;
                            i--;
                            length_waiting--;
                        }
                        i = length_connected-1;
                        while (i>=0)
                        {
                            //printf("3.\n");
                            if (send(connected[i]->sock_fd, "$HALT", 5, 0) == -1)
                                    perror("send");
                            close(connected[i]->sock_fd);
                            connected[i] = NULL;
                            i--;
                            length_waiting--;
                        }
                        printf("Server is shutting down...\n");
                        write(fd2[1],"done",15);
                        
                    }
                }

                //printf("main_accept loop.\n");
                sin_size = sizeof their_addr;
                new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                if (new_fd != -1) 
                {
                    flags = fcntl(new_fd, F_GETFL,0);
                    fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

                    inet_ntop(their_addr.ss_family,
                    get_in_addr((struct sockaddr *)&their_addr),
                    s, sizeof s);
                    printf("server: got connection from %s\n", s);
                    if (send(new_fd, "$ACKN", 6, 0) == -1)
                        perror("send");

                    USER new_user = (USER) malloc(sizeof(User));
                    new_user->sock_fd = new_fd;
                    strcpy(new_user->nickname, "guest");
                    new_user->partner = NULL;
                    new_user->dataflow = 0;
                    new_user->status = "CONNECTED";
                    new_user->isBlocked = FALSE;


                    connected[length_connected] = new_user;
                    connected[length_connected+1] = NULL;
                    length_connected++;

                    
                }

            iterator = 0;
            while (iterator<length_connected)
            //for (iterator = 0; iterator< length_connected; iterator++)
            {

                int i;
                USER temp_user = NULL;
                USER partner = NULL;
                char nickname[20] = "";
                strcpy(buf,"");
                //printf("connect loop.\n");
                if ((numbytes = recv(connected[iterator]->sock_fd, buf, MAXDATASIZE, 0)) == -1) {
                        iterator++;
                    }
                else if (strcmp("$HALT",buf)==0)
                    {
                        int i = iterator;
                        temp_user = connected[iterator];
                        if (send(temp_user->sock_fd, "$HALT", 5, 0) == -1)
                            perror("send");
                        while(i<length_connected)
                        {
                            connected[i] = connected[i+1];
                            i++;
                        }
                        length_connected--;
                    }
                else if (substr(buf, nickname)!= -1)
                    {
                        strcpy(connected[iterator]->nickname,nickname);
                        if (send(connected[iterator]->sock_fd, "nickname has been set.", 40, 0) == -1)
                                    perror("send");
                        iterator++;
                    }
                else if (strcmp("$CHAT",buf)==0)
                    {
                        printf("client want to chat.\n");
                        temp_user = connected[iterator];
                        if(temp_user->isBlocked == TRUE)
                        {
                            if (send(temp_user->sock_fd, "Sorry you are blocked from chat.", 40, 0) == -1)
                                    perror("send");
                        }
                        else
                        {
                            for (i=iterator;i<length_connected;i++)
                            {
                                connected[i] = connected[i+1];
                            }
                            length_connected --;
                            connected[length_connected] = NULL;

                            if (length_waiting >0)
                            {
                                partner = waiting[0];
                                for(i = 0; i<length_waiting;i++)
                                {
                                    waiting[i] = waiting[i+1];
                                }
                                length_waiting -- ;
                                waiting[length_waiting] = NULL;
                                chatting[2*pairs] = temp_user;
                                chatting[2*pairs+1] = partner;
                                pairs++;
                                chatting[2*pairs]= NULL;
                                temp_user->status = "CHATTING";
                                partner->status = "CHATTING";
                                strcpy(buf,"$IN_SESSION");
                                if (send(temp_user->sock_fd, buf, strlen(buf), 0) == -1)
                                    perror("send");
                                if (send(partner->sock_fd, buf, strlen(buf), 0) == -1)
                                    perror("send");
                                sleep(0.2);
                                strcpy(buf,"Begin chatting with ");
                                strcat(buf,partner->nickname);
                                if (send(temp_user->sock_fd, buf, strlen(buf), 0) == -1)
                                    perror("send");
                                strcpy(buf,"Begin chatting with ");
                                strcat(buf,temp_user->nickname);
                                if (send(partner->sock_fd, buf, strlen(buf), 0) == -1)
                                    perror("send");

                            }
                            else if (length_waiting == 0)
                            {
                                waiting[0] = temp_user;
                                waiting[1] = NULL;
                                length_waiting ++;
                                temp_user->status = "WAITING";
                                if (send(temp_user->sock_fd, "Waiting for a partner.", 40, 0) == -1)
                                    perror("send");
                            }
                        }
                    }
                else if(strcmp("$HELP",buf)==0)
                    {
                        strcpy(buf,help_msg);
                        if (send(connected[iterator]->sock_fd, buf, strlen(buf), 0) == -1)
                            perror("send");
                        iterator++;
                    }
                else 
                    {
                        strcpy(buf,"");
                        iterator ++;
                    }
                    // buf[numbytes] = '\0';
                    // printf("server: received from client '%s'\n",buf);
                    
            }
            iterator = 0;
            while (iterator<length_waiting)
            {
                strcpy(buf,"");
                USER temp_user = waiting[iterator];
                if ((numbytes = recv(waiting[iterator]->sock_fd, buf, MAXDATASIZE, 0)) == -1) {
                        iterator++;
                    }
                else if (strcmp(buf,"$HELP")==0)
                {
                    strcpy(buf,help_msg);
                    if (send(temp_user->sock_fd, buf, strlen(buf), 0) == -1)
                        perror("send");
                }
                else if (strcmp(buf,"$QUIT")==0)
                {
                    int i;
                    strcpy(buf,"You are now in CONNECTED status.");
                    if (send(temp_user->sock_fd, buf, strlen(buf), 0) == -1)
                        perror("send");
                    connected[length_connected] = temp_user;
                    length_connected++;
                    connected[length_connected] = NULL;
                    temp_user->status="CONNECTED";
                    for (i = iterator; i<length_waiting;i++)
                    {
                        waiting[i] = waiting[i+1];
                    }
                    length_waiting--;
                }
                else if (strcmp(buf,"$HALT")==0)
                {
                    int i;
                    strcpy(buf,"$HALT");
                    if (send(temp_user->sock_fd, buf, strlen(buf), 0) == -1)
                        perror("send");
                    for (i = iterator; i<length_waiting;i++)
                    {
                        waiting[i] = waiting[i+1];
                    }
                    length_waiting--;
                }
                else 
                {
                    strcpy(buf,"");
                    iterator++;
                }
            }
            iterator = 0;
            while (iterator<(2*pairs))
            {
                int i;
                USER temp_user = NULL;
                USER partner = NULL;
                strcpy(buf,"");
                if ((numbytes = recv(chatting[iterator]->sock_fd, buf, MAXDATASIZE, 0)) == -1) {
                        iterator++;
                    }
                else 
                {
                    buf[numbytes] = '\0';
                    temp_user = chatting[iterator];
                    if (iterator%2 == 0)
                        partner = chatting[iterator+1];
                    else partner = chatting[iterator-1];

                    if(strcmp("$QUIT",buf)==0)
                    {
                        terminate(temp_user,partner);
                    }
                    else if(strcmp("$HALT",buf)==0)
                    {
                        int i=0;
                        terminate(temp_user,partner);
                        while(i<length_connected)
                        {
                            if(connected[i]->sock_fd != temp_user->sock_fd)
                                i++;
                            else
                            {
                                if (send(temp_user->sock_fd, "$HALT", 5, 0) == -1)
                                    perror("send");
                                close(temp_user->sock_fd);
                                break;
                            }
                        }
                        while(i<length_connected)
                        {
                            connected[i] = connected[i+1];
                            i++;
                        }
                        length_connected--;
                    }
                    else if(strcmp("$FLAG",buf)==0)
                    {
                        terminate(temp_user,partner);
                        partner->isBlocked = TRUE;
                        if (send(partner->sock_fd, "You are blocked by server.", 30, 0) == -1)
                            perror("send");
                        total_flaged++;
                    }
                    else if(strcmp("$HELP",buf)==0)
                    {
                        strcpy(buf,help_msg);
                        temp_user->dataflow += strlen(buf);
                        if (send(temp_user->sock_fd, buf, strlen(buf), 0) == -1)
                            perror("send");
                    }
                    else
                    {
                        char *s;
                        int n;
                        int flags;
                        //printf("%s\n",buf);
                        //printf("length = %d\n\n",strlen(buf));
                        temp_user->dataflow += strlen(buf);
                        partner->dataflow += strlen(buf);
                        // strcpy(send_buf, partner->nickname);
                        // strcat(send_buf,":");
                        // strcat(send_buf, buf);
                        s = buf;
                        flags = fcntl(partner->sock_fd, F_GETFL,0);
                        fcntl(partner->sock_fd, F_SETFL, flags & (~O_NONBLOCK));
                        //guaranteed_send(partner->sock_fd,strlen(buf),s);
                        if ((n=send(partner->sock_fd, buf, numbytes, 0)) == -1)
                            perror("send");
                        flags = fcntl(partner->sock_fd, F_GETFL,0);
                        fcntl(partner->sock_fd, F_SETFL, flags | O_NONBLOCK);
                        // printf("numbytes:%d\n",numbytes);
                        // printf("len_buf:%d\n",(strlen(buf)));
                        // printf("sent:%d\n",n);
                        // printf("%s\n\n",buf);
                        //printf("%d\n",strlen(buf));
                    }
                    iterator++;
                    

                }
            }

        }

    }
    return 0;
}
