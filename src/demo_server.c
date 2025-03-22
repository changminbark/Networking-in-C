/**
 * This is a simple demo server, that always sends the same response to any request.
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
    address.sin_port = htons(17380);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 17380\n");
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
        response.header.status = OK;
        response.header.num_tracks = 2;
        response.header.num_albums = 1;
        response.header.num_playlists = 1;        

        response.data.tracks = (struct track *)calloc(response.header.num_tracks, sizeof(struct track));
        response.data.albums = (struct album *)calloc(response.header.num_albums, sizeof(struct album));
        response.data.playlists = (struct playlist *)calloc(response.header.num_playlists, sizeof(struct playlist));

        // Populate the response data with dummy data 
        strcpy(response.data.tracks[0].track_id, "track101");
        strcpy(response.data.tracks[0].name, "Server Track 1");
        strcpy(response.data.tracks[0].artist, "artist101");

        strcpy(response.data.tracks[1].track_id, "track102");
        strcpy(response.data.tracks[1].name, "Server Track 2");
        strcpy(response.data.tracks[1].artist, "artist102");

        strcpy(response.data.albums[0].album_id, "album201");
        strcpy(response.data.albums[0].name, "Server Album A");

        strcpy(response.data.playlists[0].playlist_id, "playlist301");
        strcpy(response.data.playlists[0].name, "Server Playlist X");
        strcpy(response.data.playlists[0].genre, "test genre");
        strcpy(response.data.playlists[0].subgenre, "test subgenre");

        response.header.check = compute_checksum(&response.header, sizeof(response.header), 1337);
        response.header.check += compute_checksum(response.data.tracks, response.header.num_tracks * sizeof(struct track), response.header.check);
        response.header.check += compute_checksum(response.data.albums, response.header.num_albums * sizeof(struct album), response.header.check);
        response.header.check += compute_checksum(response.data.playlists, response.header.num_playlists * sizeof(struct playlist), response.header.check);
            
        // send the complete response
        send(new_socket, &response.header, sizeof(response.header), 0);
        send(new_socket, response.data.tracks, sizeof(struct track) * response.header.num_tracks, 0);
        send(new_socket, response.data.albums, sizeof(struct album) * response.header.num_albums, 0);
        send(new_socket, response.data.playlists, sizeof(struct playlist) * response.header.num_playlists, 0);

        printf("Response sent. Disconnecting.\n");

        free(response.data.tracks);
        free(response.data.albums);
        free(response.data.playlists);
        close(new_socket);
    }
    close(server_fd);

    return 0;
}