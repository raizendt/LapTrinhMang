#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    char id[50];
    int registered;
} Client;

Client clients[FD_SETSIZE];

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];

    int nfds = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);

    printf("Server dang chay tai port 5000...\n");

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        int activity = poll(fds, nfds, -1);

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {

                if (fds[i].fd == server_fd) {
                    client_fd = accept(server_fd, NULL, NULL);

                    printf("Client moi ket noi: %d\n", client_fd);

                    fds[nfds].fd = client_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                    clients[client_fd].fd = client_fd;
                    clients[client_fd].registered = 0;

                    send(client_fd, "Nhap: client_id: client_name\n", 35, 0);
                }

                else {
                    int fd = fds[i].fd;
                    char buffer[BUFFER_SIZE];
                    int n = recv(fd, buffer, sizeof(buffer)-1, 0);

                    if (n <= 0) {
                        printf("Client %d da ngat ket noi\n", fd);
                        close(fd);

                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                        continue;
                    }

                    buffer[n] = '\0';

                    if (!clients[fd].registered) {
                        char id[50], name[50];

                        if (sscanf(buffer, "%[^:]: %s", id, name) == 2) {
                            strcpy(clients[fd].id, id);
                            clients[fd].registered = 1;

                            send(fd, "Dang ky thanh cong\n", 20, 0);
                        } else {
                            send(fd, "Sai format. Nhap lai\n", 25, 0);
                        }
                    }

                    else {
                        char msg[1200];

                        time_t now = time(NULL);
                        struct tm *t = localtime(&now);
                        char time_str[64];
                        strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S", t);

                        sprintf(msg, "%s %s: %s", time_str, clients[fd].id, buffer);

                        for (int j = 0; j < nfds; j++) {
                            int other_fd = fds[j].fd;

                            if (other_fd != server_fd && other_fd != fd) {
                                send(other_fd, msg, strlen(msg), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}