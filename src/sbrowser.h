#ifndef SBROWSER_H
#define SBROWSER_H

/**
 * @file sbrowser.h
 * @brief Header file for Spotify data browser functions.
 * @author Chang Min Bark (cb073@bucknell.edu)
 * @date 2025-01-29
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

#define MAX_LINE_LENGTH 1024 

/** 
 * Compares two string pointers for sorting (used in qsort).
 */
int compare_str_ptr(const void *a, const void *b);

/**
 * Compares two string values for key/value searching.
 */
int compare_str(const void *a, const void *b);

/**
 * Converts a string to uppercase (in place).
 */
void strcaps(char *str);

/**
 * Removes the first and last character from a string (for handling quotes).
 */
void remove_first_and_last_char(char *str);

/**
 * Destroys a hashtable where the values are singly linked lists.
 */
void htable_destroy_slist_values(struct htable *ht);

/**
 * Reads a CSV file and populates the given lists/hashtables.
 */
int read_csv_to_lists(FILE *file, struct slist *plays, struct htable *tracks, 
                      struct htable *albums, struct htable *playlists);

/**
 * Prints basic statistics about the dataset.
 */
void print_stats(struct slist *plays, struct htable *tracks, struct htable *albums, struct htable *playlists);

/**
 * Sort functions for tracks, albums, playlists, and artists.
 */
int track_sort(const void *a, const void *b);
int track_sort_flat(const void *a, const void *b);
int album_sort(const void *a, const void *b);
int album_sort_flat(const void *a, const void *b);
int playlist_sort(const void *a, const void *b);
int artist_sort(const void *a, const void *b);

/**
 * Print functions for various data structures.
 */
void print_artist(char *artist);

/**
 * Prints a specified number of tracks sorted by the provided sort function.
 */
void print_tracks(struct htable* tracks, uint32_t num, int(*sort)(const void*, const void*));

/**
 * Prints a specified number of albums sorted by the provided sort function.
 */
void print_albums(struct htable* albums, uint32_t num, int(*sort)(const void*, const void*));

/**
 * Prints a specified number of playlists sorted by the provided sort function.
 */
void print_playlists(struct htable* playlists, uint32_t num, int(*sort)(const void*, const void*));

/**
 * Prints a list of tracks associated with the given track IDs.
 */
int print_track_list(struct slist *track_ids, struct htable *tracks, struct htable *album_by_track, 
                     struct htable *albums, uint32_t *album_count);

/**
 * Prints a list of albums associated with the given album IDs.
 */
int print_album_list(struct slist *album_ids, struct htable *albums, struct htable *track_by_album, 
                     struct htable *tracks, uint32_t *track_count);

/**
 * Prints a list of playlists associated with the given playlist IDs.
 */
int print_playlist_list(struct slist *playlist_ids, struct htable *playlists, struct htable *track_by_playlist, 
                        struct htable *tracks, uint32_t *track_count);

/**
 * Prints a list of artists and their associated albums.
 */
int print_artist_list(struct slist *artist_names, struct htable *album_by_artist, 
                      struct htable *albums, uint32_t *album_count);

/**
 * Creates a hashtable mapping album IDs to track IDs.
 */
struct htable* create_track_by_album(struct slist *plays);

/**
 * Creates a hashtable mapping track IDs to album IDs.
 */
struct htable* create_album_by_track(struct slist *plays);

/**
 * Creates a hashtable mapping artist names to album IDs.
 */
struct htable* create_album_by_artist(struct slist *plays, struct htable *tracks);

/**
 * Creates a hashtable mapping playlist IDs to track IDs.
 */
struct htable* create_track_by_playlist(struct slist *plays);

/**
 * Runs the main menu loop for processing user commands.
 */
void main_menu(struct slist *plays, struct htable *tracks, struct htable *albums, struct htable *playlists);

#endif // SBROWSER_H
