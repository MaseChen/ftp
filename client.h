#ifndef CLIENT_H
#define CLIENT_H

/**
 * --- client.h defines ---
 * basic functions of client
 */

#include "base.h"
#include <features.h>

/**
 * get user name and password from stdin
 * return 0 if success or -1 if error
 */
int user_input_name_and_password(char *user_name, char *password);

/**
 * log in server using user_name and password
 * return 0 if success or -1 if error
 */
int login(int command_sockfd, const char *user_name, const char *password);

/**
 * receive a code from server
 * return 0 if success or -1 if error
 */
int recv_code(int command_sockfd, int *code);

/**
 * print a code with its meaning on stdout
 * return 0 if success or -1 if error
 */
int print_code(int code);

/**
 * get a command from stdin
 * return 0 if success or -1 if error
 */
int user_input_command(char *command);

/**
 * send the command to server
 * return 0 if success or -1 if error
 */
int send_command(int command_sockfd, const char *command);

/**
 * receive the current random port for receiving/sending data from server
 * return 0 if success or -1 if error
 */
int recv_data_port(int command_sockfd, int *data_port);

/**
 * receive a file from server via data sock fd
 * return 0 if success or -1 if error
 */
int recv_file(int data_sockfd, const char *command);

/**
 * send a file to server via data sock fd
 * return 0 if success or -1 if error
 */
int send_file(int data_sockfd, const char *command);

/**
 * receive the file list in server's work directory
 * return 0 if success or -1 if error
 */
int recv_list(int command_sockfd, int data_sockfd);

/**
 * read a piece of buffer in stdin
 * and change '\\n' and space in buffer to '\0'
 */
void read_input(char *buffer, int buf_size);

/**
 * set stdin flags to make the input texts hidden/visible
 */
int setlflag(int flags, int enable);

/**
 * command help will not be send to server,
 * the help infomation will print locally
 */
void print_help_information();

/**
 * function definitions
 * -----------------------------------------------------------------------
 */

int user_input_name_and_password(char *user_name, char *password)
{
    memset(user_name, 0, BUF_SIZE * sizeof(char));
    memset(password, 0, BUF_SIZE * sizeof(char));

    printf("user name: ");
    fflush(stdout);

    memcpy(user_name, CMD_ACCT, CMD_LEN);
    read_input(user_name + CMD_LEN, ARG_LEN);

    printf("password: ");
    fflush(stdout);

    setlflag(ECHO, 0);
    memcpy(password, CMD_ADAT, CMD_LEN);
    read_input(password + CMD_LEN, ARG_LEN);
    setlflag(ECHO, 1);

    putchar('\n');

    return 0;
}

int login(int command_sockfd, const char *user_name, const char *password)
{
    int result;

    result = send(command_sockfd, user_name, BUF_SIZE, MSG_WAITALL);

    if (result < 0)
    {
        perror("send() error");
        return -1;
    }

    result = send(command_sockfd, password, BUF_SIZE, MSG_WAITALL);

    if (result < 0)
    {
        perror("send() error");
        return -1;
    }

    return 0;
}

int recv_code(int command_sockfd, int *code)
{
    int result;
    int recv_code;

    result = recv(command_sockfd, &recv_code, sizeof(recv_code), MSG_WAITALL);

    if (result < 0)
    {
        perror("recv() error");
        return -1;
    }

    *code = ntohl(recv_code);

    return 0;
}

int print_code(int code)
{
    printf("%d: ", code);
    switch (code)
    {
    case 230:
        printf("User logged in, proceed.\n");
        printf("For FTP command help, use \"%s\"\n", CMD_HELP);
        break;
    case 430:
        printf("Invalid username or password.\n");
        return -1;

    case 120:
        printf("Service ready in a minute.\n");
        break;
    case 221:
        printf("Service closing control connection.\n");
        break;
    case 502:
        printf("Command not implemented.\n");
        break;

    case 125:
        printf("Data connection already open; transfer starting.\n");
        break;
    case 226:
        printf("Closing data connection. Requested file action successful.\n");
        break;
    default:
        error_handling("Invalid code.\n");
        return -1;
    }

    return 0;
}

int user_input_command(char *command)
{
    int i;

    printf("ftp> ");
    fflush(stdout);

    read_input(command, BUF_SIZE);

    for (i = 0; i < CMD_LEN; i++)
        command[i] = toupper(command[i]);

    if (0 == strncmp(command, CMD_LIST, CMD_LEN) ||
        0 == strncmp(command, CMD_HELP, CMD_LEN) ||
        0 == strncmp(command, CMD_QUIT, CMD_LEN) ||
        0 == strncmp(command, CMD_RETR, CMD_LEN) ||
        0 == strncmp(command, CMD_STOR, CMD_LEN) ||
        0 == strncmp(command, CMD_APPE, CMD_LEN) ||
        0 == strncmp(command, CMD_DELE, CMD_LEN) ||
        0 == strncmp(command, CMD_MKD, CMD_LEN) ||
        0 == strncmp(command, CMD_RMD, CMD_LEN) ||
        0 == strncmp(command, CMD_CWD, CMD_LEN))
        return 0;
    else
    {
        error_handling("invalid command");
        return -1;
    }
}

