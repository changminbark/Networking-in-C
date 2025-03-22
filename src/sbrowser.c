/**
 * @file sbrowser.c
 * @author Chang Min Bark (cb073@bucknell.edu)
 * @brief Spotify data browser.
 * @version 0.1
 * @date 2025-01-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "snode.h"
#include "slist.h"
#include "htable.h"
#include "spotify.h"
#include "sbrowser.h"


#define MAX_LINE_LENGTH 1024 

/** 
 * Compares two strings passed as void pointers useful for qsort.
 */
int compare_str_ptr(const void *a, const void *b) {     
    return strcmp( *(const char **) a, *(const char **) b);
}

/**
 * Compares two strings useful for searching keys/values.
 */
int compare_str(const void *a, const void *b) {     
    return strcmp( (const char *) a, (const char *) b);
}

/**
 * Capitalizes a string in place.
 */
void strcaps(char *str) {
    while(*str) {
        *str = toupper(*str);
        str++;
    }
}

void remove_first_and_last_char(char *str) {
    size_t len = strlen(str);

    // Ensure that the string is at least 2 characters long and starts/ends with a quote
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        memmove(str, str + 1, len - 1);  // Shift left to remove the first char
        str[len - 2] = '\0';  // Null-terminate to remove the last char
    }
}

void htable_destroy_slist_values(struct htable *ht) {
    if (!ht) {
        printf("Error: htable is NULL.\n");
        return;
    }

    struct htable_iter *iter = htable_create_iter(ht);
    struct kv_pair *kv;

    while ((kv = htable_iter_next(iter)) != NULL) {
        struct slist *list = (struct slist *)kv->value;
        if (list) {
            slist_destroy(list, 0);  // Destroy the list and free its elements
        }
    }

    htable_destroy_iter(iter);
    htable_destroy(ht);
}

/**
 * Load the CSV (copy from Lab02)
 */
int read_csv_to_lists (FILE *file, struct slist *plays, struct htable *tracks, struct htable *albums, struct htable *playlists) {
    char line[MAX_LINE_LENGTH];
    int line_count = 0;

    if (!plays || !tracks || !albums || !playlists) {
        perror("Error: data structures are invalid.");
        return -1;
    }

    if (plays == NULL) {
        perror("Error: lists not allocated.");
        return -1;
    }

    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {

        // remove the newline character
        line[strcspn(line, "\n")] = 0;

        // ignore the header
        if (line_count > 0){
            struct play *p = malloc(sizeof(struct play));
            struct track *t = malloc(sizeof(struct track));
            struct album *a = malloc(sizeof(struct album));
            struct playlist *pl = malloc(sizeof(struct playlist));

            if (parse_line(line, p, t, a, pl) == 0) {
                fprintf(stderr, "Error parsing line %d\n", line_count);
                exit(-1);
            } else {
                
                slist_add_back(plays, p);

                if (htable_find(tracks, p->track_id) == NULL) {
                    htable_insert(tracks, p->track_id, t);
                } else {
                    free(t);
                }
                
                if (htable_find(albums, p->album_id) == NULL) {
                    htable_insert(albums, p->album_id, a);
                } else {
                    free(a);
                }
                
                if (htable_find(playlists, p->playlist_id) == NULL) {
                    htable_insert(playlists, p->playlist_id, pl);
                } else {
                    free(pl);
                }                
// uncomment this block to print the data
// #define DEBUG_PRINT
#ifdef DEBUG_PRINT
                struct tm *release_date = localtime(&a->release_date);
                char release_date_str[64];

                strftime(release_date_str, sizeof(release_date_str), "%Y-%m-%d", release_date);

                printf("Track ID: %s\n", p->track_id);
                printf("Album ID: %s\n", a->album_id);
                printf("Playlist ID: %s\n", p->playlist_id);

                printf("Track Name: %s\n", t->name);
                printf("Artist: %s\n", t->artist);
                printf("Album Name: %s\n", a->name);
                printf("Release Date: %s\n", release_date_str);
                printf("Playlist Name: %s\n", pl->name);
                printf("Genre: %s\n", pl->genre);
                printf("Subgenre: %s\n", pl->subgenre);

                printf("Popularity: %d\n", t->popularity);

                printf("Danceability: %f\n", t->danceability);
                printf("Energy: %f\n", t->energy);
                printf("Key: %f\n", t->key);
                printf("Loudness: %f\n", t->loudness);
                printf("Speechiness: %f\n", t->speechiness);
                printf("Acousticness: %f\n", t->acousticness);
                printf("Instrumentalness: %f\n", t->instrumentalness);
                printf("Liveness: %f\n", t->liveness);
                printf("Valence: %f\n", t->valence);
                printf("Tempo: %f\n", t->tempo);
                printf("Duration: %d\n", t->duration_ms);
#endif
            }

        }
        line_count++;
    }
    return line_count - 1;
}

