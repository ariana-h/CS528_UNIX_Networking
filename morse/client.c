#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define LETTER_DELIMITTER '/'

#define PORT 12345
#define BUFF_SIZE 4096

/*
    Authors: Ariana Huhko and John Pertell
    02/2025

    Description: A UDP client that sends plaintext messages to a server.
                 The server responds with Morse code where each dot/dash is a separate character,
                 letters are separated by '/', and words are separated by a space. 
                
    Decoding?
                 The user can pass in an optional '-d' argument to specify if they want to
                 decode the recieved data.

    Packet Drop?
                 If a word can not be translated (I.E., bad packet drop), the decoded letter may be 
                 replaced with a '?' instead. 
                 A false letter may be decoded in the event of packet drop, the correct letter may be
                 interpreted by the user by examining the context of the morse code.
*/

struct morse_map {
    char letter;
    const char *code;
};

static struct morse_map morse_table[] = {
    {'A', "o-"},    {'B', "-ooo"},  {'C', "-o-o"},  {'D', "-oo"},
    {'E', "o"},     {'F', "oo-o"},  {'G', "--o"},   {'H', "oooo"},
    {'I', "oo"},    {'J', "o---"},  {'K', "-o-"},   {'L', "o-oo"},
    {'M', "--"},    {'N', "-o"},    {'O', "---"},   {'P', "o--o"},
    {'Q', "--o-"},  {'R', "o-o"},   {'S', "ooo"},   {'T', "-"},
    {'U', "oo-"},   {'V', "ooo-"},  {'W', "o--"},   {'X', "-oo-"},
    {'Y', "-o--"},  {'Z', "--oo"},
    {0, NULL}
};

/* Attempts to decode a token 
   If token can not be decoded, then a '?' 
   will be used instead.
*/
static char decode_morse(const char *token)
{
    for (int j = 0; morse_table[j].letter != 0; j++) {
        if (strcmp(morse_table[j].code, token) == 0) {
            return morse_table[j].letter;
        }
    }
    return '?';
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <SERVER_IP> [-d]\n", argv[0]);
        return 1;
    }

    // Check if decoding is enabled
    int decode = 0;
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        decode = 1;
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family      = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    // Set up server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Set a short receive timeout so we know when the server is done
    struct timeval tv = {0, 500000};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return 1;
    }

    printf("Morse Client running.\n");
    printf("==========================================\n");
    while (1)
    {
        char user_input[256];
        printf("Enter a message to send (or type 'exit' to quit): ");
        fgets(user_input, sizeof(user_input), stdin);

        // Trim newline => replace with '\0'
        user_input[strcspn(user_input, "\n")] = '\0';

        // Checking for exit command
        if (strcmp(user_input, "exit") == 0) {
            break;
        }

        // Send user input to server
        ssize_t sent = sendto(sockfd, user_input, strlen(user_input), 0,
                              (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (sent < 0) {
            perror("sendto");
            break;
        }

        // Prepare to receive server's Morse response
        char recv_buffer[BUFF_SIZE];
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int total_bytes = 0; // ensure buffer does not overfill

        // Receiving loop
        while (1) {
            ssize_t r = recvfrom(sockfd, recv_buffer+total_bytes,
                                 BUFF_SIZE-total_bytes-1, 0, NULL, NULL);
            if (r < 0) {
                // timeout => leave loop
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    perror("recvfrom");
                    break;
                }
            } else if (r == 0) {
                // No more data => leave loop
                break;
            } else {
                total_bytes += r;
                if (total_bytes >= BUFF_SIZE - 1)
                    break; // buffer full
            }
        }

        // Null terminate received data
        recv_buffer[total_bytes] = '\0';

        // Display Morse Code
        if (total_bytes > 0) {
            printf("\nReceived Morse Code: %s\n", recv_buffer);

            // Only decode if the user enabled it with -d
            if (decode) {
                char final_message[BUFF_SIZE];
                memset(final_message, 0, sizeof(final_message));
                int final_idx = 0;

                char token[64];
                int token_idx = 0;

                for (int i = 0; i < total_bytes; i++) {
                    char c = recv_buffer[i];

                    if (c == LETTER_DELIMITTER) {
                        // End of one letter
                        token[token_idx] = '\0';
                        if (token_idx > 0)
                            final_message[final_idx++] = decode_morse(token);
                        token_idx = 0;
                    } else if (c == ' ') {
                        // Space => separate word
                        if (token_idx > 0) {
                            token[token_idx] = '\0';
                            final_message[final_idx++] = decode_morse(token);
                            token_idx = 0;
                        }
                        // Then add a space to final_message
                        final_message[final_idx++] = ' ';
                    } else {
                        // This is a dot or dash => accumulate in token
                        if (token_idx < (int)sizeof(token) - 1)  
                            token[token_idx++] = c;
                    }
                }

                // If there's a leftover token that never ended with '#' or space
                if (token_idx > 0) {
                    token[token_idx] = '\0';
                    final_message[final_idx++] = decode_morse(token);
                }

                final_message[final_idx] = '\0';
                printf("Decoded Text: %s\n\n", final_message);
            }
            printf("==========================================\n\n");
        } else { 
            printf("\nNo data received from server.\n\n"); 
        }
    }

    close(sockfd);
    printf("Exiting client...\n");
    return 0;
}