#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "sbrowser.h"
#include "slist.h"
#include "htable.h"
#include "spotify.h"
#include "snode.h"

#include <signal.h>
#include <ctype.h>  // for isdigit
#include <stdbool.h>

// TODO: Fix this when have time
// Ensuring graceful shutdown
volatile sig_atomic_t keep_running = 1;  // Global flag to control loop exit

void handle_sigint(int sig) {
    (void)sig;
    printf("\nSIGINT received! Exiting loop...\n");
    keep_running = 0;  // Change flag to break out of loop
}

bool is_all_digits(const char *s) {
    if (s == NULL || *s == '\0') {
        // Empty string or NULL is usually not valid for conversion
        return false;
    }

    while (*s) {
        if (!isdigit((unsigned char)*s)) {
            return false;
        }
        s++;
    }
    return true;
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

unsigned int compute_checksum(void *message, int size, unsigned int seed) {
    unsigned char *data = (unsigned char *)message;
    for (int i = 0; i < size; i++) {
        seed += data[i];
    }
    return seed & 0xffffffff;
}

void construct_err_response(struct response_msg *resp, enum error_type err) {
    resp->header.status = ERROR;
    if (err == CHECK_SUM_ERR) {
        printf("ERROR: Request check sum not equal\n");
        strncpy(resp->error_message, "ERROR: Request check sum not equal", sizeof(resp->error_message) - 1);
    } else if (err == INVALID_CMD_ERR) {
        printf("ERROR: Invalid command\n");
        strncpy(resp->error_message, "ERROR: Invalid command", sizeof(resp->error_message) - 1);
    } else if (err == ZERO_ARGS_ERR) {
        printf("ERROR: Zero arguments received\n");
        strncpy(resp->error_message, "ERROR: Zero arguments received", sizeof(resp->error_message) - 1);
    } 
    // TODO: uncomment this for no results err
    // else if (err == NO_RESULTS_ERR) {
    //     printf("ERROR: No search results found\n");
    //     strncpy(resp->error_message, "ERROR: No search results found", sizeof(resp->error_message) - 1);} 
    else {
        strncpy(resp->error_message, "ERROR: Unknown error", sizeof(resp->error_message) - 1);
    }
    resp->error_message[sizeof(resp->error_message) - 1] = '\0';

    // Ensure the header is fully zeroed first (if needed)
    memset(&resp->header, 0, sizeof(resp->header));
    resp->header.status = ERROR;  // set again after zeroing
    // Optionally, set num_tracks/num_albums/num_playlists to 0 if not used.
    resp->header.num_tracks = 0;
    resp->header.num_albums = 0;
    resp->header.num_playlists = 0;

    // Now compute checksum for error response.
    resp->header.check = 0;
    resp->header.check = compute_checksum(&resp->header, sizeof(resp->header), 1337);
    resp->header.check += compute_checksum(resp->error_message, sizeof(resp->error_message), resp->header.check);
}


void construct_ok_response(struct response_msg *resp, enum command_id cmd, char *args,  
    struct htable *tracks, struct htable *albums, struct htable *playlists, 
    struct htable *track_by_album, struct htable *album_by_track, struct htable *album_by_artist, struct htable *track_by_playlist) {
        resp->header.status = OK;
        if (cmd == SHOW_TRACKS) {
            if (!is_all_digits(args)) {
                construct_err_response(resp, UNKNOWN_ERR);
                return;
            }

            // Count number of tracks
            uint32_t num = atoi(args);
            if (num > htable_num_elems(tracks)) {
                num = htable_num_elems(tracks);
            }

            // Allocate memory in response
            resp->data.tracks = (struct track *)malloc(num * sizeof(struct track));
            
            struct track **track_array = (struct track **)htable_values(tracks);
            qsort(track_array, htable_num_elems(tracks), sizeof(struct track *), track_sort);

            for (uint32_t i = 0; i < num; i++) {
                resp->data.tracks[i] = *(track_array[i]);
            }

            free(track_array);

            resp->header.num_tracks = num;
            resp->header.num_albums = 0;
            resp->header.num_playlists = 0;

            return;
        }

        if (cmd == SHOW_ALBUMS) {
            if (!is_all_digits(args)) {
                construct_err_response(resp, UNKNOWN_ERR);
                return;
            }

            // Count number of tracks
            uint32_t num = atoi(args);
            if (num > htable_num_elems(albums)) {
                num = htable_num_elems(albums);
            }

            // Allocate memory in response
            resp->data.albums = (struct album *)malloc(num * sizeof(struct album));

            struct album **album_array = (struct album **)htable_values(albums);
            qsort(album_array, htable_num_elems(albums), sizeof(struct album *), album_sort);

            for (uint32_t i = 0; i < num; i++) {
                resp->data.albums[i] = *(album_array[i]);
            }

            free(album_array);

            resp->header.num_tracks = 0;
            resp->header.num_albums = num;
            resp->header.num_playlists = 0;

            return;
        
        }

        if (cmd == SHOW_PLAYLISTS) {
            if (!is_all_digits(args)) {
                construct_err_response(resp, UNKNOWN_ERR);
                return;
            }

            // Count number of tracks
            uint32_t num = atoi(args);
            if (num > htable_num_elems(playlists)) {
                num = htable_num_elems(playlists);
            }

            // Allocate memory in response
            resp->data.playlists = (struct playlist *)malloc(num * sizeof(struct playlist));

            struct playlist **playlist_array = (struct playlist **)htable_values(playlists);
            qsort(playlist_array, htable_num_elems(playlists), sizeof(struct playlist *), playlist_sort);

            for (uint32_t i = 0; i < num; i++) {
                resp->data.playlists[i] = *(playlist_array[i]);
            }

            free(playlist_array);

            resp->header.num_tracks = 0;
            resp->header.num_albums = 0;
            resp->header.num_playlists = num;

            return;
        }

        if (cmd == SEARCH_TRACKS) {
            // Create keyword/args
            strcaps(args);

            // Make array from htable
            void **arr = htable_values(tracks);

            // Sort arr
            qsort(arr, htable_num_elems(tracks), sizeof(struct track *), track_sort);

            // Create list and add track ids of tracks that contain keyword
            struct slist *search_res = slist_create();
            for (uint32_t i = 0; i < htable_num_elems(tracks); i++) {
                struct track *candidate = (struct track *)arr[i];
                char *candidate_keyword = strdup(candidate->name);
                strcaps(candidate_keyword);

                // Check if args exists in candidate_keyword
                if (strstr(candidate_keyword, args) != NULL) {
                    slist_add_back(search_res, candidate->track_id);
                }
                free(candidate_keyword);
            }

            free(arr);

            // Add search_res into resp
            uint32_t matched_count = slist_num_elems(search_res);
            // TODO: uncomment for no results err
            // if (matched_count == 0) {
            //     slist_destroy(search_res, 0);
            //     construct_err_response(resp, NO_RESULTS_ERR);
            //     return;
            // }
            resp->header.num_tracks = (int)matched_count;


            if (matched_count > 0) {
                resp->data.tracks = malloc(matched_count * sizeof(struct track));
            } else {
                resp->data.tracks = NULL;
            }

            int total_albums = 0;

            // Just walk the search_res list and sum up the album list lengths
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *track_id = (char *)node->data;
                struct slist *album_list = (struct slist *)htable_find(album_by_track, track_id);
                if (album_list) {
                    total_albums += slist_num_elems(album_list);
                }
            }

            resp->header.num_albums = total_albums;
            if (total_albums > 0) {
                resp->data.albums = malloc(total_albums * sizeof(struct album));
            } else {
                resp->data.albums = NULL;
            }

            // Fill response (second pass)
            uint32_t track_index = 0;
            uint32_t album_index = 0;
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *track_id = (char *)node->data;
                // Find the track by ID
                struct track *track_ptr = (struct track *)htable_find(tracks, track_id);
                if (!track_ptr) {
                    continue;
                }

                // Shallow-copy the track into the response
                resp->data.tracks[track_index++] = *track_ptr;

                // Now find all the albums for this track
                struct slist *album_list = (struct slist *)htable_find(album_by_track, track_id);
                if (!album_list) {
                    // No albums for this track
                    continue;
                }

                // Copy each album into resp->data.albums
                struct snode *a = album_list->front;
                while (a) {
                    char *album_id = (char *)a->data;
                    struct album *album_ptr = (struct album *)htable_find(albums, album_id);
                    if (album_ptr) {
                        resp->data.albums[album_index++] = *album_ptr;
                    }
                    a = a->next;
                }
            }
            
            // Clean up the search_res list
            slist_destroy(search_res, 0);

            // Fill other header fields as needed
            resp->header.num_playlists = 0;  // or set appropriately if needed
            resp->header.check = 0;          // reset before computing

            return;
        }

        if (cmd == SEARCH_ALBUMS) {
            // Uppercase the search keyword in 'keyword'
            strcaps(args);

            // Build a sorted array of all album pointers
            void **arr = htable_values(albums);
            qsort(arr, htable_num_elems(albums), sizeof(struct album *), album_sort);

            // Collect the IDs of all albums whose names contain 'keyword'
            struct slist *search_res = slist_create();
            for (uint32_t i = 0; i < htable_num_elems(albums); i++) {
                struct album *candidate = (struct album *)arr[i];

                // Make an uppercase copy of the album's name for case-insensitive search
                char *candidate_keyword = strdup(candidate->name);
                strcaps(candidate_keyword);

                if (strstr(candidate_keyword, args) != NULL) {
                    // If the album name contains the keyword, store its album_id
                    slist_add_back(search_res, candidate->album_id);
                }
                free(candidate_keyword);
            }

            free(arr); // Done with the array from htable_values

            // Now we know how many albums matched
            uint32_t matched_count = slist_num_elems(search_res);
            // TODO: uncomment for no results err
            // if (matched_count == 0) {
            //     slist_destroy(search_res, 0);
            //     construct_err_response(resp, NO_RESULTS_ERR);
            //     return;
            // }
            resp->header.num_albums = matched_count;

            // Allocate array for matched albums
            if (matched_count > 0) {
                resp->data.albums = malloc(matched_count * sizeof(struct album));
            } else {
                resp->data.albums = NULL;
            }

            // We'll also need to gather *all tracks* for the matched albums
            // First pass: count how many tracks total we'll need
            uint32_t total_tracks = 0;

            // Walk the matched album list and count all the tracks from track_by_album
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *album_id = (char *)node->data;
                // Find the list of track IDs associated with this album
                struct slist *track_list = (struct slist *)htable_find(track_by_album, album_id);
                if (track_list) {
                    total_tracks += slist_num_elems(track_list);
                }
            }

            // Allocate array for those tracks
            resp->header.num_tracks = total_tracks;
            if (total_tracks > 0) {
                resp->data.tracks = malloc(total_tracks * sizeof(struct track));
            } else {
                resp->data.tracks = NULL;
            }

            // Second pass: fill in albums and tracks
            uint32_t album_index = 0;
            uint32_t track_index = 0;
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *album_id = (char *)node->data;
                // Find the album in the hashtable
                struct album *album_ptr = (struct album *)htable_find(albums, album_id);
                if (!album_ptr) {
                    // Shouldn't happen if your hashtable is consistent
                    continue;
                }

                // Shallow-copy the album into resp->data.albums
                resp->data.albums[album_index++] = *album_ptr;

                // Now gather the tracks for this album
                struct slist *track_list = (struct slist *)htable_find(track_by_album, album_id);
                if (!track_list) {
                    // No tracks for this album
                    continue;
                }

                // Use this if want to output sorted tracks for each album (not overall sorted tracks)
                // Sort the track IDs if you want them in a particular order
                char **temp = (char **)slist_to_array(track_list);
                qsort(temp, track_list->counter, sizeof(char *), compare_str_ptr);

                // Copy each track into resp->data.tracks
                for (uint32_t i = 0; i < track_list->counter; i++) {
                    char *track_id = temp[i];
                    struct track *track_ptr = (struct track *)htable_find(tracks, track_id);
                    if (track_ptr) {
                        resp->data.tracks[track_index++] = *track_ptr;
                    }
                }
                free(temp);
            }

            // Sort tracks (overall) -> remove if not want to sort overall (output -> sorted for each album)
            qsort(resp->data.tracks, total_tracks, sizeof(struct track), track_sort_flat);

            // Clean up the matched album list
            slist_destroy(search_res, 0);

            // If have playlists or other fields, set them or set them to zero
            resp->header.num_playlists = 0; // or a real value if you need

            return;
        }

        if (cmd == SEARCH_ARTISTS) {
            strcaps(args);

            // Build a sorted array of all track pointers (we’ll group by 'artist')
            uint32_t total_tracks = htable_num_elems(tracks);
            void **arr = htable_values(tracks);

            // sort
            qsort(arr, total_tracks, sizeof(struct track *), artist_sort);

            // Collect unique artist names that match 'keyword'
            struct slist *search_res = slist_create();  // will hold unique artist strings
            for (uint32_t i = 0; i < total_tracks; i++) {
                struct track *candidate = (struct track *)arr[i];

                // Uppercase a copy of candidate->artist
                char *candidate_keyword = strdup(candidate->artist);
                strcaps(candidate_keyword);

                // If the artist name contains the keyword, and we haven’t already added them
                // (slist_find_value(...) checks for duplicates)
                if (strstr(candidate_keyword, args) != NULL &&
                    !slist_find_value(search_res, candidate->artist, compare_str)) {
                    slist_add_back(search_res, candidate->artist);
                }
                free(candidate_keyword);
            }

            free(arr); // Done with the array from htable_values()

            // TODO: uncomment for no results err
            // if (slist_num_elems(search_res) == 0) {
            //     slist_destroy(search_res, 0);
            //     construct_err_response(resp, NO_RESULTS_ERR);
            //     return;
            // }

            // For each matched artist, we’ll gather all of their albums.
            //    First, figure out how many total albums we need.

            uint32_t total_albums = 0;
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *artist_name = (char *)node->data;
                // album_by_artist is presumably: key=artist_name, value=slist of album_ids
                struct slist *album_list = (struct slist *)htable_find(album_by_artist, artist_name);
                if (album_list) {
                    total_albums += slist_num_elems(album_list);
                }
            }

            // Fill response header fields
            resp->header.num_albums = total_albums;

            // Allocate the albums array
            if (total_albums > 0) {
                resp->data.albums = malloc(total_albums * sizeof(struct album));
            } else {
                resp->data.albums = NULL;
            }

            // Second pass: copy each matched artist’s albums into resp->data.albums
            uint32_t album_index = 0;
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *artist_name = (char *)node->data;

                // Get the list of album IDs for this artist
                struct slist *album_list = (struct slist *)htable_find(album_by_artist, artist_name);
                if (!album_list) {
                    // No albums for this artist
                    continue;
                }

                // Use this if want to output sorted albums for each artist (not overall sorted albums)
                // Convert album_list to an array for sorting by album ID 
                char **temp = (char **)slist_to_array(album_list);
                qsort(temp, album_list->counter, sizeof(char *), compare_str_ptr);

                // Copy each album struct
                for (uint32_t i = 0; i < album_list->counter; i++) {
                    // album_list holds album IDs (char *). We find the actual album struct
                    struct album *alb_ptr = (struct album *)htable_find(albums, temp[i]);
                    if (alb_ptr) {
                        // Shallow copy album into resp->data.albums
                        resp->data.albums[album_index++] = *alb_ptr;
                    }
                }
                free(temp);
            }

            // Sort albums (overall) -> remove if not want to sort overall (output -> sorted for each artist)
            qsort(resp->data.albums, total_albums, sizeof(struct album), album_sort_flat);

            // Clean up the slist of artist names
            slist_destroy(search_res, 0); // we didn’t allocate the artist strings; they’re from your track->artist

            resp->header.num_tracks = 0;       // No tracks in this response
            resp->header.num_playlists = 0;    // No playlists in this response
            
            return;
        }

        if (cmd == SEARCH_PLAYLISTS) {
            strcaps(args);

            // Build a sorted array of all playlist pointers
            void **arr = htable_values(playlists);
            qsort(arr, htable_num_elems(playlists), sizeof(struct playlist *), playlist_sort);

            // Collect the IDs of all playlists whose names contain 'keyword'
            struct slist *search_res = slist_create();
            for (uint32_t i = 0; i < htable_num_elems(playlists); i++) {
                struct playlist *candidate = (struct playlist *)arr[i];

                // Make an uppercase copy of the candidate's name
                char *candidate_keyword = strdup(candidate->name);
                strcaps(candidate_keyword);

                if (strstr(candidate_keyword, args) != NULL) {
                    // If the playlist name contains the keyword, store its ID
                    slist_add_back(search_res, candidate->playlist_id);
                }
                free(candidate_keyword);
            }

            free(arr); // Done with the array from htable_values

            // Number of matched playlists
            uint32_t matched_count = slist_num_elems(search_res);
            if (matched_count == 0) {
                slist_destroy(search_res, 0);
                construct_err_response(resp, NO_RESULTS_ERR);
                return;
            }
            resp->header.num_playlists = matched_count;

            // Allocate array for these matching playlists
            if (matched_count > 0) {
                resp->data.playlists = malloc(matched_count * sizeof(struct playlist));
            } else {
                resp->data.playlists = NULL;
            }

            // We also need to gather all tracks for the matched playlists.
            //    First pass: count total needed.
            uint32_t total_tracks = 0;
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *playlist_id = (char *)node->data;
                // Find the list of track IDs associated with this playlist
                struct slist *track_list = (struct slist *)htable_find(track_by_playlist, playlist_id);
                if (track_list) {
                    total_tracks += slist_num_elems(track_list);
                }
            }

            // Allocate array for those tracks
            resp->header.num_tracks = total_tracks;
            if (total_tracks > 0) {
                resp->data.tracks = malloc(total_tracks * sizeof(struct track));
            } else {
                resp->data.tracks = NULL;
            }

            // Fill in resp->data.playlists and resp->data.tracks
            uint32_t playlist_index = 0;
            uint32_t track_index = 0;
            for (struct snode *node = search_res->front; node != NULL; node = node->next) {
                char *playlist_id = (char *)node->data;
                // Find the playlist in the hashtable
                struct playlist *playlist_ptr = (struct playlist *)htable_find(playlists, playlist_id);
                if (!playlist_ptr) {
                    // Shouldn't happen if hashtable is consistent, but be safe
                    continue;
                }

                // Shallow-copy the playlist into resp->data.playlists
                resp->data.playlists[playlist_index++] = *playlist_ptr;

                // Now gather the tracks for this playlist
                struct slist *track_list = (struct slist *)htable_find(track_by_playlist, playlist_id);
                if (!track_list) {
                    continue;
                }

                // Sort the track IDs if you want them in a particular order (like compare_str_ptr)
                char **temp = (char **)slist_to_array(track_list);
                qsort(temp, track_list->counter, sizeof(char *), compare_str_ptr);

                // Copy each track into resp->data.tracks
                for (uint32_t i = 0; i < track_list->counter; i++) {
                    char *track_id = temp[i];
                    struct track *track_ptr = (struct track *)htable_find(tracks, track_id);
                    if (track_ptr) {
                        resp->data.tracks[track_index++] = *track_ptr;
                    }
                }
                free(temp);
            }

            // Clean up
            slist_destroy(search_res, 0);

            // If you have albums, set resp->header.num_albums = 0 or fill accordingly
            resp->header.num_albums = 0; 

            return;
        }
    }

