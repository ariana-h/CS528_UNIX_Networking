#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
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

const char *lookup_morse(char letter) {
    for (int i = 0; morse_table[i].letter != 0; i++) {
        if (morse_table[i].letter == letter)
            return morse_table[i].code;
    }
    return NULL;
}

int main(int argc, char **argv) {

    if(argc < 3) {
        printf("Usage: %s <SERVER_IP> <BROADCAST MESSAGE>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buf;
    
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
        if (recvfrom(sockfd, &buf, 1, 0, (struct sockaddr*)&client_addr, &client_addr_len) < 0) {
            perror("recvfrom");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Received initial message from client.\n");
        
        
        for (size_t i = 0; i < strlen(argv[2]); i++) {
            char ch = argv[2][i];
            if (ch == ' ') {
                buf = ' ';
                if (sendto(sockfd, &buf, 1, 0, (struct sockaddr*)&client_addr, client_addr_len) < 0)
                    perror("sendto");
            } else {
                ch = toupper(ch);
                const char *code = lookup_morse(ch);
                if (code != NULL) {
                    for (size_t j = 0; j < strlen(code); j++) {
                        buf = code[j]; 
                        if (sendto(sockfd, &buf, 1, 0, (struct sockaddr*)&client_addr, client_addr_len) < 0)
                            perror("sendto");
                    }
                }
                buf = ' ';
                if (sendto(sockfd, &buf, 1, 0, (struct sockaddr*)&client_addr, client_addr_len) < 0)
                    perror("sendto");
            }
        }
    }
    printf("Message sent.\n");
    close(sockfd);
    return 0;
}

