#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUFFER_SIZE 4096

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock,
                (struct sockaddr*)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Connect error");
        exit(1);
    }

    printf("Connected to server\n");

    while (1) {
        printf("telnet> ");

        fgets(buffer, sizeof(buffer), stdin);

        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        while (1) {
            memset(buffer, 0, sizeof(buffer));

            int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);

            if (bytes <= 0) {
                printf("Disconnected from server\n");
                close(sock);
                return 0;
            }

            buffer[bytes] = '\0';

            if (strstr(buffer, "END_OF_OUTPUT") != NULL) {
                break;
            }

            printf("%s", buffer);
        }
    }

    close(sock);

    return 0;
}