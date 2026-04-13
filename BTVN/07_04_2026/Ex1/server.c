#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int sockfd;
    char id[50];
    int registered;
} Client;

Client clients[MAX_CLIENTS];

void broadcast(int sender, char *msg) {
    char buffer[BUFFER_SIZE];

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S", t);

    snprintf(buffer, sizeof(buffer), "%s %s: %s",
             time_str, clients[sender].id, msg);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i != sender && clients[i].sockfd != 0 && clients[i].registered) {
            send(clients[i].sockfd, buffer, strlen(buffer), 0);
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set readfds;

    char buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = 0;
        clients[i].registered = 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].sockfd;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd,
                                (struct sockaddr *)&address,
                                (socklen_t*)&addrlen);

            printf("New connection\n");

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].sockfd == 0) {
                    clients[i].sockfd = new_socket;
                    clients[i].registered = 0;

                    char *msg = "Enter: client_id: client_name\n";
                    send(new_socket, msg, strlen(msg), 0);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].sockfd;

            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread <= 0) {
                    close(sd);
                    clients[i].sockfd = 0;
                    printf("Client disconnected\n");
                } else {
                    buffer[valread] = '\0';

                    if (!clients[i].registered) {
                        char *colon = strchr(buffer, ':');
                        if (colon != NULL) {
                            *colon = '\0';
                            strcpy(clients[i].id, buffer);
                            clients[i].registered = 1;

                            send(sd, "Registered!\n", 12, 0);
                            printf("Client: %s registered\n", clients[i].id);
                        } else {
                            send(sd, "Wrong format!\n", 14, 0);
                        }
                    } else {
                        broadcast(i, buffer);
                    }
                }
            }
        }
    }

    return 0;
}