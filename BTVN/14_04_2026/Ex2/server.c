#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    int state;
    char username[50];
} Client;

Client clients[FD_SETSIZE];

void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1]=='\n' || s[len-1]=='\r' || s[len-1]==' ' || s[len-1]=='\t')) {
        s[len-1] = '\0';
        len--;
    }
}

int check_login(char *user, char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;

    char u[50], p[50];
    while (fscanf(f, "%s %s", u, p) == 2) {
        if (strcmp(user, u) == 0 && strcmp(pass, p) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server running...\n");

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        poll(fds, nfds, -1);

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {

                if (fds[i].fd == server_fd) {
                    client_fd = accept(server_fd, NULL, NULL);

                    fds[nfds].fd = client_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                    clients[client_fd].fd = client_fd;
                    clients[client_fd].state = 0;

                    send(client_fd, "Username: ", 10, 0);
                } else {
                    int fd = fds[i].fd;
                    char buffer[BUFFER_SIZE];
                    int n = recv(fd, buffer, sizeof(buffer)-1, 0);

                    if (n <= 0) {
                        close(fd);
                        fds[i] = fds[nfds-1];
                        nfds--;
                        i--;
                        continue;
                    }

                    buffer[n] = '\0';

                    char *line = strtok(buffer, "\n");
                    while (line != NULL) {

                        trim(line);

                        if (clients[fd].state == 0) {
                            strcpy(clients[fd].username, line);
                            clients[fd].state = 1;
                            send(fd, "Password: ", 10, 0);
                        }
                        else if (clients[fd].state == 1) {
                            if (check_login(clients[fd].username, line)) {
                                clients[fd].state = 2;
                                send(fd, "Login success\n$ ", 17, 0);
                            } else {
                                send(fd, "Login failed\nUsername: ", 24, 0);
                                clients[fd].state = 0;
                            }
                        }
                        else {
                            if (strcmp(line, "exit") == 0) {
                                close(fd);
                                fds[i] = fds[nfds-1];
                                nfds--;
                                i--;
                                break;
                            }

                            char cmd[1200];
                            sprintf(cmd, "%s > out.txt 2>&1", line);
                            system(cmd);

                            FILE *f = fopen("out.txt", "r");
                            if (f) {
                                char out[1024];
                                while (fgets(out, sizeof(out), f)) {
                                    send(fd, out, strlen(out), 0);
                                }
                                fclose(f);
                            }

                            send(fd, "\n$ ", 3, 0);
                        }

                        line = strtok(NULL, "\n");
                    }
                }
            }
        }
    }

    return 0;
}