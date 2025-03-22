#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Client Server Communication Protocol4
 * 
 */

 // Command IDs (enum)
enum command_id {
	TEST,
    SHOW_TRACKS,
    SHOW_ALBUMS,
    SHOW_PLAYLISTS,
    SEARCH_TRACKS,
    SEARCH_ALBUMS,
    SEARCH_ARTISTS,
    SEARCH_PLAYLISTS,
    QUIT
};

// Status Codes (enum)
enum status_code {
    OK,
    ERROR
};

enum error_type {
	CHECK_SUM_ERR,
	INVALID_CMD_ERR,
	ZERO_ARGS_ERR,
	NO_RESULTS_ERR,
	UNKNOWN_ERR
};

// Response Types (enum)
enum response_id {
    TRACK,
    ALBUM,
    PLAYLIST,
    ARTIST,
    NONE
};
 
struct request_msg
{
	enum command_id command;			// command (see )			
	char args[256];		// arguments (max 4 up to 256 chars each)
	unsigned int check;				// check sum
};
struct response_header{
	enum status_code status; // Status code (enum: OK, ERROR)	
	int num_tracks; // Number of tracks in the data block
	int num_albums; // Number of albums in the data block
	int num_playlists; // Number of playlists in the data block
	unsigned int check; // Check sum
};
struct response_data {
	struct track *tracks; // Array of Track structures (dynamically allocated)
	struct album *albums; // Array of Album structures (dynamically allocated)
	struct playlist *playlists; // Array of Playlist structures (dynamically allocated)
};
struct response_msg {
	struct response_header header; // Response header block
	// Union of response data and error message (they share the same memory)
	// since only one of them will be used at a time
	union {
		struct response_data data; // Response data block
		char error_message[256]; // Error message string
    };	
};

/**
 * @brief Defines structures for the spotify play data.
 * 
 */
struct play
{
	char track_id[32];
	char album_id[32];
	char playlist_id[32];
};

struct track
{
	char track_id[32];
	char name[256];
	char artist[256];
	int popularity;
	float danceability;
	float energy;
	float key;
	float loudness;
	float speechiness;
	float acousticness;
	float instrumentalness;
	float liveness;
	float valence;	
	float tempo;
	int duration_ms; 
};

struct album
{
	char album_id[32];
	char name[256];
	time_t release_date;
};

struct playlist
{
	char playlist_id[32];
	char name[256];
	char genre[64];
	char subgenre[64];
};



// print a single track for consistency
void print_track(struct track *t);
void print_album(struct album *a);
void print_playlist(struct playlist *pl);


/**
 * @brief parse a CSV line into the play, track, album, and playlist structures	
 * 
 * @param line 
 * @param p 
 * @param t 
 * @param a 
 * @param pl 
 * @return int one on success, zero on failure
 */
int 
parse_line (char *line, 
	struct play *p, 
	struct track *t, 
	struct album *a, 
	struct playlist *pl);

char* strtok_quotes(char *str, const char *delim);

char* clean_str(char *token);

#endif