#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 8192

double calculate(double a, double b, char *op)
{
    if(strcmp(op,"add")==0) return a+b;
    if(strcmp(op,"sub")==0) return a-b;
    if(strcmp(op,"mul")==0) return a*b;

    if(strcmp(op,"div")==0)
    {
        if(b==0) return 0;
        return a/b;
    }

    return 0;
}

void send_form(int client)
{
    char html[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"

    "<html>"
    "<body>"
    "<h2>Calculator</h2>"

    "<form method='POST' action='/calc'>"

    "A: <input name='a'><br><br>"
    "B: <input name='b'><br><br>"

    "<select name='op'>"
    "<option value='add'>+</option>"
    "<option value='sub'>-</option>"
    "<option value='mul'>*</option>"
    "<option value='div'>/</option>"
    "</select><br><br>"

    "<input type='submit'>"

    "</form>"
    "</body>"
    "</html>";

    send(client, html, strlen(html), 0);
}

void handle_client(int client)
{
    char buffer[BUFFER_SIZE];

    int n = recv(client, buffer, sizeof(buffer)-1, 0);

    if(n<=0)
        return;

    buffer[n]=0;

    if(strncmp(buffer,"GET / ",6)==0)
    {
        send_form(client);
        return;
    }

    if(strncmp(buffer,"GET /calc?",10)==0)
    {
        char op[20];
        double a,b;

        sscanf(buffer,
        "GET /calc?op=%19[^&]&a=%lf&b=%lf",
        op,&a,&b);

        double result = calculate(a,b,op);

        char html[1024];

        sprintf(html,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type:text/html\r\n\r\n"

        "<html><body>"
        "<h2>Result</h2>"
        "<p>%lf</p>"
        "</body></html>",

        result);

        send(client,html,strlen(html),0);
    }

    else if(strncmp(buffer,"POST /calc",10)==0)
    {
        char *body = strstr(buffer,"\r\n\r\n");

        if(body)
        {
            body += 4;

            double a,b;
            char op[20];

            sscanf(body,
            "a=%lf&b=%lf&op=%19s",
            &a,&b,op);

            double result = calculate(a,b,op);

            char html[1024];

            sprintf(html,

            "HTTP/1.1 200 OK\r\n"
            "Content-Type:text/html\r\n\r\n"

            "<html><body>"
            "<h2>Result</h2>"
            "<p>%lf</p>"
            "</body></html>",

            result);

            send(client,html,strlen(html),0);
        }
    }
}

int main()
{
    int server_fd, client_fd;

    struct sockaddr_in server_addr, client_addr;

    socklen_t len;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd,
        (struct sockaddr*)&server_addr,
        sizeof(server_addr));

    listen(server_fd,5);

    printf("Calculator HTTP Server running on %d\n",PORT);

    while(1)
    {
        len=sizeof(client_addr);

        client_fd=
        accept(server_fd,
              (struct sockaddr*)&client_addr,
              &len);

        handle_client(client_fd);

        close(client_fd);
    }

    close(server_fd);

    return 0;
}