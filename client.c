#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "You need to provide the port to connect to and your name!\nUsage: %s <port> <name>\n", argv[0]);
        return 1;
    }

    const int PORT = atoi(argv[1]);
    const char *NAME = argv[2];
    int max_fd;
    fd_set read_fds;

    // Create connection
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Failed to create socket");
        return 2;
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr)); // Zero out the structure
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    inet_aton("127.0.0.1", &client_addr.sin_addr);

    // Checks the connection status
    if (connect(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        perror("Socket connection error");
        return 3;
    }

    printf("Connected to the server at:%d\n", PORT);

    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 16]; // Additional space for the NAME and formatting

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        max_fd = client_socket > STDIN_FILENO ? client_socket : STDIN_FILENO;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        // Check if there is incoming data from the server
        if (FD_ISSET(client_socket, &read_fds)) {
            ssize_t message_recv = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (message_recv < 0) {
                perror("Failed to receive message");
                break;
            } else if (message_recv == 0) {
                printf("Server closed the connection\n");
                break;
            }
            buffer[message_recv] = '\0'; // Null-terminate the received data
            printf("%s\n", buffer);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
                snprintf(message, sizeof(message), "%s: %s", NAME, buffer); // Prepend NAME to the message
                if (send(client_socket, message, strlen(message), 0) < 0) {
                    perror("Message can't be sent\n");
                    break;
                }
            }
        }
    }

    close(client_socket);

    return 0;
}
