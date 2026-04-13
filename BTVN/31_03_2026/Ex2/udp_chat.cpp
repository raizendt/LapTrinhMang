#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cout << "Usage: ./udp_chat <port_s> <ip_d> <port_d>\n";
        return 1;
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    int sockfd;
    struct sockaddr_in my_addr, dest_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port_s);

    bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port_d);
    inet_pton(AF_INET, ip_d, &dest_addr.sin_addr);

    cout << "UDP Chat started on port " << port_s << endl;

    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);

        FD_SET(sockfd, &readfds);

        FD_SET(STDIN_FILENO, &readfds);

        int max_fd = sockfd;

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);

            struct sockaddr_in sender_addr;
            socklen_t addr_len = sizeof(sender_addr);

            int len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                               (struct sockaddr*)&sender_addr, &addr_len);

            if (len > 0) {
                cout << "\n[Received]: " << buffer << endl;
                cout << "You: ";
                fflush(stdout);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            string msg;
            getline(cin, msg);

            sendto(sockfd, msg.c_str(), msg.size(), 0,
                   (struct sockaddr*)&dest_addr, sizeof(dest_addr));

            cout << "You: ";
        }
    }

    close(sockfd);
    return 0;
}