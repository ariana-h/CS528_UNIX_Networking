#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 12345

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
    int sockfd;
    struct sockaddr_in server_addr, my_addr;
    socklen_t addr_len = sizeof(server_addr);
    char recv_char;
    char token[12];
    int token_index = 0;
    
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
    server_addr.sin_addr.s_addr = inet_addr("10.0.0.20");

    const char *init_msg = "Hello";
    if (sendto(sockfd, init_msg, strlen(init_msg), 0,
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

    printf("Waiting for message from server:\n");
    while (recvfrom(sockfd, &recv_char, 1, 0, NULL, NULL) > 0) {
         if (recv_char == ' ') {

             if (token_index == 0) { printf(" "); } 
             else {
                 token[token_index] = '\0';
                 printf("%c", lookup_letter(token));
                 token_index = 0; 
             }

         } else if (recv_char == '-' || recv_char == 'o') {
             token[token_index++] = recv_char;
         } else {
             token[token_index++] = recv_char;
         }
    }
    
    printf("\n");
    close(sockfd);
    return 0;
}