int send_command(int command_sockfd, const char *command)
{
    int result;

    result = send(command_sockfd, command, BUF_SIZE, MSG_WAITALL);

    if (result < 0)
    {
        perror("send() error");
        return -1;
    }

    return 0;
}

int recv_data_port(int command_sockfd, int *data_port)
{
    int recv_port;
    int result;

    result = recv(command_sockfd, &recv_port, sizeof(recv_port), MSG_WAITALL);

    if (result < 0)
    {
        perror("recv() error");
        return -1;
    }

    *data_port = ntohl(recv_port);

    return 0;
}

int recv_file(int data_sockfd, const char *command)
{
    char buffer[BUF_SIZE];
    int size;
    char arg[ARG_LEN];
    FILE *fd;

    int size_of_file = 0;

    memcpy(arg, command + CMD_LEN, ARG_LEN);
    fd = fopen(arg, "w");

    while ((size = recv(data_sockfd, buffer, BUF_SIZE, MSG_WAITALL)) > 0)
    {
        size_of_file += size;
        fwrite(buffer, 1, size, fd);
    }

    printf("received %d bytes of file\n", size_of_file);

    if (size < 0)
    {
        perror("recv_data() error");
        return -1;
    }

    fclose(fd);

    return 0;
}

int send_file(int data_sockfd, const char *command)
{
    char buffer[BUF_SIZE];
    char filename[ARG_LEN];

    int result;
    int size;
    FILE *fd;

    int size_of_file = 0;

    memcpy(filename, command + CMD_LEN, ARG_LEN - 1);
    fd = fopen(filename, "r");

    fseek(fd, SEEK_SET, 0);

    memset(buffer, 0, BUF_SIZE);
    while ((size = fread(buffer, 1, BUF_SIZE - 1, fd)) > 0)
    {
        size_of_file += size;
        buffer[size] = '\0';
        result = send(data_sockfd, buffer, size + 1, 0);
        if (result < 0)
        {
            perror("send() error");
            return -1;
        }

        memset(buffer, 0, BUF_SIZE);
    }

    printf("sent file: %d bytes\n", size_of_file);

    fclose(fd);

    return 0;
}

int recv_list(int command_sockfd, int data_sockfd)
{
    char buffer[BUF_SIZE];
    int size;

    int size_of_list = 0;

    printf("\nLIST: \n");

    memset(buffer, 0, sizeof(buffer));
    while ((size = recv(data_sockfd, buffer, BUF_SIZE, MSG_WAITALL)) > 0)
    {
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
        size_of_list += size;
    }

    printf("\nreceived %d bytes of list\n", size_of_list);

    if (size < 0)
    {
        perror("recv() error");
        return -1;
    }
    return 0;
}

void read_input(char *buffer, int buf_size)
{
    char *nl = NULL;
    memset(buffer, 0, buf_size);

    if (fgets(buffer, buf_size, stdin) != NULL)
    {
        nl = strchr(buffer, '\n');
        if (nl)
            *nl = '\0';

        nl = strchr(buffer, ' ');
        if (nl)
            *nl = '\0';
    }
}

int setlflag(int flags, int enable)
{
    struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    if (enable)
        attr.c_lflag |= flags;
    else
        attr.c_lflag &= ~flags;

    return tcsetattr(STDIN_FILENO, TCIFLUSH, &attr);
}

void print_help_information()
{
    printf("AVAILABLE COMMANDS:\n");
    printf("%-11s:\treturn the file list in work directory\n", CMD_LIST);
    printf("%s <file>:\treceive a file from server\n", CMD_RETR);
    printf("%s <file>:\tsend a file to server\n", CMD_STOR);
    printf("%s <path>:\tcreate file on server\n", CMD_APPE);
    printf("%s <path>:\tdelete file on server\n", CMD_DELE);
    printf("%s <path>:\tmake dir on server\n", CMD_MKD);
    printf("%s <path>:\tremove dir on server\n", CMD_RMD);
    printf("%s <path>:\tchange working dir\n", CMD_CWD);
    printf("%-11s:\tprint help information\n", CMD_HELP);
    printf("%-11s:\tclose the client\n", CMD_QUIT);
}

#endif