/**
 * @file sclient.c
 * @author Chang Min Bark
 * @brief Spotify data client
 * @version 0.1
 * @date 2025-02-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "spotify.h"

/**
 * Capitalizes a string in place.
 */
void strcaps(char *str) {
    while(*str) {
        *str = toupper(*str);
        str++;
    }
}

unsigned int compute_checksum(void *message, int size, unsigned int seed) {
    if (message == NULL) {
        // fprintf(stderr, "Error: compute_checksum received NULL message pointer!\n");
        return seed;
    }

    unsigned char *data = (unsigned char *)message;

    // printf("Computing checksum: message=%p, size=%d, seed=%u\n", message, size, seed);
    // fflush(stdout);

    for (int i = 0; i < size; i++) {
        seed += data[i];		
    }
	// ensure the check sum is 32 bits
    return seed & 0xffffffff;
}

int parse_req(char *command, struct request_msg *req) {
    char *tokens[3] = {0};
    char cmd_copy[256] = {0};  // Zero-initialized copy to avoid garbage
    if (!req || !command) {
        return 1;
    }

    // Copy command into a local buffer
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    // Tokenize the command into at most 3 tokens
    tokens[0] = cmd_copy;
    for (int i = 1; i < 3; i++) {
        // If the previous token is NULL, stop tokenization.
        if (tokens[i - 1] == NULL)
            break;
        tokens[i] = strchr(tokens[i - 1], ' ');
        if (tokens[i] == NULL) // If there are no more spaces, break
            break;
        *tokens[i] = '\0';  // Terminate previous token
        tokens[i]++;        // Move past the space

        // Trim leading whitespace
        while (isspace(*tokens[i])) {
            tokens[i]++;
        }
        // Trim trailing whitespace
        size_t tlen = strlen(tokens[i]);
        while (tlen > 0 && isspace(tokens[i][tlen - 1])) {
            tokens[i][tlen - 1] = '\0';
            tlen--;
        }
        // Remove enclosing quotes, if present
        if (tlen >= 2 && tokens[i][0] == '"' && tokens[i][tlen - 1] == '"') {
            tokens[i][tlen - 1] = '\0';
            tokens[i]++;
        }
    }

    // Make sure we have at least one token
    if (!tokens[0] || *tokens[0] == '\0')
        return 1;

    // Convert first token (command) to uppercase
    char *cmd1 = tokens[0];
    strcaps(cmd1);

    // Process QUIT command (which takes no further argument)
    if (strcmp(cmd1, "QUIT") == 0 || strcmp(cmd1, "TEST") == 0) {
        req->command = QUIT;
        req->args[0] = '\0';
        goto compute_checksum_label;
    }
    // Process SHOW or SEARCH commands
    else if (strcmp(cmd1, "SHOW") == 0 || strcmp(cmd1, "SEARCH") == 0) {
        // Require a second token for the subcommand
        if (!tokens[1] || *tokens[1] == '\0') {
            fprintf(stderr, "Error: Missing argument after %s\n", cmd1);
            return 1;
        }
        char *cmd2 = tokens[1];
        strcaps(cmd2);

        if (strcmp(cmd2, "TRACKS") == 0) {
            req->command = (strcmp(cmd1, "SHOW") == 0) ? SHOW_TRACKS : SEARCH_TRACKS;
        } else if (strcmp(cmd2, "ALBUMS") == 0) {
            req->command = (strcmp(cmd1, "SHOW") == 0) ? SHOW_ALBUMS : SEARCH_ALBUMS;
        } else if (strcmp(cmd2, "PLAYLISTS") == 0) {
            req->command = (strcmp(cmd1, "SHOW") == 0) ? SHOW_PLAYLISTS : SEARCH_PLAYLISTS;
        } else if (strcmp(cmd2, "ARTISTS") == 0) {
			if (strcmp(cmd1, "SEARCH") == 0) {
				req->command = SEARCH_ARTISTS;
			} else {
				fprintf(stderr, "Error: Invalid argument \"%s\"\n", cmd2);
            	return 1;
			}
        } else {
            fprintf(stderr, "Error: Invalid second argument \"%s\"\n", cmd2);
            return 1;
        }

        // For SHOW commands, require a third token (e.g. a number)
        if (!tokens[2] || *tokens[2] == '\0') {
            if (strcmp(cmd1, "SHOW") == 0) {
                fprintf(stderr, "Error: Missing numeric argument for %s %s\n", cmd1, cmd2);
                return 1;
            } else {
                // For SEARCH commands, allow an empty argument (if desired)
                req->args[0] = '\0';
            }
        } else {
            strncpy(req->args, tokens[2], sizeof(req->args) - 1);
            req->args[sizeof(req->args) - 1] = '\0';
        }
    } else {
        fprintf(stderr, "Error: Unknown command \"%s\"\n", cmd1);
        return 1;
    }

compute_checksum_label:
    // Zero out the checksum field before computing the checksum.
    req->check = 0;
    req->check = compute_checksum(req, sizeof(struct request_msg), 7331);

    return 0;
}




