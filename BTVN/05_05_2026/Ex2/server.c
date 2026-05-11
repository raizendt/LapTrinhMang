#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define WORKER_COUNT 5

int server_fd;

void handle_client(int client_sock)
{
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));

    int bytes = recv(client_sock, buffer, sizeof(buffer), 0);

    if (bytes <= 0)
    {
        close(client_sock);
        return;
    }

    printf("\n===== HTTP REQUEST =====\n");
    printf("%s\n", buffer);

    char response[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 95\r\n"
        "\r\n"
        "<html>"
        "<body>"
        "<h1>HTTP Preforking Server</h1>"
        "<p>Hello from worker process!</p>"
        "</body>"
        "</html>";

    send(client_sock, response, strlen(response), 0);

    close(client_sock);
}

void worker_process()
{
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_sock = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &client_len);

        if (client_sock < 0)
        {
            perror("accept");
            continue;
        }

        printf("Worker PID %d handling client\n", getpid());

        handle_client(client_sock);
    }
}

int main()
{
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("HTTP Preforking Server listening on port %d\n", PORT);

    for (int i = 0; i < WORKER_COUNT; i++)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            worker_process();
            exit(0);
        }
    }

    while (1)
    {
        pause();
    }

    close(server_fd);

    return 0;
}