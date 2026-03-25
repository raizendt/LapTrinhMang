#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&server, sizeof(server));

    char buffer[BUF_SIZE];

    printf("UDP Echo Server dang chay...\n");

    while (1) {
        int bytes = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                             (struct sockaddr*)&client, &client_len);

        buffer[bytes] = '\0';

        printf("Nhan: %s\n", buffer);

        sendto(sock, buffer, bytes, 0,
               (struct sockaddr*)&client, client_len);
    }

    close(sock);
    return 0;
}