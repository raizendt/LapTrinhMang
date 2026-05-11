#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main()
{
    int sock;

    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    char request[] =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    send(sock, request, strlen(request), 0);

    char buffer[BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));

    recv(sock, buffer, sizeof(buffer), 0);

    printf("\n===== SERVER RESPONSE =====\n");
    printf("%s\n", buffer);

    close(sock);

    return 0;
}