/**
 * Prints some basic statics about the data. (from lab02)
 */
void print_stats (struct slist *plays, struct htable *tracks, struct htable *albums, struct htable *playlists) {
    printf("--- Statistics ---\n");
    printf("Number of plays: %d\n", slist_num_elems(plays));
    printf("Number of tracks: %d\n", htable_num_elems(tracks));
    printf("Number of albums: %d\n", htable_num_elems(albums));
    printf("Number of playlists: %d\n", htable_num_elems(playlists));
}

// SORT FUNCTIONS ============================================
int track_sort(const void *a, const void *b) {
    struct track *t1 = *(struct track **)a;
    struct track *t2 = *(struct track **)b;
    return strcmp(t1->track_id, t2->track_id);
}

int track_sort_flat(const void *a, const void *b) {
    struct track *t1 = (struct track *)a;
    struct track *t2 = (struct track *)b;
    return strcmp(t1->track_id, t2->track_id);
}

int album_sort(const void *a, const void *b) {
    struct album *t1 = *(struct album **)a;
    struct album *t2 = *(struct album **)b;
    return strcmp(t1->album_id, t2->album_id);
}

int album_sort_flat(const void *a, const void *b) {
    struct album *t1 = (struct album *)a;
    struct album *t2 = (struct album *)b;
    return strcmp(t1->album_id, t2->album_id);
}

int playlist_sort(const void *a, const void *b) {
    struct playlist *t1 = *(struct playlist **)a;
    struct playlist *t2 = *(struct playlist **)b;
    return strcmp(t1->playlist_id, t2->playlist_id);
}

int artist_sort(const void *a, const void *b) {
    struct track *t1 = *(struct track **)a;
    struct track *t2 = *(struct track **)b;
    return strcmp(t1->artist, t2->artist);
}

// PRINT FUNCTIONS ============================================

// print track, album, playlist found in spotify.c file

void print_artist(char *artist) {
    printf("[ARTIST] %s\n", artist);
}

/**
 * Prints at most <num> tracks from the hash table sorted by the sort function. 
 */
void print_tracks(struct htable* tracks, uint32_t num, int(*sort)(const void*, const void*)) {
    // HINT: you should use the print_track function!
    if (!tracks){
        printf("Error: print_tracks, tracks is NULL.");
    }

    if (num > htable_num_elems(tracks)) {
        num = htable_num_elems(tracks);
    }

    struct track **track_array = (struct track **)htable_values(tracks);
    qsort(track_array, htable_num_elems(tracks), sizeof(struct track *), sort);

    for (uint32_t i = 0; i < num; i++) {
        print_track(track_array[i]);
    }

    free(track_array);

    return;
}

void print_albums(struct htable* albums, uint32_t num, int(*sort)(const void*, const void*)) {
    // HINT: you should use the print_album function!
    if (!albums){
        printf("Error: print_albums, albums is NULL.");
    }


    if (num > htable_num_elems(albums)) {
        num = htable_num_elems(albums);
    }

    struct album **album_array = (struct album **)htable_values(albums);
    qsort(album_array, htable_num_elems(albums), sizeof(struct album *), sort);

    for (uint32_t i = 0; i < num; i++) {
        print_album(album_array[i]);
    }

    free(album_array);
}

void print_playlists(struct htable* playlists, uint32_t num, int(*sort)(const void*, const void*)) {
    // HINT: you should use the print_playlist function!
    if (!playlists){
        printf("Error: print_playlists, playlists is NULL.");
    }
    if (num > htable_num_elems(playlists)) {
        num = htable_num_elems(playlists);        
    }


    struct playlist **playlist_array = (struct playlist **)htable_values(playlists);
    qsort(playlist_array, htable_num_elems(playlists), sizeof(struct playlist *), sort);

    for (uint32_t i = 0; i < num; i++) {
        print_playlist(playlist_array[i]);
    }

    free(playlist_array);
}

