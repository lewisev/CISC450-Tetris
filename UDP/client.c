#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <sys/time.h> // Needed for timeval struct
#include <time.h>     // time() for random seed

typedef struct frame
{
    int sequence;
    int ack;
    char buffer;
} Frame;

// Configure sockets and loss ratio
#define PORT 8090
#define SERVER_IP "127.0.0.1"
#define PACKET_LOSS_RATIO 0.3

// Log for statistics
int packets_transmission = 0;
int packets_retransmissions = 0;
int packets_dropped = 0;
int packets_success = 0;
int acks_received = 0;
int timeout_expirations = 0;

// Internal state management
char GameOn = TRUE;
int sequence = 0;

int simulate_loss(void)
{
    return ((double)rand() / (double)RAND_MAX) < PACKET_LOSS_RATIO;
}

int main()
{

    struct sockaddr_in server_addr;
    int client_fd, len, n;
    Frame send_frame, recv_frame;

    // Creating socket file descriptor
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    // Filling server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    initscr(); // Initialize ncurses
    cbreak();  // Disable line buffering
    noecho();  // Don't echo characters entered by the user
    char c;

    // Randomize rand() seed
    srand(time(NULL));

    len = sizeof(server_addr); // len is value/resuslt

    while (GameOn)
    {
        int acked = 0;

        if ((c = getch()) != ERR)
        {

            switch (c)
            {
            case 's':
            case 'd':
            case 'a':
            case 'w':
                // Valid keystroke: send to server, update packet count
                clear();

                // Set other fields of Frame struct
                send_frame.sequence = sequence; // Assuming sequence number
                send_frame.ack = 0;             // Assuming ack number
                send_frame.buffer = c;

                //* Packet n generated for transmission
                printw("Packet %d generated for transmission\n", sequence);
                refresh();

                packets_transmission++;
                packets_retransmissions++;
                packets_success++;

                if (simulate_loss() == 0)
                {
                    sendto(client_fd, &send_frame, sizeof(Frame), 0, (const struct sockaddr *)&server_addr, len);

                    //* Packet n successfully transmitted with c data bytes
                    printw("Packet %d successfully transmitted with %lu data bytes\n", sequence, sizeof(send_frame));
                    refresh();
                }
                else
                {
                    //* Packet n lost
                    printw("Packet %d lost\n", sequence);
                    refresh();
                    packets_dropped++;
                }

                break;
            case 'q':
                // Quit the game & terminate connection
                clear();
                GameOn = FALSE;
                continue;
            default:
                continue;
            }
        }

        // Receive response from server
        while (acked == 0)
        {

            // Setup timeout
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 0.1 seconds

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(client_fd, &fds);
            int result = select(client_fd + 1, &fds, NULL, NULL, &timeout);
            if (result == -1)
            {
                perror("select error");
                GameOn = FALSE;
                continue;
                // exit(EXIT_FAILURE);
            }
            else if (result == 0)
            {

                //* Timeout expired for packet numbered n
                printw("Timeout expired for packet numbered %d\n", sequence);
                //* Packet n generated for re-transmission
                printw("Packet %d generated for re-transmission\n", sequence);
                packets_retransmissions++;
                timeout_expirations++;

                // printw("Timeout occurred. No response from server.\n");
                sendto(client_fd, &send_frame, sizeof(Frame), 0, (const struct sockaddr *)&server_addr, len);

                //* Packet n successfully transmitted with c data bytes
                printw("Packet %d successfully transmitted with %lu data bytes\n", sequence, sizeof(send_frame));
                refresh();
                packets_success++;
                continue;
            }
            else
            {
                // Data is ready to be received
                n = recvfrom(client_fd, &recv_frame, sizeof(Frame), 0, NULL, NULL);

                if (recv_frame.sequence == sequence + 1 && recv_frame.ack == 1)
                {
                    //* ACK n received
                    printw("ACK %d received\n", recv_frame.sequence);
                    refresh();
                    acks_received++;
                }
                else
                {
                    // Invalid ACK
                    continue;
                }

                acked = 1;
            }
        }

        // Client received ACK, update sequence number
        sequence++;
    }

    close(client_fd);
    endwin();

    // Log statitistics:
    //* 1. Number of data packets generated for transmission (initial transmission only)
    //* 2. Total number of data packets generated for retransmission (initial transmissions plus retransmissions)
    //* 3. Number of data packets dropped due to loss
    //* 4. Number of data packets transmitted successfully (initial transmissions plus retransmissions)
    //* 5. Number of ACKs received
    //* 6. Count of how many times the timeout expired
    printf("Number of data packets generated for transmission: %d\n", packets_transmission);
    printf("Total number of data packets generated for retransmission: %d\n", packets_retransmissions);
    printf("Number of data packets dropped due to loss: %d\n", packets_dropped);
    printf("Number of data packets transmitted successfully: %d\n", packets_success);
    printf("Number of ACKs received: %d\n", acks_received);
    printf("Count of how many times the timeout expired: %d\n", timeout_expirations);

    return 0;
}
