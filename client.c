#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <fcntl.h> // Include fcntl.h for non-blocking sockets
#include <errno.h> // Include errno.h for error handling

// Client port
#define PORT 8080

int packets = 0;
char GameOn = TRUE;

void sendAction(int socket, int action)
{
    // Validate the client control command before sending to server
    switch (action)
    {
    case 's':
    case 'd':
    case 'a':
    case 'w':
        // Valid keystroke: send to server, update packet count
        clear();
        send(socket, &action, sizeof(action), 0);
        printw("Packet %d transmitted\n", ++packets);
        break;
    case 'q':
        // Quit the game & terminate connection
        clear();
        close(socket);
        GameOn = FALSE;
        break;
    }
}

int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    int buffer;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        return -1;
    }

    // Set the socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    initscr(); // Initialize ncurses
    cbreak();  // Disable line buffering
    noecho();  // Don't echo characters entered by the user
    int c;

    while (GameOn)
    {
        if ((c = getch()) != ERR)
        {
            sendAction(sock, c);
        }

        // Try to receive data from server
        valread = recv(sock, &buffer, sizeof(buffer), 0);
        if (valread == 0)
        {
            printw("Server disconnected\n");
            break;
        }
        else if (valread < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // No data available yet
            }
            else
            {
                perror("recv failed");
                break;
            }
        }
        else
        {
            // Data received
        }
    }
    endwin();
    printf("Number of data packets transmitted: %d\n", packets);

    return 0;
}
