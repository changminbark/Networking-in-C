/**
 * This is a simple demo server, that always sends the same ERROR response to any request.
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "spotify.h"

unsigned int compute_checksum(void *message, int size, unsigned int seed) {
    unsigned char *data = (unsigned char *)message;
    for (int i = 0; i < size; i++) {
        seed += data[i];
    }
    return seed & 0xffffffff;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // set up the address struct for any IP address and port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(17381);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 17381\n");
    while (1){
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted\n");

        // read the request and ignore it
        struct request_msg request ={0};
        int read_size = recv(new_socket, &request, sizeof(request), 0);
        if (read_size <= 0) {
            perror("recv");
            close(new_socket);
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // create a sample response message
        struct response_msg response = {0};
        response.header.status = ERROR;
        response.header.num_tracks = 0;
        response.header.num_albums = 0;
        response.header.num_playlists = 0;
        
        strcpy(response.error_message, "Sample error message");

        response.header.check = compute_checksum(&response.header, sizeof(response.header), 1337);
        response.header.check += compute_checksum(response.error_message, sizeof(response.error_message), response.header.check);
        
        // send the complete response
        send(new_socket, &response.header, sizeof(response.header), 0);
        send(new_socket, response.error_message, sizeof(response.error_message), 0);

        printf("Response sent. Disconnecting.\n");
        
        close(new_socket);
    }
    close(server_fd);

    return 0;
}