int print_track_list(struct slist *track_ids, struct htable *tracks, struct htable *album_by_track, 
    struct htable *albums, uint32_t *album_count) {
    if (!track_ids || !tracks) {
        printf("Error: print_track_list, list or tracks is NULL.");
        return 0;
    }
    // and return the number of tracks printed.
    // HINT: you should use the print_track function!
    int count = 0;
    struct snode *track_node = track_ids->front;

    while (track_node) {
        // Find track id and print it
        char *track_id = (char *)track_node->data;
        struct track *track = (struct track *)htable_find(tracks, track_id);
        print_track(track);

        // Go through list and print album (no need sort)
        struct slist *album_list = (struct slist *)htable_find(album_by_track, track_id);
        struct snode *a = album_list->front;
        while (a != NULL) {
            char *album_id = (char *)a->data;
            struct album *album = (struct album *)htable_find(albums, album_id);
            print_album(album);
            (*album_count)++;
            
            a = a->next;
        }
        count++;
        track_node = track_node->next;
    }


    return count;
}

int print_album_list(struct slist *album_ids, struct htable *albums, struct htable *track_by_album, 
    struct htable *tracks, uint32_t *track_count) {
    if (!album_ids || !albums || !track_by_album || !tracks || !track_count) {
        printf("Error: print_album_list, one or more arguments are NULL.\n");
        return 0;
    }

    int count = 0;
    struct snode *album_node = album_ids->front;

    while (album_node) {
        char *album_id = (char *)album_node->data;
        struct album *album = (struct album *)htable_find(albums, album_id);
        print_album(album);

        // Create sorted array of track id's
        struct slist *track_list = (struct slist *)htable_find(track_by_album, album_id);
        (*track_count) += track_list->counter;
        char **temp = (char **)slist_to_array(track_list);
        qsort(temp, track_list->counter, sizeof(char *), compare_str_ptr);

        // Loop through sorted array and print tracks
        for (uint32_t i = 0; i < track_list->counter; i++) {
            struct track *track = (struct track *)htable_find(tracks, temp[i]);
            if (track) {
                print_track(track);
            }
        }
        
        free(temp);
        count++;
        album_node = album_node->next;
    }

    return count;
}

int print_playlist_list(struct slist *playlist_ids, struct htable *playlists, struct htable *track_by_playlist, 
    struct htable *tracks, uint32_t *track_count) {
    if (!playlist_ids || !playlists || !track_by_playlist || !tracks || !track_count) {
        printf("Error: print_playlist_list, one or more arguments are NULL.\n");
        return 0;
    }

    int count = 0;
    struct snode *playlist_node = playlist_ids->front;

    while (playlist_node) {
        char *playlist_id = (char *)playlist_node->data;
        struct playlist *playlist = (struct playlist *)htable_find(playlists, playlist_id);
        print_playlist(playlist);

        // Create sorted array of track id's
        struct slist *track_list = (struct slist *)htable_find(track_by_playlist, playlist_id);
        (*track_count) += track_list->counter;
        char **temp = (char **)slist_to_array(track_list);
        qsort(temp, track_list->counter, sizeof(char *), compare_str_ptr);

        // Loop through sorted array and print tracks
        for (uint32_t i = 0; i < track_list->counter; i++) {
            struct track *track = (struct track *)htable_find(tracks, temp[i]);
            if (track) {
                print_track(track);
            }
        }
        
        free(temp);
        count++;
        playlist_node = playlist_node->next;
    }

    return count;
}

int print_artist_list(struct slist *artist_names, struct htable *album_by_artist, 
    struct htable *albums, uint32_t *album_count) {
    if (!artist_names || !album_by_artist || !albums || !album_count) {
        printf("Error: print_artist_list, one or more arguments are NULL.\n");
        return 0;
    }

    int count = 0;
    struct snode *artist_node = artist_names->front;

    while (artist_node) {
        char *artist_name = (char *)artist_node->data;
        print_artist(artist_name);

        // Create sorted array of album id's
        struct slist *album_list = (struct slist *)htable_find(album_by_artist, artist_name);
        (*album_count) += album_list->counter;
        char **temp = (char **)slist_to_array(album_list);
        qsort(temp, album_list->counter, sizeof(char *), compare_str_ptr);

        // Loop through sorted array and print albums
        for (uint32_t i = 0; i < album_list->counter; i++) {
            struct album *album = (struct album *)htable_find(albums, temp[i]);
            if (album) {
                print_album(album);
            }
        }
        
        free(temp);
        count++;
        artist_node = artist_node->next;
    }

    return count;
}