int connect_to(char *host, char *port)
{
	int sockfd, status;
	struct addrinfo hints, *res, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return -1;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
			char ipstr[INET6_ADDRSTRLEN];
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			void *addr = &(ipv4->sin_addr);
			inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
			printf(" Making IPv4 connection to %s\n", ipstr);
			printf("   Connected.\n");
			freeaddrinfo(res);
			return sockfd;
		}
	}

	freeaddrinfo(res);

	return -1;
}


int recv_response(int sockfd, struct response_msg *resp)
{	
	if (!resp || sockfd < 0){
		fprintf(stderr, "Invalid response or socket\n");
		return -1;		
	}
	// Receive and parse response header
	if (recv(sockfd, &resp->header, sizeof(struct response_header), 0) != sizeof(struct response_header)) {
		perror("Error receiving response header");
		return -1;
	}

	// Check status
	if (resp->header.status == ERROR) {
		if (recv(sockfd, &resp->error_message, sizeof(resp->error_message), 0) <= 0) {
			perror("Error receiving error message");
			return -1;
		}
		return 0;
	} else if (resp->header.status == OK) {
		// Valid response, so we parse and populate our response struct

		if (resp->header.num_tracks > 0) {
			// Allocate memory first
			resp->data.tracks = malloc(resp->header.num_tracks * sizeof(struct track));
			if (!resp->data.tracks) {
				fprintf(stderr, "Memory allocation failed for tracks\n");
				return -1;
			}
			// Receive data
			if (recv(sockfd, resp->data.tracks, resp->header.num_tracks * sizeof(struct track), 0) != (ssize_t)(resp->header.num_tracks * sizeof(struct track))) {
				perror("Error receiving tracks data");
				return -1;
			}
		} else {
			resp->data.tracks = NULL;
		}
		
		if (resp->header.num_albums > 0) {
			// Allocate memory first
			resp->data.albums = malloc(resp->header.num_albums * sizeof(struct album));
			if (!resp->data.albums) {
				fprintf(stderr, "Memory allocation failed for albums\n");
				return -1;
			}
			// Receive data
			if (recv(sockfd, resp->data.albums, resp->header.num_albums * sizeof(struct album), 0) != (ssize_t)(resp->header.num_albums * sizeof(struct album))) {
				perror("Error receiving albums data");
				return -1;
			}
		} else {
			resp->data.albums = NULL;
		}

		if (resp->header.num_playlists > 0) {
			// Allocate memory first
			resp->data.playlists = malloc(resp->header.num_playlists * sizeof(struct playlist));
			if (!resp->data.playlists) {
				fprintf(stderr, "Memory allocation failed for playlists\n");
				return -1;
			}
			// Receive data
			if (recv(sockfd, resp->data.playlists, resp->header.num_playlists * sizeof(struct playlist), 0) != (ssize_t)(resp->header.num_playlists * sizeof(struct playlist))) {
				perror("Error receiving playlists data");
				return -1;
			}
		} else {
			resp->data.playlists = NULL;
		}
		
		return 0;
	}

	// return 0 on success, -1 on failure

	return -1;
}

void free_resp(struct response_msg *resp) {
	// This makes sure if resp is not available then we return
	// We also return if there is an error message (thus no data)
    if (!resp || resp->error_message) return;

	// Go through all of the data and check whether they exist (not NULL), then free
    if (resp->data.tracks) {
        free(resp->data.tracks);
        resp->data.tracks = NULL;
    }
    if (resp->data.albums) {
        free(resp->data.albums);
        resp->data.albums = NULL;
    }
    if (resp->data.playlists) {
        free(resp->data.playlists);
        resp->data.playlists = NULL;
    }
}


