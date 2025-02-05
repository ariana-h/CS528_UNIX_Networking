/*
    Authors: Ariana Huhko and John Pertell
    02/2025

    Description: Clients send text messages that are converted to Morse code by the server and sent back. Think of a translation app.

    Why UDP?: Speed is prioritized over reliability since minor loss won't impact understanding. 
              Imagine if a text message had a typo...not a big deal, humans can still figure out what the word means. 
              Also a fun way to see the effect of packet loss when using Core.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

char lookup_letter(const char *code) {
    for (int i = 0; morse_table[i].letter; i++) {
        if (strcmp(morse_table[i].code, code) == 0)
            return morse_table[i].letter;
    }
    return '?';  // unknown 
}


int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: %s <SERVER_IP>\n", argv[0]);
        return 1;
    }
    
    int sockfd;
    struct sockaddr_in server_addr, my_addr;
    socklen_t addr_len = sizeof(server_addr);
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
         perror("socket");
         exit(EXIT_FAILURE);
    }
    
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = 0;
    
    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
         perror("bind");
         close(sockfd);
         exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);  

    while(1) {
        // Take user input and send to server
        char user_input[256];
        printf("Enter a message to send (or type 'exit' to quit): ");
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = '\0'; // Remove newline if necessary

        if (strcmp(user_input, "exit") == 0) {
            break;
        }

        if (sendto(sockfd, user_input, strlen(user_input), 0,
                (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("sendto");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        struct timeval tv = {0, 1};
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("setsockopt");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        
        char morse_message[BUFFER_SIZE] = {0};
        int morse_index = 0;

        printf("Waiting for Morse code from server...\n\n");
        ssize_t recv_len;
        while ((recv_len = recvfrom(sockfd, morse_message + morse_index, BUFFER_SIZE - morse_index - 1, 0, NULL, NULL)) > 0) {
            morse_index += recv_len;
            if (morse_index >= BUFFER_SIZE - 1 || morse_message[morse_index - 1] == '\0') {
                break;
            }
        }

        morse_message[morse_index] = '\0';
        printf("\tMorse Code Message: %s", morse_message);

        char message[BUFFER_SIZE] = {0};
        int message_index = 0;
        char token[12];
        int token_index = 0;

        for (int i = 0; morse_message[i] != '\0'; i++) {
            if (morse_message[i] == ' ') {
                if (token_index > 0) { // Translate token
                    token[token_index] = '\0';
                    char translated = lookup_letter(token);
                    message[message_index++] = translated;
                    token_index = 0;
                } else { // Handle space between words
                    if (message_index < sizeof(message) - 1) {
                        message[message_index++] = ' ';
                    }
                }
            } else {
                if (token_index < sizeof(token) - 1) {
                    token[token_index++] = morse_message[i];
                }
            }
        }

        if (token_index > 0) {
            token[token_index] = '\0';
            char translated = lookup_letter(token);
            message[message_index++] = translated;
        }

        message[message_index] = '\0';
        printf("\n\tFull Translated Message: %s\n", message);
        
        printf("\n===================================================================\n\n");
    }
    
    printf("Exiting client...\n");
    close(sockfd);
    return 0;
}

