#ifndef SERVER_H
#define SERVER_H

/**
 * --- server.h defines ---
 * command port
 * data port range
 * basic functions of server
 */

#include "base.h"

/**
 * the lower bound of random data port
 */
#define DATA_PORT_FLOOR 8900

/**
 * the upper bound of random data port
 */
#define DATA_PORT_CEIL 8950

/**
 * receive the user name and password from client
 * return 0 if success or -1 if error
 */
int login(int sockfd, char *user_name, char *password);

/**
 * check the user's validity using user name and password
 * return 0 if success or -1 if error
 */
int validate_user(const char *user_name, const char *password);

/**
 * analyse the command and divide it to cmd section and arg section
 * return 0 if success or -1 if error
 */
int analyse_command(const char *command, char *cmd, char *arg);

/**
 * send a code to client via command sock fd
 * return 0 if success or -1 if error
 */
int send_code(int command_sockfd, int code);

/**
 * get a new random port that is not equal to the old one
 * return 0 if success or -1 if error
 */
int get_new_data_port(int old_data_port);

/**
 * send a data port to client via command sock fd
 * return 0 if success or -1 if error
 */
int send_data_port(int command_sockfd, int data_port);

/**
 * receive a standard buffer from client
 * return the number read or -1 if error
 */
int recv_buffer(int sockfd, char *buffer);

/**
 * send a file to client via data sock fd
 * return 0 if success or -1 if error
 */
int send_file(int data_sockfd, const char *filename);

/**
 * receive a file from client via data sock fd
 * return 0 if success or -1 if error
 */
int recv_file(int data_sockfd, const char *filename);

/**
 * send the file list in arg directory
 * return 0 if success or -1 if error
 */
int send_list(int data_sockfd);

/**
 * make a new file in current work directory
 * return 0 if success or -1 if error
 */
int create_file(char *name);

/**
 * remove a file in current work directory
 * return 0 if success or -1 if error
 */
int delete_file(char *name);

/**
 * make a new directory in current work directory
 * return 0 if success or -1 if error
 */
int make_directory(char *name);

/**
 * remove a directory in current work directory
 * return 0 if success or -1 if error
 */
int remove_directory(char *name);

/**
 * change the current server's work directory
 * return 0 if success or -1 if error
 */
int change_work_directory(char *path);

/**
 * change '\\n' and space in str to '\0'
 */
void handle_space(char *str, int n);

/**
 * functions definitions
 * --------------------------------------------------------------------------
 */

int login(int sockfd, char *user_name, char *password)
{
    int result;

    memset(user_name, 0, BUF_SIZE);
    result = recv_buffer(sockfd, user_name);

    if (result < 0)
    {
        error_handling("recv_buffer() error");
        return -1;
    }

    memset(password, 0, BUF_SIZE);
    result = recv_buffer(sockfd, password);
    if (result < 0)
    {
        error_handling("recv_buffer() error");
        return -1;
    }

    return 0;
}

int validate_user(const char *user_name, const char *password)
{
    char uname[BUF_SIZE];
    char pword[BUF_SIZE];

    char buffer[BUF_SIZE];

    char *pch;

    FILE *fd;

    fd = NULL;
    fd = fopen(FILE_ACCOUNT, "r");

    if (!fd)
    {
        perror("account file not found");
        return -1;
    }

    memset(buffer, 0, BUF_SIZE);

    while (fgets(buffer, BUF_SIZE, fd))
    {
        pch = strtok(buffer, " ");
        strcpy(uname, pch);

        if (pch != NULL)
        {
            pch = strtok(NULL, " ");
            strcpy(pword, pch);
        }

        handle_space(pword, strlen(pword)); /* remove end of line and whitespace */

        if ((0 == strcmp(user_name + CMD_LEN, uname)) && (0 == strcmp(password + CMD_LEN, pword)))
        {
            fclose(fd);

            printf("user %s login succeed.\n", uname);

            return 0;
        }
        else if (0 == strcmp(user_name + CMD_LEN, ANONYMOUS))
        {
            fclose(fd);

            printf("user %s login succeed.\n", ANONYMOUS);

            return 0;
        }
    }
    fclose(fd);

    return -1;
}

int analyse_command(const char *command, char *cmd, char *arg)
{
    memcpy(cmd, command, CMD_LEN);
    memcpy(arg, command + CMD_LEN, ARG_LEN);

    return 0;
}

int send_code(int command_sockfd, int code)
{
    int result;
    int temp;

    temp = htonl(code);

    result = send(command_sockfd, &temp, sizeof(temp), MSG_WAITALL);

    if (result < 0)
    {
        perror("send() error");
        return -1;
    }

    printf("code %d sent.\n", code);

    return 0;
}

int get_new_data_port(int old_data_port)
{
    int data_port_range;
    int data_port_offset;
    int new_data_port;

    data_port_range = DATA_PORT_CEIL - DATA_PORT_FLOOR;
    data_port_offset = (old_data_port - DATA_PORT_FLOOR + 1) % data_port_range;
    new_data_port = DATA_PORT_FLOOR + data_port_offset;

    return new_data_port;
}

