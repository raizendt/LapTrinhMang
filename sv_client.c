// Bai 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Kiểm tra tham số dòng lệnh
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket failed");
        return 1;
    }

    // Cấu hình server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    // Kết nối
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Da ket noi den sv_server!\n");

    // Nhập thông tin sinh viên
    char mssv[50], name[100], dob[50], gpa[10];

    printf("Nhap MSSV: ");
    fgets(mssv, sizeof(mssv), stdin);

    printf("Nhap ho ten: ");
    fgets(name, sizeof(name), stdin);

    printf("Nhap ngay sinh (YYYY-MM-DD): ");
    fgets(dob, sizeof(dob), stdin);

    printf("Nhap GPA: ");
    fgets(gpa, sizeof(gpa), stdin);

    // Xóa ký tự xuống dòng '\n'
    mssv[strcspn(mssv, "\n")] = 0;
    name[strcspn(name, "\n")] = 0;
    dob[strcspn(dob, "\n")] = 0;
    gpa[strcspn(gpa, "\n")] = 0;

    // Đóng gói dữ liệu (cách nhau bằng khoảng trắng)
    snprintf(buffer, BUFFER_SIZE, "%s %s %s %s", mssv, name, dob, gpa);

    // Gửi dữ liệu
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("Send failed");
    } else {
        printf("Da gui du lieu thanh cong!\n");
    }

    close(sock);
    return 0;
}