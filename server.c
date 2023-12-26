/**
 * the server implementation
 * with recusive error handling
 */

#include "server.h"

/**
 * open a new process to deal with a client's request
 */
int child_process(int command_sockfd);

int main(int argc, char *argv[])
{
    int cmd_listen_sockfd, command_sockfd;
    int port;
    int pid;
    int result;

    if (argc != 2)
    {
        error_handling("command port number should be the only argument");
        exit(1);
    }

    port = atoi(argv[1]);

    cmd_listen_sockfd = server_socket_initialize(port);
    if (cmd_listen_sockfd < 0)
    {
        error_handling("server_socket_initialize() error");
        exit(1);
    }

    while (1)
    {
        command_sockfd = server_socket_accept(cmd_listen_sockfd);
        if (command_sockfd < 0)
        {
            close(cmd_listen_sockfd);
            error_handling("server_socket_accept() error");
            exit(1);
        }

        pid = fork();

        if (0 == pid)
        {
            close(cmd_listen_sockfd);
            result = child_process(command_sockfd);
            close(command_sockfd);
            if (result < 0)
            {
                error_handling("child_process() error");
                exit(1);
            }

            exit(0);
        }
        else if (pid < 0)
        {
            perror("fork() error");
            exit(1);
        }

        close(command_sockfd);
    }

    close(cmd_listen_sockfd);
    exit(0);
}

int child_process(int command_sockfd)
{
    int data_listen_sockfd, data_sockfd;

    int data_port;
    int result;

    char user_name[BUF_SIZE];
    char password[BUF_SIZE];

    char command[BUF_SIZE];
    char cmd[CMD_LEN];
    char arg[ARG_LEN];

    result = login(command_sockfd, user_name, password);
    if (result < 0)
    {
        error_handling("login() error");
        return -1;
    }

    result = validate_user(user_name, password);

    if (result < 0)
    {
        result = send_code(command_sockfd, 430);
        if (result < 0)
        {
            error_handling("send_code() error");
            return -1;
        }
        error_handling("validate_user() error");
        return -1;
    }

    result = send_code(command_sockfd, 230);
    if (result < 0)
    {
        error_handling("send_code() error");
        return -1;
    }

    srand((unsigned)time(NULL));
    data_port = rand() % (DATA_PORT_CEIL - DATA_PORT_FLOOR) + DATA_PORT_FLOOR;

    result = chdir(DEFAULT_SERVER_WORK_DIR);
    if (result < 0)
    {
        error_handling("The DEFAULT_SERVER_WORK_DIR is unavailable");
        perror("chdir() error");
        return -1;
    }

    while (1)
    {
        result = recv_buffer(command_sockfd, command);
        if (result < 0)
        {
            error_handling("recv_buffer() error");
            return -1;
        }

        result = analyse_command(command, cmd, arg);
        if (result < 0)
        {
            error_handling("analyse_command() error");
            return -1;
        }

        if (0 == strcmp(cmd, CMD_LIST) ||
            0 == strcmp(cmd, CMD_RETR) ||
            0 == strcmp(cmd, CMD_STOR))
        {
            result = send_code(command_sockfd, 120);
            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }

            data_port = get_new_data_port(data_port);
            if (data_port < 0)
            {
                close(command_sockfd);
                error_handling("get_new_data_port() error");
                return -1;
            }

            data_listen_sockfd = server_socket_initialize(data_port);
            if (data_listen_sockfd < 0)
            {
                close(command_sockfd);
                error_handling("server_socket_initialize() error");
                return -1;
            }

            result = send_data_port(command_sockfd, data_port);
            if (result < 0)
            {
                close(data_listen_sockfd);
                close(command_sockfd);
                error_handling("send_data_port() error");
                return -1;
            }

            data_sockfd = server_socket_accept(data_listen_sockfd);
            if (data_sockfd < 0)
            {
                close(data_listen_sockfd);
                close(command_sockfd);
                error_handling("server_socket_accept() error");
                return -1;
            }

            result = send_code(command_sockfd, 125);
            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }

            if (0 == strcmp(cmd, CMD_RETR))
            {
                result = send_file(data_sockfd, arg);
                if (result < 0)
                {
                    close(data_sockfd);
                    close(data_listen_sockfd);
                    close(command_sockfd);
                    error_handling("send_file() error");
                    return -1;
                }
            }
            else if (0 == strcmp(cmd, CMD_STOR))
            {
                result = recv_file(data_sockfd, arg);
                if (result < 0)
                {
                    close(data_sockfd);
                    close(data_listen_sockfd);
                    close(command_sockfd);
                    error_handling("recv_file() error");
                    return -1;
                }
            }
            else if (0 == strcmp(cmd, CMD_LIST))
            {
                result = send_list(data_sockfd);
                if (result < 0)
                {
                    close(data_sockfd);
                    close(data_listen_sockfd);
                    close(command_sockfd);
                    error_handling("send_list() error");
                    return -1;
                }
            }

            close(data_sockfd);
            close(data_listen_sockfd);

            result = send_code(command_sockfd, 226);
            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
        else if (0 == strcmp(cmd, CMD_APPE))
        {
            result = create_file(arg);
            if (result < 0)
            {
                close(command_sockfd);
                error_handling("create_file() error");
                return -1;
            }

            if (0 == result)
                result = send_code(command_sockfd, 120);
            else
                result = send_code(command_sockfd, 502);

            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
        else if (0 == strcmp(cmd, CMD_DELE))
        {
            result = delete_file(arg);
            if (result < 0)
            {
                close(command_sockfd);
                error_handling("delete_file() error");
                return -1;
            }

            if (0 == result)
                result = send_code(command_sockfd, 120);
            else
                result = send_code(command_sockfd, 502);

            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
        else if (0 == strcmp(cmd, CMD_MKD))
        {
            result = make_directory(arg);
            if (result < 0)
            {
                close(command_sockfd);
                error_handling("make_directory() error");
                return -1;
            }

            if (0 == result)
                result = send_code(command_sockfd, 120);
            else
                result = send_code(command_sockfd, 502);

            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
        else if (0 == strcmp(cmd, CMD_RMD))
        {
            result = remove_directory(arg);
            if (result < 0)
            {
                close(command_sockfd);
                error_handling("remove_directory() error");
                return -1;
            }

            if (0 == result)
                result = send_code(command_sockfd, 120);
            else
                result = send_code(command_sockfd, 502);

            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
        else if (0 == strcmp(cmd, CMD_CWD))
        {
            result = change_work_directory(arg);
            if (result < 0)
            {
                close(command_sockfd);
                error_handling("change_work_directory() error");
                return -1;
            }

            if (0 == result)
                result = send_code(command_sockfd, 120);
            else
                result = send_code(command_sockfd, 502);

            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
        else if (0 == strcmp(cmd, CMD_QUIT))
        {
            result = send_code(command_sockfd, 221);
            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }

            break;
        }
        else
        {
            result = send_code(command_sockfd, 502);
            if (result < 0)
            {
                error_handling("send_code() error");
                return -1;
            }
        }
    }

    return 0;
}