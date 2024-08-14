#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MESSAGE_SIZE 1024
#define MAX 99

// Mutex for chat log
pthread_mutex_t chat_log_mutex;
pthread_mutex_t client_list_mutex;

// Structure for each client data
typedef struct {
    int client_socket;
    FILE *chat_history;
} client_data_t;

int client_count = 0;
int client_sockets[MAX];

void *handle_client(void *arg) {

    client_data_t *data = (client_data_t *) arg;
    int client_socket = data->client_socket;
    FILE *chat_history = data->chat_history;
    char buffer[MESSAGE_SIZE];

    while (1) {
        ssize_t message_recv = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (message_recv > 0) {
            buffer[message_recv] = '\0';  // Null-terminate the received data
            
            // Write the message to the chat log
            pthread_mutex_lock(&chat_log_mutex);
            fwrite(buffer, sizeof(char), message_recv, chat_history);
            fwrite("\n", sizeof(char), 1, chat_history);
            fflush(chat_history);
            pthread_mutex_unlock(&chat_log_mutex);

            printf("%s", buffer);
            // Only send it to the others not the person who sent it.
            pthread_mutex_lock(&client_list_mutex);
            for(int i = 0; i < client_count; i++) {
                if (client_sockets[i] != client_socket) 
                    send(client_sockets[i], buffer, message_recv, 0);
            }
            pthread_mutex_unlock(&client_list_mutex);
        } else if (message_recv == 0) {
            // Client has closed the connection
            printf("Client disconnected.\n");
            pthread_mutex_lock(&client_list_mutex);
            for (int i = 0; i < client_count; i++) {
                if (client_sockets[i] == client_socket) {
                    // Shift the remaining clients down
                    for (int j = i; j < client_count - 1; j++) {
                        client_sockets[j] = client_sockets[j + 1];
                    }
                    client_count--;
                    break;
                }
            }
            pthread_mutex_unlock(&client_list_mutex);
            break;
        } else {
            perror("Failed to receive message");
            break;
        }
    }

    close(client_socket);
    free(data);
    return NULL;
};

int main(int argc, char* argv[]) {
    // This is what the format should be
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <Max # of people>\n", argv[0]);
        return 1;
    }
    // Init mutex
    pthread_mutex_init(&client_list_mutex, NULL);

    // PORT number here
    const int PORT = atoi(argv[1]);

    // Max number of people you want to have in the chat
    const int NUM_OF_PEOPLE = atoi(argv[2]);

    // Open up the file descripter for chat log
    FILE* chat_history = fopen("chat_log.txt", "a");
    if (!chat_history) {
        perror("Failed to open or create the chat log");
        return 2;
    }

    // This is the start of the server implementation
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        return 3;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        return 4;
    }

    if (listen(server_socket, NUM_OF_PEOPLE) < 0) {
        perror("Failed to listen on socket");
        return 5;
    }

    printf("Server listening on port %d\n", PORT);

    // start up logo (use it wisely)
    FILE* logo = fopen("./start_up_logo.txt", "r");
    char pepe[4844];
    fread(pepe, sizeof(pepe), sizeof(logo), logo);
    printf("%s", pepe);
    fclose(logo);

    int client_socket;
    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Failed to accept client connection");
            continue;
        }

        pthread_mutex_lock(&client_list_mutex);
        if (client_count < NUM_OF_PEOPLE) {
            client_sockets[client_count++] = client_socket;
        } else {
            printf("Maximum clients reached. Cannot accept more connections.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&client_list_mutex);

        // Make the memory space for the client data
        client_data_t *client_data = malloc(sizeof(client_data_t));
        // init value
        client_data->client_socket = client_socket;
        client_data->chat_history = chat_history;

        // Create pthread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_data) != 0) {
            perror("Error while creating thread for the client");
            close(client_socket);
            free(client_data);
            continue;
        }
    }

    fclose(chat_history);
    close(server_socket);
    pthread_mutex_destroy(&client_list_mutex);
    return 0;
}
