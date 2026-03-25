#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    // Get path
    char path[1024];
    getcwd(path, sizeof(path));

    int path_len = strlen(path);
    send(sock, &path_len, sizeof(int), 0);
    send(sock, path, path_len, 0);

    DIR *dir = opendir(".");
    struct dirent *entry;

    // Get and send file
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG)
            count++;
    }

    send(sock, &count, sizeof(int), 0);

    rewinddir(dir);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *name = entry->d_name;
            int name_len = strlen(name);

            struct stat st;
            stat(name, &st);

            send(sock, &name_len, sizeof(int), 0);
            send(sock, name, name_len, 0);
            send(sock, &st.st_size, sizeof(long), 0);
        }
    }

    closedir(dir);
    close(sock);
    return 0;
}