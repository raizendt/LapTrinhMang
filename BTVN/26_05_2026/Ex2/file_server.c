#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 8192

char *get_content_type(const char *filename)
{
    char *ext = strrchr(filename, '.');

    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".c") == 0) return "text/plain";
    if (strcmp(ext, ".cpp") == 0) return "text/plain";

    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";

    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";

    if (strcmp(ext, ".mp4") == 0) return "video/mp4";

    return "application/octet-stream";
}

void send_404(int client)
{
    const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body>"
        "<h1>404 Not Found</h1>"
        "</body></html>";

    send(client, response, strlen(response), 0);
}

void send_directory(int client, const char *path, const char *url_path)
{
    DIR *dir = opendir(path);

    if (!dir)
    {
        send_404(client);
        return;
    }

    char html[65536];

    strcpy(html,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Connection: close\r\n"
           "\r\n"
           "<html><body>"
           "<h2>Directory Listing</h2>");

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[2048];
        struct stat st;

        snprintf(fullpath,
                 sizeof(fullpath),
                 "%s/%s",
                 path,
                 entry->d_name);

        if (stat(fullpath, &st) != 0)
            continue;

        char line[2048];

        if (strcmp(url_path, "/") == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                snprintf(line,
                         sizeof(line),
                         "<b><a href='/%s'>%s</a></b><br>",
                         entry->d_name,
                         entry->d_name);
            }
            else
            {
                snprintf(line,
                         sizeof(line),
                         "<i><a href='/%s'>%s</a></i><br>",
                         entry->d_name,
                         entry->d_name);
            }
        }
        else
        {
            if (S_ISDIR(st.st_mode))
            {
                snprintf(line,
                         sizeof(line),
                         "<b><a href='%s/%s'>%s</a></b><br>",
                         url_path,
                         entry->d_name,
                         entry->d_name);
            }
            else
            {
                snprintf(line,
                         sizeof(line),
                         "<i><a href='%s/%s'>%s</a></i><br>",
                         url_path,
                         entry->d_name,
                         entry->d_name);
            }
        }

        if (strlen(html) + strlen(line) < sizeof(html))
            strcat(html, line);
    }

    strcat(html, "</body></html>");

    send(client, html, strlen(html), 0);

    closedir(dir);
}

void send_file(int client, const char *filepath, int is_head)
{
    FILE *fp = fopen(filepath, "rb");

    if (!fp)
    {
        send_404(client);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[1024];

    snprintf(header,
             sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Accept-Ranges: bytes\r\n"
             "Connection: close\r\n"
             "\r\n",
             get_content_type(filepath),
             filesize);

    send(client, header, strlen(header), 0);

    if (is_head)
    {
        fclose(fp);
        return;
    }

    char buffer[8192];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        send(client, buffer, bytes_read, 0);
    }

    fclose(fp);
}

void handle_client(int client)
{
    char request[BUFFER_SIZE];

    int n = recv(client,
                 request,
                 sizeof(request) - 1,
                 0);

    if (n <= 0)
        return;

    request[n] = '\0';

    char method[16];
    char path[1024];

    if (sscanf(request,
               "%15s %1023s",
               method,
               path) != 2)
    {
        return;
    }

    int is_head = 0;

    if (strcmp(method, "GET") == 0)
    {
        is_head = 0;
    }
    else if (strcmp(method, "HEAD") == 0)
    {
        is_head = 1;
    }
    else
    {
        return;
    }

    if (strstr(path, ".."))
    {
        send_404(client);
        return;
    }

    char fullpath[2048];

    snprintf(fullpath,
             sizeof(fullpath),
             ".%s",
             path);

    printf("\n========== REQUEST ==========\n");
    printf("Method : %s\n", method);
    printf("Path   : %s\n", path);
    printf("File   : %s\n", fullpath);
    printf("=============================\n");

    struct stat st;

    if (stat(fullpath, &st) != 0)
    {
        send_404(client);
        return;
    }

    if (S_ISDIR(st.st_mode))
    {
        send_directory(client, fullpath, path);
    }
    else
    {
        send_file(client, fullpath, is_head);
    }
}

int main()
{
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    int opt = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        return 1;
    }

    printf("HTTP File Server running on port %d\n", PORT);
    
    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
            continue;

        handle_client(client_fd);

        close(client_fd);
    }

    close(server_fd);

    return 0;
}