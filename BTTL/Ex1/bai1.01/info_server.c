#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    listen(server_fd, 5);

    int client_sock = accept(server_fd, NULL, NULL);

    // Receive path
    int path_len;
    recv(client_sock, &path_len, sizeof(int), 0);

    char path[1024];
    recv(client_sock, path, path_len, 0);
    path[path_len] = '\0';

    printf("Thu muc: %s\n", path);

    // Receive file
    int count;
    recv(client_sock, &count, sizeof(int), 0);

    for (int i = 0; i < count; i++) {
        int name_len;
        recv(client_sock, &name_len, sizeof(int), 0);

        char name[256];
        recv(client_sock, name, name_len, 0);
        name[name_len] = '\0';

        long size;
        recv(client_sock, &size, sizeof(long), 0);

        printf("%s - %ld bytes\n", name, size);
    }

    close(client_sock);
    close(server_fd);
    return 0;
}