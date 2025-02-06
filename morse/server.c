/*
    Authors: Ariana Huhko and John Pertell
    02/2025

    Description: The server converts plaintext messages sent by the clients into
                 Morse code and sends it back, one symbol('o' or '-') at a time.
                 Letters are separated by '/' and words are separated by a space. 

    Why UDP?: Speed is prioritized over reliability since minor loss won't impact understanding.
              Also a fun way to see the effect of packet loss when using Core.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define LETTER_DELIMITTER "/"

#define PORT 12345
#define BUFFER_SIZE 1024

struct morse_map {
    char letter;
    const char *code;
};

struct morse_map morse_table[] = {
    {'A', "o-"},    {'B', "-ooo"},  {'C', "-o-o"},  {'D', "-oo"},
    {'E', "o"},     {'F', "oo-o"},  {'G', "--o"},   {'H', "oooo"},
    {'I', "oo"},    {'J', "o---"},  {'K', "-o-"},   {'L', "o-oo"},
    {'M', "--"},    {'N', "-o"},    {'O', "---"},   {'P', "o--o"},
    {'Q', "--o-"},  {'R', "o-o"},   {'S', "ooo"},   {'T', "-"},
    {'U', "oo-"},   {'V', "ooo-"},  {'W', "o--"},   {'X', "-oo-"},
    {'Y', "-o--"},  {'Z', "--oo"},
    {0, NULL}
};

/* Lookup the Morse code string for a single character. */
const char *lookup_morse(char letter) {
    for (int i = 0; morse_table[i].letter != 0; i++) {
        if (morse_table[i].letter == letter) {
            return morse_table[i].code;
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <SERVER_IP>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind server to the given IP and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port        = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for a client on %s:%d...\n",
           argv[1], PORT);

    while (1) {
        // Receive the message from client
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Null-terminate the buffer
        buffer[recv_len] = '\0';
        printf("Received message: %s\n", buffer);

        printf("SENDING MORSE CODE:\n\t");
        for (size_t i = 0; i < strlen(buffer); i++) {
            char ch = toupper(buffer[i]);

            // If it's a space, just send a space to indicate word separator
            if (ch == ' ') {
                sendto(sockfd, " ", 1, 0,
                       (struct sockaddr*)&client_addr, client_addr_len);
                printf(" ");
                continue;
            }

            // Look up the Morse code
            const char *code = lookup_morse(ch);
            if (code != NULL) {
                // Send each dot/dash individually
                for (size_t j = 0; j < strlen(code); j++) {
                    char symbol[2] = {code[j], '\0'};
                    sendto(sockfd, symbol, 1, 0,
                           (struct sockaddr*)&client_addr, client_addr_len);
                    printf("%s", symbol);
                }
                // Send LETTER_DELIMITTER to separate letters
                sendto(sockfd, LETTER_DELIMITTER, 1, 0,
                       (struct sockaddr*)&client_addr, client_addr_len);
                printf("%s", LETTER_DELIMITTER);
            }
        }
        printf("\nFinished sending Morse code.\n");
    }

    close(sockfd);
    return 0;
}