#include <arpa/inet.h>  // Required for `inet_pton()`

int main(int argc, char *argv[]) {
    putenv("TZ=US/Eastern");
	tzset();
    // Validate arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <FILE_NAME> <PORT>\n", argv[0]);
        return 1;
    }

    // Convert port argument to integer
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
        return 1;
    }

    // SET UP DATA ==========================================================================================
    FILE *file;
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 2;
    }

    // Create the data structures
    struct slist *plays = slist_create();
    struct htable *tracks = htable_create(1024);
    struct htable *albums = htable_create(1024);
    struct htable *playlists = htable_create(128);
    if (!plays || !tracks || !albums || !playlists) {
        perror("Error creating data.");
        return 3;
    }

    // Read data from the file
    read_csv_to_lists(file, plays, tracks, albums, playlists);
    fclose(file);

    // Create lookup tables
    struct htable *track_by_album = create_track_by_album(plays);
    struct htable *album_by_track = create_album_by_track(plays);
    struct htable *album_by_artist = create_album_by_artist(plays, tracks);
    struct htable *track_by_playlist = create_track_by_playlist(plays);

    if (!track_by_album || !track_by_playlist || !album_by_track || !album_by_artist) {
        perror("Error creating lookup tables.");
        return 1;
    }

    enum command_id local_cmd;
    char local_args[256] = {0};

    // SET UP SERVER SOCKET ================================================================================
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Set up address struct
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port); // Convert to network byte order

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on localhost:%d\n", port);

    while (keep_running) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        while (1) {
            // Read the request
            struct request_msg request = {0};
            int read_size = recv(new_socket, &request, sizeof(request), 0);
            if (read_size == 0) {
                // The client has gracefully closed the connection.
                printf("Client disconnected.\n");
                close(new_socket);
                break;  // Exit the inner loop to accept a new connection.
            } else if (read_size < 0) {
                perror("recv failed");
                close(new_socket);
                close(server_fd);
                exit(EXIT_FAILURE);
            }

            // Copy over command and args to parse
            local_cmd = request.command;
            strncpy(local_args, clean_str(request.args), sizeof(local_args) - 1);
            local_args[sizeof(local_args) - 1] = '\0';  // Ensure null termination

            // Initialize response data
            struct response_msg resp = {0};

            // Checksum check
            struct request_msg temp = request;
            temp.check = 0;  // Zero out the checksum field before computing
            unsigned int check = compute_checksum(&temp, sizeof(temp), 7331);
            if (check != request.check) {
                construct_err_response(&resp, CHECK_SUM_ERR);
            }


            // Validate command
            else if (local_cmd < SHOW_TRACKS || local_cmd > QUIT) {
                construct_err_response(&resp, INVALID_CMD_ERR);
            }

            // TODO: Fix this later
            // Check if args are empty
            // else if (strlen(local_args) == 0) {
            //     construct_err_response(&resp, ZERO_ARGS_ERR);
            // }

            // Handle QUIT command
            else if (local_cmd == QUIT) {
                close(new_socket);
                keep_running = 0;
                break;
            } else if (SHOW_TRACKS <= local_cmd && local_cmd < QUIT) {
                construct_ok_response(&resp, local_cmd, local_args, tracks, albums, playlists, track_by_album, album_by_track, album_by_artist, track_by_playlist);
            } else {
                construct_err_response(&resp, UNKNOWN_ERR);
            }

            // Compute the checksum exactly as in the demo
            // Zero out the check field before computing the checksum.
            if (resp.header.status == OK) {
                resp.header.check = 0;
                unsigned int checksum = compute_checksum(&resp.header, sizeof(resp.header), 1337);
                checksum += compute_checksum(resp.data.tracks, resp.header.num_tracks * sizeof(struct track), checksum);
                checksum += compute_checksum(resp.data.albums, resp.header.num_albums * sizeof(struct album), checksum);
                checksum += compute_checksum(resp.data.playlists, resp.header.num_playlists * sizeof(struct playlist), checksum);
                resp.header.check = checksum;
            }
            

            // Send the resp header
            if (send(new_socket, &resp.header, sizeof(resp.header), 0) != sizeof(resp.header)) {
                perror("send header failed");
                // Handle error...
            }

            // Depending on the status, send either error message or the data arrays
            if (resp.header.status == ERROR) {
                if (send(new_socket, resp.error_message, sizeof(resp.error_message), 0) != sizeof(resp.error_message)) {
                    perror("send error message failed");
                    // Handle error...
                }
            } else if (resp.header.status == OK) {
                if (resp.header.num_tracks > 0) {
                    if (send(new_socket, resp.data.tracks, sizeof(struct track) * resp.header.num_tracks, 0)
                        != (ssize_t)(sizeof(struct track) * resp.header.num_tracks)) {
                        perror("send tracks failed");
                        // Handle error...
                    }
                }
                if (resp.header.num_albums > 0) {
                    if (send(new_socket, resp.data.albums, sizeof(struct album) * resp.header.num_albums, 0)
                        != (ssize_t)(sizeof(struct album) * resp.header.num_albums)) {
                        perror("send albums failed");
                        // Handle error...
                    }
                }
                if (resp.header.num_playlists > 0) {
                    if (send(new_socket, resp.data.playlists, sizeof(struct playlist) * resp.header.num_playlists, 0)
                        != (ssize_t)(sizeof(struct playlist) * resp.header.num_playlists)) {
                        perror("send playlists failed");
                        // Handle error...
                    }
                }
            }


            printf("Response sent.\n");
            free_resp(&resp);
        }
        printf("Disconnecting\n");
        
        close(new_socket);
    }
    close(server_fd);

    // Clean up allocated memory
    char **values1 = (char **)htable_values(tracks);
    char **values2 = (char **)htable_values(albums);
    char **values3 = (char **)htable_values(playlists);

    for (uint32_t i = 0; i < htable_num_elems(tracks); i++) {		
		free(values1[i]);
	}
    for (uint32_t i = 0; i < htable_num_elems(albums); i++) {		
		free(values2[i]);
	}
    for (uint32_t i = 0; i < htable_num_elems(playlists); i++) {		
		free(values3[i]);
	}

    free(values1);
    free(values2);
    free(values3);

    slist_destroy(plays, 1);
    htable_destroy(tracks);
    htable_destroy(albums);
    htable_destroy(playlists);

    htable_destroy_slist_values(track_by_album);
    htable_destroy_slist_values(album_by_track);
    htable_destroy_slist_values(album_by_artist);
    htable_destroy_slist_values(track_by_playlist);

    return 0;
}
