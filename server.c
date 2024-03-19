#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h> // Include errno.h for error handling

//! Test server to make sure client works before adding into Tetris game

#define PORT 8080

void ActionRecv(char action)
{
    switch (action)
    {
    case 's':
        printf("s received\n");
        break;
    case 'd':
        printf("d received\n");
        break;
    case 'a':
        printf("a received\n");
        break;
    case 'w':
        printf("w received\n");
        break;
    }
}

int main()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer;
    int ack = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Set the accepted socket to non-blocking mode
    int flags = fcntl(new_socket, F_GETFL, 0);
    fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

    while (1)
    {
        valread = recv(new_socket, &buffer, sizeof(buffer), 0);
        if (valread == 0)
        {
            printf("Client disconnected\n");
            break;
        }
        else if (valread < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // No data available yet, continue waiting
                // TODO: move down stuff
                continue;
            }
            else
            {
                perror("recv failed");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // Data received
            ActionRecv(buffer);
            send(new_socket, &ack, sizeof(ack), 0); // Send ack to client
        }
    }

    return 0;
}
