#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    listen(server_fd, 5);

    printf("Server dang cho...\n");

    int client_sock = accept(server_fd, NULL, NULL);

    char buffer[BUF_SIZE];
    char prev[20] = ""; 
    char pattern[] = "0123456789";
    int total = 0;

    while (1) {
        int bytes = recv(client_sock, buffer, BUF_SIZE - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';

        char combined[BUF_SIZE + 20];
        strcpy(combined, prev);
        strcat(combined, buffer);

        for (int i = 0; i <= strlen(combined) - 10; i++) {
            if (strncmp(&combined[i], pattern, 10) == 0) {
                total++;
            }
        }

        printf("So lan xuat hien: %d\n", total);

        int len = strlen(combined);
        if (len >= 9) {
            strncpy(prev, combined + len - 9, 9);
            prev[9] = '\0';
        } else {
            strcpy(prev, combined);
        }
    }

    close(client_sock);
    close(server_fd);
    return 0;
}