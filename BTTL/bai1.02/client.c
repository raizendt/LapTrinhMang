#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    char input[1024];

    while (1) {
        printf("Nhap du lieu: ");
        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = 0;

        send(sock, input, strlen(input), 0);
    }

    close(sock);
    return 0;
}