#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> // Include fcntl.h for non-blocking sockets
#include <errno.h> // Include errno.h for error handling

typedef struct frame
{
    int sequence;
    int ack;
    char buffer;
} Frame;

#define PORT 8090

int main()
{
    struct sockaddr_in server_addr, client_addr;
    int server_fd, client_fd, len, n;
    Frame recv_frame, send_frame;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Filling server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Set flags to make socket non-blocking
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Bind the socket with the server address
    if (bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    len = sizeof(client_addr); // len is value/resuslt

    while (1)
    {
        n = recvfrom(server_fd, &recv_frame, sizeof(Frame), 0, (struct sockaddr *)&client_addr, &len);

        if (n < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // No data available yet, continue waiting
                // TODO: move down stuff
                continue;
            }
        }

        // Handle incoming frame
        printf("Received frame:\n");
        printf("Sequence: %d\n", recv_frame.sequence);
        printf("Ack: %d\n", recv_frame.ack);
        printf("Buffer: %c\n", recv_frame.buffer);

        send_frame.sequence = recv_frame.sequence + 1;
        send_frame.ack = 1;
        send_frame.buffer = recv_frame.buffer;

        // TODO: do stuff with tetris, input received
        sendto(server_fd, &send_frame, sizeof(Frame), 0, (const struct sockaddr *)&client_addr, len);
    }

    return 0;
}
