#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000
#define BUFFER_SIZE 1024

int main() {

    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("Socket failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1",
              &server_addr.sin_addr);

    if (connect(sock,
                (struct sockaddr*)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Connect failed");
        exit(1);
    }

    printf("Connected to server\n");

    char buffer[BUFFER_SIZE];

    while (1) {

        printf("\nEnter command: ");
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0)
            break;

        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(sock, buffer,
                         sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            printf("Server disconnected\n");
            break;
        }

        buffer[bytes] = '\0';

        printf("Server response: %s\n", buffer);
    }

    close(sock);

    return 0;
}