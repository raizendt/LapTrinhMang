#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    char buffer[BUF_SIZE];

    while (1) {
        printf("Nhap: ");
        fgets(buffer, BUF_SIZE, stdin);

        sendto(sock, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server, len);

        int bytes = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                             NULL, NULL);

        buffer[bytes] = '\0';

        printf("Server echo: %s\n", buffer);
    }

    close(sock);
    return 0;
}