// CREATE HTABLE FUNCTIONS ============================================

struct htable* create_track_by_album(struct slist *plays) {
    struct htable *track_by_album = htable_create(512);
    if (!plays || track_by_album == NULL) {
        printf("Error: create_track_by_album, plays or table is NULL.\n");
        return NULL;
    }
    
    struct snode *p = plays->front;
    while (p) {
        struct play *data = (struct play*)p->data;

        // Retrieve existing list or create a new one
        struct slist *track_list = (struct slist*)htable_find(track_by_album, data->album_id);
        if (!track_list) {
            track_list = slist_create();
            htable_insert(track_by_album, data->album_id, track_list);
        }
        // Check if track ID is already in the list before adding
        if (!slist_find_value(track_list, data->track_id, compare_str)) {
            slist_add_back(track_list, data->track_id);
        }

        p = p->next;
    }

    return track_by_album;
}

struct htable* create_album_by_track(struct slist *plays) {
    struct htable *album_by_track = htable_create(512);
    if (!plays || album_by_track == NULL) {
        printf("Error: create_album_by_track, plays or table is NULL.\n");
        return NULL;
    }
    
    struct snode *p = plays->front;
    while (p) {
        struct play *data = (struct play*)p->data;

        // Retrieve existing list or create a new one
        struct slist *album_list = (struct slist*)htable_find(album_by_track, data->track_id);
        if (!album_list) {
            album_list = slist_create();
            htable_insert(album_by_track, data->track_id, album_list);
        }
        // Check if album ID is already in the list before adding
        if (!slist_find_value(album_list, data->album_id, compare_str)) {
            slist_add_back(album_list, data->album_id);
        }

        p = p->next;
    }

    return album_by_track;
}

struct htable* create_album_by_artist(struct slist *plays, struct htable *tracks) {
    struct htable *album_by_artist = htable_create(256);
    if (!plays || !tracks || album_by_artist == NULL) {
        printf("Error: create_album_by_artist, plays, tracks, or table is NULL.\n");
        return NULL;
    }
    
    struct snode *p = plays->front;
    while (p) {
        struct play *data = (struct play *)p->data;
        struct track *t = (struct track *)htable_find(tracks, data->track_id);
        if (!t) {
            p = p->next;
            continue;
        }

        // Retrieve existing list or create a new one
        struct slist *album_list = (struct slist*)htable_find(album_by_artist, t->artist);
        if (!album_list) {
            album_list = slist_create();
            htable_insert(album_by_artist, t->artist, album_list);
        }
        // Check if album ID is already in the list before adding
        if (!slist_find_value(album_list, data->album_id, compare_str)) {
            slist_add_back(album_list, data->album_id);
        }

        p = p->next;
    }

    return album_by_artist;
}

struct htable* create_track_by_playlist(struct slist *plays) {
    struct htable *track_by_playlist = htable_create(128);
    if (!plays || track_by_playlist == NULL) {
        printf("Error: create_track_by_playlist, plays or table is NULL.\n");
        return NULL;
    }
    
    struct snode *p = plays->front;
    while (p) {
        struct play *data = (struct play*)p->data;

        // Retrieve existing list or create a new one
        struct slist *track_list = (struct slist*)htable_find(track_by_playlist, data->playlist_id);
        if (!track_list) {
            track_list = slist_create();
            htable_insert(track_by_playlist, data->playlist_id, track_list);
        }
        // Check if track ID is already in the list before adding
        if (!slist_find_value(track_list, data->track_id, compare_str)) {
            slist_add_back(track_list, data->track_id);
        }

        p = p->next;
    }

    return track_by_playlist;
}



// DRIVER CODE ====================================================

