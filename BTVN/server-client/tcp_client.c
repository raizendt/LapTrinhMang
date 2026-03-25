// Bai 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Da ket noi toi server %s:%s\n", argv[1], argv[2]);

    int len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Server: %s\n", buffer);
    }

    while (1) {
        printf("Nhap du lieu (type 'exit' de thoat): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}