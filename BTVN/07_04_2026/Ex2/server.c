#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9090
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int sockfd;
    int step;
    char username[50];
} Client;

Client clients[MAX_CLIENTS];

int check_login(char *user, char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) {
        perror("Cannot open users.txt");
        return 0;
    }

    char u[50], p[50];
    while (fscanf(f, "%s %s", u, p) != EOF) {
        if (strcmp(u, user) == 0 && strcmp(p, pass) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void execute_command(int sock, char *cmd) {
    char syscmd[BUFFER_SIZE];
    snprintf(syscmd, sizeof(syscmd), "%s > out.txt", cmd);

    system(syscmd);

    FILE *f = fopen("out.txt", "r");
    if (!f) {
        send(sock, "Cannot execute\n", 15, 0);
        return;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), f)) {
        send(sock, line, strlen(line), 0);
    }

    fclose(f);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    fd_set readfds;
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sockfd = 0;
        clients[i].step = 0;
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

            printf("New client connected\n");

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].sockfd == 0) {
                    clients[i].sockfd = new_socket;
                    clients[i].step = 0;

                    send(new_socket, "Username: ", 10, 0);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].sockfd;

            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE - 1);

                if (valread < 0) {
                    continue;
                }

                if (valread == 0) {
                    close(sd);
                    clients[i].sockfd = 0;
                    printf("Client disconnected\n");
                    continue;
                }

                buffer[valread] = '\0';

                buffer[strcspn(buffer, "\r\n")] = '\0';

                if (strlen(buffer) == 0) {
                    continue;
                }

                printf("DEBUG step=%d | '%s'\n", clients[i].step, buffer);

                if (clients[i].step == 0) {
                    strcpy(clients[i].username, buffer);
                    clients[i].step = 1;
                    send(sd, "Password: ", 10, 0);
                }
                else if (clients[i].step == 1) {
                    if (check_login(clients[i].username, buffer)) {
                        clients[i].step = 2;
                        send(sd, "Login success!\n$ ", 18, 0);
                    } else {
                        send(sd, "Login failed!\nUsername: ", 25, 0);
                        clients[i].step = 0;
                    }
                }
                else {
                    if (strcmp(buffer, "exit") == 0) {
                        close(sd);
                        clients[i].sockfd = 0;
                        printf("Client exit\n");
                        continue;
                    }

                    execute_command(sd, buffer);
                    send(sd, "\n$ ", 3, 0);
                }
            }
        }
    }

    return 0;
}