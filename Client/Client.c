//
//  main.c
//  Client
//
//  Created by Di Wang on 2/4/14.
//  Copyright (c) 2014 Di Wang. All rights reserved.
//

/*
 ** client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "8080" // the port client will be connecting to

#define MAXDATASIZE 1024 // max number of bytes we can get at once
#define FILE_NAME_MAX_SIZE 1024 // max number of bytes in file_name
#define FILE_BUFFSIZE 1024

/* Definition of Command Call numbers                        */

#define         CONNECT                                0
#define         ACKN                                   1
#define         CHAT                                   2
#define         IN_SESSION                             3
#define         QUIT                                   4
#define         TRANSFER                               5
#define         FLAG                                   6
#define         HELP                                   7
#define         NICK                                   8
#define         HALT                                   9

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[BUFSIZ];
    char msg[MAXDATASIZE];
    char command[MAXDATASIZE];
    char buffer[BUFSIZ+1];
    char file_path[FILE_NAME_MAX_SIZE];
    char file_name[FILE_NAME_MAX_SIZE];
    char file_buffer[FILE_BUFFSIZE];
    char host_name[MAXDATASIZE];
    char nick_name[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    pid_t pid;
    int n,fd[2];
    int command_type;
    FILE *fp;
    int file_size;
    char file_size_str[FILE_NAME_MAX_SIZE];
    char file_head[FILE_NAME_MAX_SIZE];
    char ch;
    int flags;
    int i;
    struct hostent *he;
    struct in_addr **addr_list;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    gets(msg);//accept the massages from the keyboard
    while (strcmp(msg, "CONNECT") != 0) {
        printf("Please type CONNECT command!\n");
        gets(msg);
    }
    printf("Server Hostname: ");
    gets(host_name);
    
    if ((he = gethostbyname(host_name)) == NULL) {  // get the host info
        herror("gethostbyname");
        return 2;
    }
    addr_list = (struct in_addr **)he->h_addr_list;
    if ((rv = getaddrinfo(inet_ntoa(*addr_list[0]), PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (pipe(fd)<0) {
        printf("pipe failed!\n ");
        exit(1);
    }
    if ((pid=fork())<0) {
        printf("fork error");
    }else if (pid == 0){//child process
        flags = fcntl(fd[0],F_GETFL);
        if (fcntl(fd[0],F_SETFL,flags|O_NONBLOCK) == -1) {
            perror("fcntl");
            exit(1);
        }
        close(fd[1]);//shut down the writing terminal of pipe
        while (1) {
            read(fd[0],buffer,BUFSIZ);
            if (strcmp(buffer, "$HALT") == 0) {
                break;
            }
            bzero(buf, BUFSIZ);
            if ((numbytes = recv(sockfd, buf, BUFSIZ, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            
            if (strcmp(buf, "$TRANSFER") == 0){
                printf("Another client wants to transfer a file\n");
                
                if ((numbytes = recv(sockfd, buf, FILE_NAME_MAX_SIZE, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                buf[numbytes] = '\0';
                strcpy(file_head, buf);
                printf("file_head: '%s'\n",file_head);
                strcpy(file_size_str, strrchr(file_head, '+') + 1);
                printf("file_size_str: %s\n", file_size_str);
                file_size = atoi(file_size_str);
                char *connect = "+";
                strcpy(file_name,strtok(file_head, connect));
                fp = fopen(file_name,"wb+");
                if (fp == NULL) {
                    printf("\nerror on open %s file!\n", file_path);
                }else{
                    int length = 0;
                    int writelen = 0;
                    int total_file_size=0;
                    while((length=recv(sockfd,file_buffer,FILE_BUFFSIZE,0)))
                    {
                        if(length<0)
                        {
                            perror("recv");
                            exit(1);
                        }
                        printf("length: %d\n", length);
                        total_file_size = total_file_size + length;
                        writelen=fwrite(file_buffer,sizeof(char),length,fp);
                        bzero(file_buffer,FILE_BUFFSIZE);
                        if(total_file_size >= file_size)
                        {
                            break;
                        }
                    }
                    printf("Receieved file: %s finished!\n",file_name);
                    fclose(fp);
                }
                
            }else if(strcmp(buf, "$ACKN") == 0){//do nothing
                printf("Receive ACKN!\n");
            }else if(strcmp(buf, "$IN_SESSION") == 0){//do nothing
                printf("Receive IN_SESSION!\n");
            }else if(strcmp(buf, "$QUIT") == 0){
                printf("Receive QUIT!\n");
            }else if(strcmp(buf, "$HALT") == 0){
                printf("Receive HALT!\n");
                exit(1);
            }else{
                printf("Client: received '%s'\n",buf);
            }
        }
        
    }else{//parent process
        close(fd[0]);//shut down the reading terminal of pipe
        while (1) {
            command[0] = '$';
            command[1] = '\0';
            gets(msg);//accept the massages from the keyboard
            if (strcmp(msg, "CHAT") == 0){
                command_type = 2;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
            }else if (strcmp(msg, "QUIT") == 0){
                command_type = 4;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
            }else if (strcmp(msg, "TRANSFER") == 0){
                command_type = 5;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
            }else if (strcmp(msg, "FLAG") == 0){
                command_type = 6;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
            }else if (strcmp(msg, "HELP") == 0){
                command_type = 7;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
            }else if (strcmp(msg, "NICK") == 0){
                command_type = 8;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
                printf("Nickname: ");
                gets(nick_name);
                sprintf(msg, "%s %s", msg, nick_name);
            }else if (strcmp(msg, "HALT") == 0){
                command_type = 9;
                strcat(command, msg);
                strcpy(msg, command);
                command[0] = '$';
                command[1] = '\0';
            }else{
                command_type = -1;
            }
            //Send msg or command
            if (send(sockfd, msg, strlen(msg)+1, 0) == -1)
                perror("send");
//            printf("Client: sent '%s'\n",msg);
            switch (command_type) {
                case CHAT:
                    break;
                case QUIT:
                    break;
                case TRANSFER:
                    //Send file information
                    printf("file_path: ");
                    gets(file_path);
                    printf("file_path: %s\n", file_path);
                    strcpy(file_name, strrchr(file_path, '/') + 1);
                    printf("file_name: %s\n", file_name);
                    
                    if((fp=fopen(file_path,"rb")) == NULL){
                        printf("\nerror on open %s file!\n", file_path);
                    }else{
                        fseek(fp, 0, SEEK_END);
                        file_size = ftell(fp);
                        while (file_size > 104857600) {//if file size is more than 100M
                             printf("Please transfer the file whose size is less than 100M!\n");
                            //Send file information
                            printf("file_path: ");
                            gets(file_path);
                            printf("file_path: %s\n", file_path);
                            strcpy(file_name, strrchr(file_path, '/') + 1);
                            printf("file_name: %s\n", file_name);
                            if((fp=fopen(file_path,"rb")) == NULL){
                                printf("\nerror on open %s file!\n", file_path);
                            }else{
                                fseek(fp, 0, SEEK_END);
                                file_size = ftell(fp);
                            }
                        }
                        sprintf(file_size_str, "%d", file_size);
                        sprintf(file_head, "%s+%s", file_name, file_size_str);
                        printf("file_head: %s\n", file_head);
                        fclose(fp);
                        printf("FileSize: %d\n", file_size);
                        if (send(sockfd, file_head, FILE_NAME_MAX_SIZE, 0) < 0) {
                            printf("Send FileHead: %s Failed!\n", file_head);
                            break;
                        }
                        sleep(2);
                        if((fp=fopen(file_path,"rb")) == NULL){
                            printf("\nerror on open %s file!\n", file_path);
                        }else{
                            bzero(file_buffer,FILE_BUFFSIZE);
                            int file_block_length = 0;
                            while ((file_block_length = fread(file_buffer, sizeof(char), FILE_BUFFSIZE, fp)) > 0) {
                                printf("file_block_length = %d\n", file_block_length);
                                if (send(sockfd, file_buffer, file_block_length, 0) < 0) {
                                    printf("Send File: %s Failed!\n", file_path);
                                    break;
                                }
                                bzero(file_buffer,FILE_BUFFSIZE);
                            }
                            fclose(fp);
                            printf("File: %s Transfer Finished!\n", file_path);
                        }
                    }
                    break;
                case FLAG:
                    break;
                case HELP:
                    break;
                case NICK:
                    break;
                case HALT:
                    write(fd[1], msg, sizeof(msg));
                    close(sockfd);
                    exit(1);
                    break;
                default:
                    break;
            }
        }
    }
   	return 0;
}