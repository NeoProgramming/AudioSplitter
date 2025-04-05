#pragma once

#pragma pack(push, 1)
struct ID3V1 {
	char header[3];		// "TAG"
	char title[30];		// 30 characters of the title
	char artist[30];	// 30 characters of the artist name
	char album[30];		// 30 characters of the album name
	char year[4];		// A four-digit year
	char comment[28];	// The comment.
	char zero[1];		// If a track number is stored, this byte contains a binary 0.
	char track[1];		// The number of the track on the album, or 0. Invalid, if previous byte is not a binary 0.
	char genre[1];		// Index in a list of genres, or 255
};
#pragma pack(pop)

bool gen(const char *fname);



