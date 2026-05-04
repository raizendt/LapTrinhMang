#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9000
#define MAX_CLIENTS 100
#define MAX_TOPICS 100
#define BUFFER_SIZE 1024

typedef struct {
    char topic[50];
    int clients[MAX_CLIENTS];
    int count;
} Topic;

Topic topics[MAX_TOPICS];
int topic_count = 0;

int clients[MAX_CLIENTS];
int client_count = 0;

int find_topic(char *name) {
    for (int i = 0; i < topic_count; i++) {
        if (strcmp(topics[i].topic, name) == 0)
            return i;
    }
    return -1;
}

int get_or_create_topic(char *name) {
    int idx = find_topic(name);
    if (idx != -1) return idx;

    strcpy(topics[topic_count].topic, name);
    topics[topic_count].count = 0;
    return topic_count++;
}

void subscribe(int client_fd, char *topic) {
    int idx = get_or_create_topic(topic);

    for (int i = 0; i < topics[idx].count; i++) {
        if (topics[idx].clients[i] == client_fd)
            return;
    }

    topics[idx].clients[topics[idx].count++] = client_fd;
    printf("Client %d SUB %s\n", client_fd, topic);
}

void unsubscribe(int client_fd, char *topic) {
    int idx = find_topic(topic);
    if (idx == -1) return;

    for (int i = 0; i < topics[idx].count; i++) {
        if (topics[idx].clients[i] == client_fd) {
            topics[idx].clients[i] = topics[idx].clients[--topics[idx].count];
            break;
        }
    }

    printf("Client %d UNSUB %s\n", client_fd, topic);
}


void publish(char *topic, char *msg) {
    int idx = find_topic(topic);
    if (idx == -1) return;

    for (int i = 0; i < topics[idx].count; i++) {
        int fd = topics[idx].clients[i];
        send(fd, msg, strlen(msg), 0);
    }

    printf("PUB %s: %s\n", topic, msg);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    fd_set readfds;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        int max_fd = server_fd;

        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i], &readfds);
            if (clients[i] > max_fd) max_fd = clients[i];
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            clients[client_count++] = new_socket;
            printf("New client: %d\n", new_socket);
        }

        for (int i = 0; i < client_count; i++) {
            int fd = clients[i];

            if (FD_ISSET(fd, &readfds)) {
                char buffer[BUFFER_SIZE] = {0};
                int valread = read(fd, buffer, BUFFER_SIZE);

                if (valread <= 0) {
                    close(fd);
                    clients[i] = clients[--client_count];
                    i--;
                    continue;
                }

                char cmd[10], topic[50], msg[900];

                if (sscanf(buffer, "SUB %s", topic) == 1) {
                    subscribe(fd, topic);
                }
                else if (sscanf(buffer, "UNSUB %s", topic) == 1) {
                    unsubscribe(fd, topic);
                }
                else if (sscanf(buffer, "PUB %s %[^\n]", topic, msg) == 2) {
                    publish(topic, msg);
                }
            }
        }
    }

    return 0;
}