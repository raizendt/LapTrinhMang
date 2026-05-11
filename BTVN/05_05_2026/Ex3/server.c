#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 9000
#define BUFFER_SIZE 1024

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            printf("Client disconnected\n");
            break;
        }

        buffer[bytes] = '\0';

        printf("Received: %s\n", buffer);

        char command[100];
        char format[100];

        int n = sscanf(buffer, "%s %s", command, format);

        if (n != 2 || strcmp(command, "GET_TIME") != 0) {
            char *msg = "ERROR: Invalid command\n";
            send(client_sock, msg, strlen(msg), 0);
            continue;
        }

        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        char time_str[100];

        if (strcmp(format, "dd/mm/yyyy") == 0) {
            strftime(time_str, sizeof(time_str),
                     "%d/%m/%Y", t);

        } else if (strcmp(format, "dd/mm/yy") == 0) {
            strftime(time_str, sizeof(time_str),
                     "%d/%m/%y", t);

        } else if (strcmp(format, "mm/dd/yyyy") == 0) {
            strftime(time_str, sizeof(time_str),
                     "%m/%d/%Y", t);

        } else if (strcmp(format, "mm/dd/yy") == 0) {
            strftime(time_str, sizeof(time_str),
                     "%m/%d/%y", t);

        } else {
            char *msg = "ERROR: Unsupported format\n";
            send(client_sock, msg, strlen(msg), 0);
            continue;
        }

        send(client_sock, time_str, strlen(time_str), 0);
    }

    close(client_sock);
    exit(0);
}

int main() {
    int server_sock, client_sock;

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock < 0) {
        perror("Socket failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET,
               SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock,
             (struct sockaddr*)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Time Server running on port %d...\n", PORT);

    while (1) {

        while (waitpid(-1, NULL, WNOHANG) > 0);

        client_sock = accept(server_sock,
                             (struct sockaddr*)&client_addr,
                             &client_len);

        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected\n");

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            close(client_sock);

        } else if (pid == 0) {
            close(server_sock);

            handle_client(client_sock);

        } else {
            close(client_sock);
        }
    }

    close(server_sock);

    return 0;
}