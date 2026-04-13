#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

string generateEmail(string fullname, string mssv) {
    vector<string> words;
    string word = "";

    for (char c : fullname) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word = "";
            }
        } else {
            word += tolower(c);
        }
    }
    if (!word.empty()) words.push_back(word);

    string lastName = words.back();
    string initials = "";

    for (int i = 0; i < words.size() - 1; i++) {
        initials += words[i][0];
    }

    return lastName + "." + initials + mssv + "@hust.edu.vn";
}

struct ClientState {
    string fullname;
    bool hasName = false;
};

int main() {
    int server_fd, new_socket, max_sd, activity;
    int client_socket[30];
    ClientState states[30];
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < 30; i++) {
        client_socket[i] = 0;
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    cout << "Server dang lang nghe tai port " << PORT << endl;

    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < 30; i++) {
            int sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            cout << "Client ket noi!" << endl;

            for (int i = 0; i < 30; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    states[i] = ClientState();
                    break;
                }
            }

            string msg = "Nhap ho ten:\n";
            send(new_socket, msg.c_str(), msg.size(), 0);
        }

        for (int i = 0; i < 30; i++) {
            int sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread <= 0) {
                    close(sd);
                    client_socket[i] = 0;
                    cout << "Client ngat ket noi\n";
                } else {
                    string input(buffer);

                    input.erase(input.find_last_not_of("\n") + 1);

                    if (!states[i].hasName) {
                        states[i].fullname = input;
                        states[i].hasName = true;

                        string ask = "Nhap MSSV:\n";
                        send(sd, ask.c_str(), ask.size(), 0);
                    } else {
                        string mssv = input;

                        string email = generateEmail(states[i].fullname, mssv);
                        email += "\n";

                        send(sd, email.c_str(), email.size(), 0);

                        states[i].hasName = false;
                    }
                }
            }
        }
    }

    return 0;
}