int send_data_port(int command_sockfd, int data_port)
{
    int result;
    int temp;

    temp = htonl(data_port);

    result = send(command_sockfd, &temp, sizeof(temp), MSG_WAITALL);

    if (result < 0)
    {
        perror("send() error");
        return -1;
    }

    printf("data port %d sent.\n", data_port);

    return 0;
}

int recv_buffer(int sockfd, char *buffer)
{
    int size;

    memset(buffer, 0, BUF_SIZE);
    size = recv(sockfd, buffer, BUF_SIZE, MSG_WAITALL);
    if (size < 0)
    {
        perror("recv() error");
        return -1;
    }

    return size;
}

int send_file(int data_sockfd, const char *filename)
{
    char buffer[BUF_SIZE];
    int result;
    int size;
    FILE *fd;

    fd = fopen(filename, "r");

    fseek(fd, SEEK_SET, 0);

    memset(buffer, 0, BUF_SIZE);
    while ((size = fread(buffer, 1, BUF_SIZE - 1, fd)) > 0)
    {
        buffer[size] = '\0';
        result = send(data_sockfd, buffer, size + 1, 0);
        if (result < 0)
        {
            perror("send() error");
            return -1;
        }

        memset(buffer, 0, BUF_SIZE);
    }

    fclose(fd);

    printf("file %s sent.\n", filename);

    return 0;
}

int recv_file(int data_sockfd, const char *filename)
{
    char buffer[BUF_SIZE];
    int size;
    FILE *fd;

    fd = fopen(filename, "w");

    while ((size = recv_buffer(data_sockfd, buffer)) > 0)
    {
        fwrite(buffer, 1, size, fd);
    }

    if (size < 0)
    {
        perror("recv_data() error");
        return -1;
    }

    fclose(fd);

    printf("file %s received.\n", filename);

    return 0;
}

int send_list(int data_sockfd)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    char buffer[BUF_SIZE];
    int size;
    int result;

    FILE *fd;

    dp = opendir(".");

    if (!dp)
    {
        perror("opendir() error");
        return -1;
    }

    fd = tmpfile();

    if (!fd)
    {
        perror("tmpfile() error");
        return -1;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            if (0 == strncmp(".", entry->d_name, 1))
                continue;
            fprintf(fd, "%s/\n", entry->d_name);
        }
        else
        {
            if (0 == strncmp(".", entry->d_name, 1))
                continue;
            fprintf(fd, "%s\n", entry->d_name);
        }
    }

    closedir(dp);

    fseek(fd, SEEK_SET, 0);

    memset(buffer, 0, BUF_SIZE);
    while ((size = fread(buffer, 1, BUF_SIZE - 1, fd)) > 0)
    {
        buffer[size] = '\0';
        result = send(data_sockfd, buffer, size + 1, 0);
        if (result < 0)
        {
            perror("send() error");
            return -1;
        }

        memset(buffer, 0, BUF_SIZE);
    }

    fclose(fd);

    printf("list sent.\n");

    return 0;
}

int create_file(char *name)
{
    int result;

    handle_space(name, ARG_LEN);

    result = creat(name, 0755);

    if (result < 0)
    {
        perror("creat() error");
        return -1;
    }

    printf("file %s created.\n", name);

    return 0;
}

int delete_file(char *name)
{
    int result;

    handle_space(name, ARG_LEN);

    if (0 == strncmp(name, ".", 1))
    {
        return 1;
    }

    result = unlink(name);

    if (result < 0)
    {
        perror("unlink() error");
        return -1;
    }

    printf("file %s deleted.\n", name);

    return 0;
}

int make_directory(char *name)
{
    int result;

    handle_space(name, ARG_LEN);

    result = mkdir(name, 0755);

    if (result < 0)
    {
        perror("mkdir() error");
        return -1;
    }

    printf("directory %s made.\n", name);

    return 0;
}

int remove_directory(char *name)
{
    int result;

    handle_space(name, ARG_LEN);

    if (0 == strncmp(name, ".", 1))
    {
        return 1;
    }

    result = rmdir(name);

    if (result < 0)
    {
        perror("rmdir() error");
        return -1;
    }

    printf("directory %s removed.\n", name);

    return 0;
}

int change_work_directory(char *path)
{
    int result;
    char *result_p;
    char buffer[BUF_SIZE];

    result = chdir(path);

    if (result < 0)
    {
        perror("chdir() error");
        return -1;
    }

    memset(buffer, 0, BUF_SIZE);
    result_p = getcwd(buffer, BUF_SIZE);

    if (!result_p)
    {
        perror("getcwd() error");
        return -1;
    }

    printf("work directory changed to %s.\n", buffer);

    return 0;
}

void handle_space(char *str, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (isspace(str[i]))
            str[i] = 0;
        if ('\n' == str[i])
            str[i] = 0;
    }
}

#endif