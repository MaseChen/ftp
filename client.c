/**
 * the client implementation
 * with recusive error handling
 */

#include "client.h"

int main(int argc, char *argv[])
{
    int command_sockfd, data_sockfd;
    int cmd_port, data_port;

    char *host;
    int code;

    int result;

    char user_name[BUF_SIZE];
    char password[BUF_SIZE];
    char command[BUF_SIZE];

    if (argc != 3)
    {
        error_handling("usage: ./client hostname port\n");
        exit(1);
    }

    host = argv[1];
    cmd_port = atoi(argv[2]);

    command_sockfd = client_socket_connect(host, cmd_port);
    if (command_sockfd < 0)
    {
        error_handling("client_socket_connect() error");
        exit(1);
    }

    result = user_input_name_and_password(user_name, password);
    if (result < 0)
    {
        close(command_sockfd);
        error_handling("user_input_name_and_password() error");
        exit(1);
    }

    result = login(command_sockfd, user_name, password);
    if (result < 0)
    {
        close(command_sockfd);
        error_handling("login() error");
        exit(1);
    }

    result = recv_code(command_sockfd, &code);
    if (result < 0)
    {
        close(command_sockfd);
        error_handling("recv_code() error");
        exit(1);
    }

    result = print_code(code);
    if (result < 0)
    {
        close(command_sockfd);
        error_handling("print_code() error");
        exit(1);
    }

    result = chdir(DEFAULT_CLIENT_WORK_DIR);
    if (result < 0)
    {
        error_handling("The DEFAULT_CLIENT_WORK_DIR is unavailable");
        perror("chdir() error");
        return -1;
    }

    while (1)
    {
        result = user_input_command(command);
        if (result < 0)
        {
            continue;
        }

        if (0 == strncmp(command, CMD_HELP, ARG_LEN))
        {
            print_help_information();
            continue;
        }

        result = send_command(command_sockfd, command);
        if (result < 0)
        {
            close(command_sockfd);
            error_handling("send_command() error");
            exit(1);
        }

        result = recv_code(command_sockfd, &code);
        if (result < 0)
        {
            close(command_sockfd);
            error_handling("recv_code() error");
            exit(1);
        }

        result = print_code(code);
        if (result < 0)
        {
            close(command_sockfd);
            error_handling("print_code() error");
            exit(1);
        }

        switch (code)
        {
        case 120:
            if (0 == strncmp(command, CMD_LIST, CMD_LEN) ||
                0 == strncmp(command, CMD_RETR, CMD_LEN) ||
                0 == strncmp(command, CMD_STOR, CMD_LEN))
            {
                result = recv_data_port(command_sockfd, &data_port);
                if (result < 0)
                {
                    close(command_sockfd);
                    error_handling("recv_data_port() error");
                    exit(1);
                }

                data_sockfd = client_socket_connect(host, data_port);
                if (data_sockfd < 0)
                {
                    close(command_sockfd);
                    error_handling("client_socket_connect() error");
                    exit(1);
                }

                result = recv_code(command_sockfd, &code);
                if (result < 0)
                {
                    close(command_sockfd);
                    error_handling("recv_code() error");
                    exit(1);
                }

                result = print_code(code);
                if (result < 0)
                {
                    close(command_sockfd);
                    error_handling("print_code() error");
                    exit(1);
                }

                if (0 == strncmp(command, CMD_RETR, CMD_LEN))
                {
                    result = recv_file(data_sockfd, command);
                    if (result < 0)
                    {
                        close(command_sockfd);
                        close(data_sockfd);
                        error_handling("recv_file() error");
                        exit(1);
                    }
                }
                else if (0 == strncmp(command, CMD_STOR, CMD_LEN))
                {
                    result = send_file(data_sockfd, command);
                    if (result < 0)
                    {
                        close(command_sockfd);
                        close(data_sockfd);
                        error_handling("send_file() error");
                        exit(1);
                    }
                }
                else if (0 == strncmp(command, CMD_LIST, CMD_LEN))
                {
                    result = recv_list(command_sockfd, data_sockfd);
                    if (result < 0)
                    {
                        close(command_sockfd);
                        close(data_sockfd);
                        error_handling("recv_list() error");
                        exit(1);
                    }
                }

                close(data_sockfd);

                result = recv_code(command_sockfd, &code);
                if (result < 0)
                {
                    close(command_sockfd);
                    error_handling("recv_code() error");
                    exit(1);
                }

                result = print_code(code);
                if (result < 0)
                {
                    close(command_sockfd);
                    error_handling("print_code() error");
                    exit(1);
                }
            }

            break;
        case 221:
            goto break_2;
        case 502:
            break;
        default:
            break;
        }
    }

break_2:

    close(command_sockfd);

    exit(0);
}