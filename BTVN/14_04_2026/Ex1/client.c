#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Da ket noi server\n");

    struct pollfd fds[2];

    fds[0].fd = 0;
    fds[0].events = POLLIN;

    fds[1].fd = sock;
    fds[1].events = POLLIN;

    while (1) {
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN) {
            fgets(buffer, BUFFER_SIZE, stdin);
            send(sock, buffer, strlen(buffer), 0);
        }

        if (fds[1].revents & POLLIN) {
            int n = recv(sock, buffer, sizeof(buffer)-1, 0);

            if (n <= 0) {
                printf("Server da dong ket noi\n");
                break;
            }

            buffer[n] = '\0';
            printf("%s", buffer);
        }
    }

    close(sock);
    return 0;
}