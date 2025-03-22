#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "spotify.h"


// print a single track for consistency
void print_track(struct track *t) {
    printf("[TRACK] %s, %26.26s, %26.26s,", t->track_id, t->name, t->artist);
    printf(" %3d, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %8d\n",
        t->popularity,
        t->danceability,
        t->energy,
        t->key,
        t->loudness,
        t->speechiness,
        t->acousticness,
        t->instrumentalness,
        t->liveness,
        t->valence,
        t->tempo,
        t->duration_ms);
}
void print_album(struct album *a) {
    struct tm *release_date = localtime(&a->release_date);
    char release_date_str[64];
    strftime(release_date_str, sizeof(release_date_str), "%Y-%m-%d", release_date);
    printf("[ALBUM] %s, %26.26s, %s\n", a->album_id, a->name, release_date_str);
}
void print_playlist(struct playlist *pl) {
    printf("[PLAYLIST] %s, %26.26s, %26.26s, %26.26s\n", pl->playlist_id, pl->name, pl->genre, pl->subgenre);
}

// from Gemini
// Convert a string date in the format "YYYY-MM-DD" to a Linux time	
time_t string_to_linux_time(const char* date_string) {
  struct tm parsed_time;
  memset(&parsed_time, 0, sizeof(parsed_time)); // Initialize struct to 0

  // Parse the date string (assuming format "YYYY-MM-DD")
  if (sscanf(date_string, "%4d-%2d-%2d",
             &parsed_time.tm_year, &parsed_time.tm_mon, &parsed_time.tm_mday) == EOF) {
    fprintf(stderr, "Invalid date string format: %s\n", date_string);
    return -1; // Indicate error
  }

  // Adjust month and year (tm_year starts from 1900, tm_mon from 0)
  parsed_time.tm_year -= 1900;
  parsed_time.tm_mon -= 1;

  // Convert to Linux time
  return mktime(&parsed_time);
}

/**
 * Clean up a string by removing non-printable characters, leading and trailing spaces, and quotes in place.
 */
char* clean_str(char *token) {
	uint32_t len = strlen(token);
	if (len == 0) return token;
	// remove non printable characters
	char *copy = strdup(token);
	uint32_t j = 0;
	for (uint32_t i = 0; i < len; i++) {
		if (isprint(copy[i])) {
			token[j++] = copy[i];
		} else {
			token[j++] = '-';
		}		
	}
	token[j] = '\0';
	free(copy);

	len = j;
	
	// strip leading spaces
	while (isspace(*token)) {
		token ++;
		len -= 1;
	}
	// strip trailing spaces
	while (isspace(token[len-1])) {
		token[len-1] = '\0';
		len -= 1;
	}
	// remove the quotes
	while ((len>1) && (token[0] == '"') && (token[len-1] == '"')) {
		// remove the quotes
		token ++;
		token[len-2] = '\0';
		len -= 2;
	}
	return token;
}

// A version of strtok that handles quoted strings
char* strtok_quotes(char *str, const char *delim) {
	char *token = strtok(str, delim);

	if (token == NULL) {
		return NULL;
	}

	// if the token starts with a quote, then it is a string	
	if (token[0] == '"') {
		
		while (token[strlen(token) - 1] != '"') {
			// get the next token to check if it's closed
			char *next = strtok(NULL, delim);

			// end of string searching for the closing quote
			if (next == NULL) {
				fprintf(stderr, "Error: missing closing quote\n");
				return token;
			}

			// join the tokens
			token[strlen(token)] = *delim; // restore the comma
		}
	}		
	return clean_str(token);
}

int 
parse_line(char *line, 
	struct play *p, 
	struct track *t, 
	struct album *a, 
	struct playlist *pl)
{
	char *token = clean_str(strtok(line, ","));
	if (token == NULL) {
		return 0;
	}
	// store the track_id in the play and track structures (yes two copies!)
	strncpy(p->track_id, token, 32);
	strncpy(t->track_id, token, 32);

	// get the next token (track_name), it might be quoted, so use strtok_quotes!
	token = clean_str(strtok_quotes(NULL, ","));
	if (token == NULL) {
		return 0;
	}

	// save the result
    strncpy(t->name, token, 256);

	// then continue calling strtok to get the rest of the tokens and
	// populate the structures with the data!
	// artist
   	token = clean_str(strtok_quotes(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(t->artist, token, 256);

	// popularity
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->popularity = atoi(token);

	// album id
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(p->album_id, token, 32);
	strncpy(a->album_id, token, 32);

	// album name
	token = clean_str(strtok_quotes(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(a->name, token, 256);

	// release date
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	a->release_date = string_to_linux_time(token);

	// playlist name
	token = clean_str(strtok_quotes(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(pl->name, token, 256);

	// playlist id
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(p->playlist_id, token, 32);
	strncpy(pl->playlist_id, token, 32);

	// playlist genre
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(pl->genre, token, 64);

	// playlist subgenre
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	strncpy(pl->subgenre, token, 64);

	// track stats
	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->danceability = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->energy = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->key = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->loudness = atof(token);

	// Skip mode?
	token = strtok(NULL, ",");

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->speechiness = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->acousticness = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->instrumentalness = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->liveness = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->valence = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->tempo = atof(token);

	token = clean_str(strtok(NULL, ","));
	if (token == NULL) {
		return 0;
	}
	t->duration_ms = atoi(token);
     

	// for fields that can be quoted, use strtok_quotes!

	// for fields that are numbers, use atof or atoi!
	return 1;
}