void print_response(struct response_msg *resp)
{
	// This function is complete. You should not need to modify it.
	if (!resp){
		fprintf(stderr, "Invalid response\n");
		return;
	}
	if (resp->header.status == OK){
		printf("Response Status: OK\n");
		printf("Number of Tracks: %d\n", resp->header.num_tracks);
		printf("Number of Albums: %d\n", resp->header.num_albums);
		printf("Number of Playlists: %d\n", resp->header.num_playlists);
		
		unsigned int received_check = resp->header.check;	
		resp->header.check = 0; // reset the check sum to zero for computation
		unsigned int check = compute_checksum(&resp->header, sizeof(resp->header), 1337);
		check += compute_checksum(resp->data.tracks, resp->header.num_tracks * sizeof(struct track), check);
		check += compute_checksum(resp->data.albums, resp->header.num_albums * sizeof(struct album), check);
		check += compute_checksum(resp->data.playlists, resp->header.num_playlists * sizeof(struct playlist), check);
		resp->header.check = received_check; // restore the original check sum

		printf("Computed Check Sum: %d\n", check);
		printf("  Header Check Sum: %d\n", resp->header.check);
		if (check != resp->header.check){
			fprintf(stderr, "Check sum mismatch: computed %d, header %d\n", check, resp->header.check);
			return;
		}		
				
		printf("Data:\n");

		for (int i = 0; i < resp->header.num_tracks; i++){
			print_track( &resp->data.tracks[i]);
		}	
		for (int i = 0; i < resp->header.num_albums; i++){
			print_album(&resp->data.albums[i]);
		}
		for (int i = 0; i < resp->header.num_playlists; i++){
			print_playlist(&resp->data.playlists[i]);
		}

	} else if (resp->header.status == ERROR){
		unsigned int received_check = resp->header.check;
		resp->header.check = 0; // reset the check sum to zero for computation
		unsigned int check = compute_checksum(&resp->header, sizeof(resp->header), 1337);
		check += compute_checksum(resp->error_message, sizeof(resp->error_message), check);
		resp->header.check = received_check; // restore the original check sum

		printf("Response Status: ERROR\n");	
		printf("Computed Check Sum: %d\n", check);
		printf("  Header Check Sum: %d\n", resp->header.check);
		if (check != resp->header.check){
			fprintf(stderr, "Check sum mismatch: computed %d, header %d\n", check, resp->header.check);
		}
		fprintf(stderr, "Error: %s\n", resp->error_message);
	} else {
		fprintf(stderr, "Unknown response status: %d\n", resp->header.status);
	}
}

int main(int argc, char *argv[])
{
	putenv("TZ=US/Eastern");
	tzset();
	char* host = NULL;
	char* port = NULL;
	if (argc == 3){
		host = argv[1];
		port = argv[2];
	} else	{
		fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
		return 1;
	}

	while(1){
		char cmd[256] = {0};
		struct request_msg req = {0};

		// print the prompt
		printf("$ ");

        // read the user input
        if (!fgets(cmd, 256, stdin)){
            // eof
            break;
		}
		// remove the newline character
		cmd[strcspn(cmd, "\n")] = 0;

		if (parse_req(cmd, &req)){
			fprintf(stderr, "Invalid command: %s\n", cmd);
			fprintf(stderr, "Commands:\n");
			fprintf(stderr, "  show tracks <num>\n");
			fprintf(stderr, "  show albums <num>\n");
			fprintf(stderr, "  show artists <num>\n");
	
			fprintf(stderr, "  search tracks <str>\n");
			fprintf(stderr, "  search albums <str>\n");
			fprintf(stderr, "  search artists <str>\n");			
			continue;
		}

		int sockfd = connect_to(host, port);
		if (sockfd < 0){
			fprintf(stderr, "Failed to connect to %s:%s\n", host, port);
			return 2;
		}

		if (send(sockfd, &req, sizeof(req), 0) != sizeof(req)) {
			perror("Error sending data");
			close(sockfd);
			return 1;
		}
				
		struct response_msg resp = {0};
		if (recv_response(sockfd, &resp) < 0){
			perror("recv");
			return 4;
		}


		// print the response
		print_response(&resp);

		// Free response
		free_resp(&resp);

		// close the socket
		close(sockfd);
	}
	return 0;
}