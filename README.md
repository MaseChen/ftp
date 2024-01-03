# A Toy FTP Implementation

A ftp implementation based on Linux and C. For Computer Network Lab.

## Function

It is a client-server based software so the distribution includes client and server CLI program. Implementation of a client and a server similar with ftp protocol; completion of the basic function of file transfer.

1. List the files/directories
2. Change working-directory
3. Download file/s
4. Upload files/s
5. Create/remove files/directories
6. Etc…

### Commands

The following commands in [List of FTP commands - Wikipedia](https://en.wikipedia.org/wiki/List_of_FTP_commands) are implemented.

| Command | Description                                                         |
| ------- | ------------------------------------------------------------------- |
| LIST    | Returns information of a file or directory.                         |
| HELP    | Returns a general help document.                                    |
| QUIT    | Disconnect.                                                         |
| RETR    | Retrieve a copy of the file.                                        |
| STOR    | Accept the data and to store the data as a file at the server site. |
| APPE    | Append file(with create).                                           |
| DELE    | Delete file.                                                        |
| MKDR    | Make directory.                                                     |
| RMDR    | Remove a directory.                                                 |
| CWDR    | Change working directory.                                           |

### Server return codes

The following server return codes in [List of FTP server return codes - Wikipedia](https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes) are implemented.

| Code | Explanation                                                |
| ---- | ---------------------------------------------------------- |
| 230  | User logged in, proceed.                                   |
| 430  | Invalid username or password.                              |
| 120  | Service ready in a minute.                                 |
| 221  | Service closing control connection.                        |
| 502  | Command not implemented.                                   |
| 125  | Data connection already open; transfer starting.           |
| 226  | Closing data connection. Requested file action successful. |

## Compilation

Make sure you are in a **Linux** environment and a **gcc** compiler is available.

Clone the directory `ftp/` to your Linux and run the command in your shell.

```shell
$ make
```

Four files including `server`, `client`, `server.o` and `client.o` are in `ftp/` now. You can clean the files with `*.o` by this command if you like it.

```shell
$ make clean
```

The compilation is successful with my environment.

```shell
$ cat /proc/version
Linux version 5.15.133.1-microsoft-standard-WSL2 (gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.37)

$ cat /etc/issue
Ubuntu 22.04.3 LTS \n \l

$ gcc --version
gcc (Ubuntu 12.3.0-1ubuntu1~22.04) 12.3.0
Copyright (C) 2022 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Compilation error

If you are using a gcc with version lower than 8, maybe you should edit the `Makefile` because c17 is not available in gcc < 8.

**Before editing**

```makefile
# debug
# CFLAGS = -g -Wall -D_DEFAULT_SOURCE -std=c17

# release
CFLAGS = -O3 -Wall -D_DEFAULT_SOURCE -std=c17
```

**After editing**

```makefile
# debug
# CFLAGS = -g -Wall -D_DEFAULT_SOURCE

# release
CFLAGS = -O3 -Wall -D_DEFAULT_SOURCE
```

The program has not been tried to compile on other compilers such as clang. If you have questions or discovery on compilation, issues are welcome.

## Usage

Before you launch the two programs in shell, you should make two directories in `ftp/` first.

```shell
$ mkdir ser cli
```

These two directories are the default work directories of the two programs, which is defined in `base.h`

```c
#define DEFAULT_SERVER_WORK_DIR "./ser" /* the default work directory of server */
#define DEFAULT_CLIENT_WORK_DIR "./cli" /* the default work directory of client */
```

### Launch

**server**

```shell
$ ./server <port>
```

**client**

```shell
$ ./client <IP address> <port>
```

For example, if you are testing these programs on the same computer, you can use a command like this:

*Shell 1*

```shell
$ ./server 8980
```

*Shell 2*

```shell
./client 127.0.0.1 8980
```

### Login

The account infomation is saved in file `.accounts`, which includes user names and passwords for login.

```plaintext
mase helloworld
user pass
test testpass
anonymous 
```

A anonymous user can log in the server without password.

The account file name and the anonymous user name are defined in `base.h`

```c
#define FILE_ACCOUNT ".accounts"        /* the file manages user name and password */
#define ANONYMOUS "anonymous"           /* the user name of no password account */
```

## Design

The file structure, design ideas and code styles are inspired from [beckysag/ftp: Simple FTP client-server implementation in C (github.com)](https://github.com/beckysag/ftp). Simultaneously, [Siim/ftp: Lightweight FTP server written in C (github.com)](https://github.com/Siim/ftp) is also good to be learnt from.

* Reasonably split the functions and program main loops.
* Almost all the function calls of `socket.h` is concentrated to `base.h`
* Standard ANSI C code style
* Good comment style
* Recursivly return error messages from functions

## License

This software is under GPL 3.0, which means

* The source code cannot be closed after modification.
* New code needs to use the same license.

## Issues

All friendly issues are welcome.