void main_menu(struct slist *plays, struct htable *tracks, struct htable *albums, struct htable *playlists) {
    char cmd[256];
    char *tokens[256];
    
    // here we construct some lookup tables to make searching easier
    // you don't have to use these and you can create alternate 
    // data structures if you prefer. These are just what I used.
    // These avoid the need to iterate over the plays list to find
    // the data you need to answer requests.
    
    // key is album_id, value is slist of track_ids
    struct htable *track_by_album = create_track_by_album(plays);    
    
    // key is track_id, value is slist of album_ids
    struct htable *album_by_track = create_album_by_track(plays);

    // key is artist, value is slist of album_ids
    struct htable *album_by_artist = create_album_by_artist(plays, tracks);

    // key is playlist_id, value is slist of track_ids
    struct htable *track_by_playlist = create_track_by_playlist(plays);
    
    if ((track_by_album == NULL) | (track_by_playlist == NULL) || (album_by_track == NULL) || (album_by_artist == NULL)) {
        perror("Error creating data.");
        return;
    }
    
    while (1) {
        // search stats
        uint32_t album_count = 0;
        uint32_t track_count = 0;
        uint32_t playlist_count = 0;

        // print the prompt
        printf(" [%d, %d, %d, %d]:$ ", 
            slist_num_elems(plays),  
            htable_num_elems(tracks),
            htable_num_elems(albums), 
            htable_num_elems(playlists));

        // read the user input
        if (!fgets(cmd, 256, stdin))
            // eof
            break;
        
        // remove the newline character
        cmd[strcspn(cmd, "\n")] = '\0';

        // tokenize on spaces, allowing quoted strings (use strtok_quotes)
        // store the results in the tokens array
        tokens[0] = strtok(cmd, " ");
        for (int i = 1; i < 256; i++) {
            tokens[i] = strtok_quotes(NULL, " ");
            // don't stop on null to fully initialize the array with NULLs
        }        
        if (!tokens[0])
            continue;

        // process commands in the array of tokens
        if (strncmp(tokens[0], "quit", 256) == 0)
            break;
        else if (strncmp(tokens[0], "show", 256) == 0){    
            int32_t num  = 0;
            if (!tokens[1]){
                printf("Error: show command requires a parameter.\n");
                continue;
            }            
            // parse the optional number of items to show
            if (!tokens[2] || (sscanf(tokens[2], "%d", &num) != 1)){
                num = 10;
            }            
            if (strncmp(tokens[1], "tracks", 256) == 0){
                // print the tracks sorted by TRACK NAME.
                // HINT: you might want to create a compare function that compares track names.
                print_tracks(tracks, num, track_sort);
                printf("Found %d tracks and printed %d.\n", htable_num_elems(tracks), num);
            }
            else if (strncmp(tokens[1], "track", 256) == 0){
                if (!tokens[2]){
                    printf("Error: track command requires a parameter.\n");
                    continue;
                }                

                // Find relevant track ids and put into list and then call print_track_list
                // Create keyword
                char *keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);
                strcaps(keyword);

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

                    // Check if keyword exists in candidate_keyword
                    if (strstr(candidate_keyword, keyword) != NULL) {
                        slist_add_back(search_res, candidate->track_id);
                    }

                    free(candidate_keyword);
                }

                // Print list 
                track_count = print_track_list(search_res, tracks, album_by_track, albums, &album_count);

                // For printing purposes
                free(keyword);
                keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);

                printf("Found %d albums and %d tracks for '%s'.\n", album_count, track_count, keyword);

                // Free data memory
                slist_destroy(search_res, 0);
                free(keyword);
                free(arr);
            }
            else if (strncmp(tokens[1], "artist", 256) == 0){
                if (!tokens[2]){
                    printf("Error: artist command requires a parameter.\n");
                    continue;
                }   
                // Find relevant artist ids and put into list and then call print_artist_list
                // Create keyword
                char *keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);
                strcaps(keyword);

                // Make array from htable
                void **arr = htable_values(tracks);

                // Sort arr
                qsort(arr, htable_num_elems(tracks), sizeof(struct track *), artist_sort);

                // Create list and add artist names that contain keyword
                struct slist *search_res = slist_create();
                for (uint32_t i = 0; i < htable_num_elems(tracks); i++) {
                    struct track *candidate = (struct track *)arr[i];
                    char *candidate_keyword = strdup(candidate->artist);
                    strcaps(candidate_keyword);

                    // Check if keyword exists in candidate_keyword
                    if (strstr(candidate_keyword, keyword) != NULL && 
                    !slist_find_value(search_res, candidate->artist, compare_str)) {
                        slist_add_back(search_res, candidate->artist);
                    }

                    free(candidate_keyword);
                }

                uint32_t artist_count = print_artist_list(search_res, album_by_artist, albums, &album_count);

                // For printing purposes
                free(keyword);
                keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);

                printf("Found %d artists and %d albums for '%s'.\n", artist_count, album_count, keyword);

                // Free data memory
                slist_destroy(search_res, 0);
                free(keyword);
                free(arr);       
            }
            else if (strncmp(tokens[1], "albums", 256) == 0){
                print_albums(albums, num, album_sort);
                printf("Found %d albums and printed %d.\n", htable_num_elems(albums), num);
            }

            else if (strncmp(tokens[1], "album", 256) == 0){
                if (!tokens[2]){
                    printf("Error: track command requires a parameter.\n");
                    continue;
                }                
                // Find relevant album ids and put into list and then call print_album_list
                // Create keyword
                char *keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);
                strcaps(keyword);

                // Make array from htable
                void **arr = htable_values(albums);

                // Sort arr
                qsort(arr, htable_num_elems(albums), sizeof(struct album *), album_sort);

                // Create list and add album ids of albums that contain keyword
                struct slist *search_res = slist_create();
                for (uint32_t i = 0; i < htable_num_elems(albums); i++) {
                    struct album *candidate = (struct album *)arr[i];
                    char *candidate_keyword = strdup(candidate->name);
                    strcaps(candidate_keyword);

                    // Check if keyword exists in candidate_keyword
                    if (strstr(candidate_keyword, keyword) != NULL) {
                        slist_add_back(search_res, candidate->album_id);
                    }

                    free(candidate_keyword);
                }

                album_count = print_album_list(search_res, albums, track_by_album, tracks, &track_count);

                // For printing purposes
                free(keyword);
                keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);

                printf("Found %d albums and %d tracks for '%s'.\n", album_count, track_count, keyword);

                // Free data memory
                slist_destroy(search_res, 0);
                free(keyword);
                free(arr);                
            }
            else if (strncmp(tokens[1], "playlists", 256) == 0){
                print_playlists(playlists, num, playlist_sort);
                printf("Found %d playlists and printed %d.\n", htable_num_elems(playlists), num);
            }
            else if (strncmp(tokens[1], "playlist", 256) == 0){
                if (!tokens[2]){
                    printf("Error: track command requires a parameter.\n");
                    continue;
                }     
            
                // Find relevant playlist ids and put into list and then call print_playlist_list
                // Create keyword
                char *keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);
                strcaps(keyword);

                // Make array from htable
                void **arr = htable_values(playlists);

                // Sort arr
                qsort(arr, htable_num_elems(playlists), sizeof(struct playlist *), playlist_sort);

                // Create list and add playlist ids of playlists that contain keyword
                struct slist *search_res = slist_create();
                for (uint32_t i = 0; i < htable_num_elems(playlists); i++) {
                    struct playlist *candidate = (struct playlist *)arr[i];
                    char *candidate_keyword = strdup(candidate->name);
                    strcaps(candidate_keyword);

                    // Check if keyword exists in candidate_keyword
                    if (strstr(candidate_keyword, keyword) != NULL) {
                        slist_add_back(search_res, candidate->playlist_id);
                    }

                    free(candidate_keyword);
                }

                playlist_count = print_playlist_list(search_res, playlists, track_by_playlist, tracks, &track_count);

                // For printing purposes
                free(keyword);
                keyword = strdup(tokens[2]);
                remove_first_and_last_char(keyword);

                printf("Found %d playlists and %d tracks for '%s'.\n", playlist_count, track_count, keyword);

                // Free data memory
                slist_destroy(search_res, 0);
                free(keyword);
                free(arr);                 
            }
            else {
                printf("Invalid list command: '%s'\n", tokens[1]);
            }
        } 
        else
            printf("Invalid command: %s\n", cmd);
    }

    htable_destroy_slist_values(track_by_album);
    htable_destroy_slist_values(album_by_track);
    htable_destroy_slist_values(album_by_artist);
    htable_destroy_slist_values(track_by_playlist);
}

