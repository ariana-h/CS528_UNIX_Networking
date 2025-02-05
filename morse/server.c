/*
    Authors: Ariana Huhko and John Pertell
    02/2025

    Description: The server converts plaintext messages sent by the clients into Morse code and sends it back. Think of a translation app.

    Why UDP?: Speed is prioritized over reliability since minor loss won't impact understanding. 
              Imagine if a text message had a typo...not a big deal, humans can still figure out what the word means. 
              Also a fun way to see the effect of packet loss when using Core.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
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

const char *lookup_morse(char letter) {
    for (int i = 0; morse_table[i].letter != 0; i++) {
        if (morse_table[i].letter == letter)
            return morse_table[i].code;
    }
    return NULL;
}

int main(int argc, char **argv) {

    if(argc < 2) {
        printf("Usage: %s <SERVER_IP>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    //char buf;
    char buffer[BUFFER_SIZE] = {0};
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port        = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
         perror("bind");
         close(sockfd);
         exit(EXIT_FAILURE);
    }
    
    printf("Server is running and waiting for a client...\n");
    /* Main Loop */
    while(1) {
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, 
                    (struct sockaddr*)&client_addr, &client_addr_len);
        if ( recv_len < 0) {
            perror("recvfrom");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
 
        buffer[recv_len] = '\0';
        printf("Received message: %s\n", buffer);

        char morse_message[4096] = {0};
        int morse_index = 0;
        
        for (size_t i = 0; i < strlen(buffer); i++) {
            char ch = toupper(buffer[i]);
            if (ch == ' ') {
                if (morse_index < sizeof(morse_message) - 2) {
                    morse_message[morse_index++] = ' ';
                }
            } else {
                const char *code = lookup_morse(ch);
                if (code != NULL) {
                    if (morse_index + strlen(code) < sizeof(morse_message) - 2) {
                        strcpy(&morse_message[morse_index], code);
                        morse_index += strlen(code);
                        morse_message[morse_index++] = ' ';
                    }
                }
            }
        }
        morse_message[morse_index] = '\0';

        if (sendto(sockfd, morse_message, strlen(morse_message), 0, (struct sockaddr*)&client_addr, client_addr_len) < 0) {
            perror("sendto");
        } else {
            printf("Sent Morse message: %s\n", morse_message);
        }

    }

    close(sockfd);
    return 0;
}

