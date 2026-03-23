#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Kiểm tra tham số
    if (argc != 3) {
        printf("Usage: %s <PORT> <log_file>\n", argv[0]);
        return 1;
    }

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Tạo socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        return 1;
    }

    // Cho phép reuse port (tránh lỗi Address already in use)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Cấu hình server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("sv_server dang chay tren port %s...\n", argv[1]);

    while (1) {
        // Accept client
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        // Nhận dữ liệu
        int len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (len <= 0) {
            close(client_fd);
            continue;
        }

        buffer[len] = '\0';

        // Lấy thời gian hiện tại
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

        // Lấy IP client
        char *client_ip = inet_ntoa(client_addr.sin_addr);

        // Tạo dòng log
        char log_line[BUFFER_SIZE];
        snprintf(log_line, BUFFER_SIZE, "%s %s %.900s", client_ip, time_str, buffer);
        
        // In ra màn hình
        printf("%s\n", log_line);

        // Ghi vào file
        FILE *f = fopen(argv[2], "a");
        if (f == NULL) {
            perror("Mo file log that bai");
        } else {
            fprintf(f, "%s\n", log_line);
            fclose(f);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}