// Bai 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Kiểm tra tham số
    if (argc != 4) {
        printf("Usage: %s <PORT> <welcome_file> <output_file>\n", argv[0]);
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

    printf("Server dang chay tren port %s...\n", argv[1]);

    while (1) {
        // Accept client
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client da ket noi!\n");

        // 🔹 1. Gửi lời chào từ file
        FILE *wf = fopen(argv[2], "r");
        if (wf == NULL) {
            perror("Mo file welcome that bai");
        } else {
            while (fgets(buffer, BUFFER_SIZE, wf)) {
                send(client_fd, buffer, strlen(buffer), 0);
            }
            fclose(wf);
        }

        // 🔹 2. Nhận dữ liệu từ client (có thể nhiều lần)
        FILE *of = fopen(argv[3], "a");
        if (of == NULL) {
            perror("Mo file output that bai");
        } else {
            int len;
            while ((len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
                buffer[len] = '\0';
                fprintf(of, "%s", buffer);
            }
            fprintf(of, "\n"); // xuống dòng sau mỗi client
            fclose(of);
        }

        close(client_fd);
        printf("Client da ngat ket noi\n");
    }

    close(server_fd);
    return 0;
}
