#ifndef BASE_H
#define BASE_H

/**
 * A ftp system based on C and Linux.
 * The whole program includes
 * --- code file ---
 * base.h,
 * server.h, client.h,
 * server.c, client.c,
 * Makefile,
 *
 * --- default work directory ---
 * DEFAULT_SERVER_WORK_DIR, DEFAULT_CLIENT_WORK_DIR
 *
 * --- binary file ---
 * server, client
 *
 * --- data file for holding user names and passwords ---
 * FILE_ACCOUNT
 *
 * A basic flow of ftp in Chinese:
 * 1. 服务器建立监听端口
 * 2. 客户端发送用户名和密码
 * 3. 服务器确认后返回成功信息
 * 4. 客户端发送请求
 * 5. 服务器返回状态码
 * 6. 服务器开放随机数据端口
 * 7. 服务器发送数据端口号
 * 8. 服务器发送数据
 * 9. 跳至4
 *
 * --- base.h defines ---
 * commands' information,
 * standard buffer and
 * basic functions based on socket.h
 *
 */

/* socket inclusion */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>

/**
 * A Standard Buffer
 * buffer: 128 chars with
 * command: 3 chars
 * argument: 125 chars
 */
#define BUF_SIZE 128
#define CMD_LEN 5
#define ARG_LEN BUF_SIZE - CMD_LEN

#define DEFAULT_SERVER_WORK_DIR "./ser" /* the default work directory of server */
#define DEFAULT_CLIENT_WORK_DIR "./cli" /* the default work directory of client */
#define FILE_ACCOUNT ".accounts"        /* the file manages user name and password */
#define ANONYMOUS "anonymous"           /* the user name of no password account */

/**
 * the commands user input and transfers in ftp
 * their size is defined by CMD_LEN
 */

#define CMD_ACCT "ACCT" /* 	Account information. */
#define CMD_ADAT "PAWD" /* Authentication/Security Data. */

#define CMD_LIST "LIST" /* Returns information of a file or directory. */
#define CMD_HELP "HELP" /* Returns a general help document. */
#define CMD_QUIT "QUIT" /* Disconnect. */

#define CMD_RETR "RETR" /* Retrieve a copy of the file. */
#define CMD_STOR "STOR" /* Accept the data and to store the data as a file at the server site. */

#define CMD_APPE "APPE" /* Append file(with create). */
#define CMD_DELE "DELE" /* Delete file. */

#define CMD_MKD "MKDR" /* Make directory. */
#define CMD_RMD "RMDR" /* Remove a directory. */
#define CMD_CWD "CWDR" /* Change working directory. */

/**
 * the status code server returns
 * size of status code is int
 *
 * 230  User logged in, proceed.
 * 430  Invalid username or password.
 *
 * 120  Service ready in a minute.
 * 221  Service closing control connection.
 * 502  Command not implemented.
 *
 * 125  Data connection already open; transfer starting.
 * 226  Closing data connection. Requested file action successful.
 */

/**
 * create a socket(), bind() it and listen() it using port in server
 * return the sock fd or -1 if error
 */
int server_socket_initialize(int port);

/**
 * accept a connection of client in listen_sockfd in server
 * return the new connected sock fd or -1 if error
 */
int server_socket_accept(int listen_sockfd);

/**
 * connect to a ftp server using IP address host and its port in client
 * return the connected sock fd or -1 if error
 */
int client_socket_connect(const char *host, int port);

/**
 * print message in stderr
 */
void error_handling(const char *message);

/**
 * function definitions
 * -------------------------------------------------------------------
 */

int server_socket_initialize(int port)
{
    int sockfd;
    int len;
    struct sockaddr_in address;

    int result;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket() error");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    len = sizeof(address);

    result = bind(sockfd, (struct sockaddr *)&address, len);

    if (result < 0)
    {
        close(sockfd);
        perror("bind() error");
        return -1;
    }

    result = listen(sockfd, 5);

    if (result < 0)
    {
        close(sockfd);
        perror("listen() error");
        return -1;
    }

    return sockfd;
}

int server_socket_accept(int listen_sockfd)
{
    int sockfd;
    struct sockaddr_in address;
    socklen_t len;

    len = sizeof(address);
    sockfd = accept(listen_sockfd, (struct sockaddr *)&address, &len);

    if (sockfd < 0)
    {
        perror("accept() error");
        return -1;
    }

    return sockfd;
}

int client_socket_connect(const char *host, int port)
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket() error");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(host);
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result < 0)
    {
        close(sockfd);
        perror("connect() error");
        return -1;
    }

    return sockfd;
}

void error_handling(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
}

#endif
