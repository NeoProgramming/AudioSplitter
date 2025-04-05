#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "id3v1.h"

// based on:
/*-----------------------------------------------
 * Mp3 ID3v1 Tag Editor (Version 1.0)
 * Programmed by Mike Rosenberg
 * Completed 4/15/2013
 * http://mikerosenberger.com
 * https://github.com/miker525
 *---------------------------------------------*/

/* https://en.wikipedia.org/wiki/ID3
Field	Length	Description
header	3	"TAG"
title	30	30 characters of the title
artist	30	30 characters of the artist name
album	30	30 characters of the album name
year	4	A four-digit year
comment	28 or 30	The comment.
zero	1	If a track number is stored, this byte contains a binary 0.
track	1	The number of the track on the album, or 0. Invalid, if previous byte is not a binary 0.
genre	1	Index in a list of genres, or 255

*/


int getfsize(FILE *file)
{
	//returns the size of the file as an int
	int loc = ftell(file);
	fseek(file,0,SEEK_END);
	int size = ftell(file);
	fseek(file, loc, SEEK_SET);
	return size;
}

void fill(ID3V1 *id, const char *fname)
{
	const char *dot = strchr(fname, '.');
	int len = dot ? dot-fname : strlen(fname);

	memset(id->title, 0, sizeof(id->title));
	memset(id->artist, 0, sizeof(id->artist));
	memset(id->album, 0, sizeof(id->album));

	memcpy(id->header, "TAG", 3);
	
	memcpy(id->title, fname, len);
	memcpy(id->artist, fname, len);
	memcpy(id->album, fname, len);
}

bool gen(const char *fpath)
{
	const char *fname = std::max( strrchr(fpath, '\\'), strrchr(fpath, '/') );
	if(!fname)
		return false;
	fname++;
	
	FILE *f = fopen(fpath, "rb+"); // Opens for both reading and writing. (The file must exist.)
	if(!f) 
		return false;

	
	int fsize = getfsize(f);
	fseek(f, fsize-sizeof(ID3V1), SEEK_SET);

	ID3V1 id;
	fread(&id, sizeof(id), 1, f);
	if(!memcmp(id.header, "TAG", 3)) {
		// tag found
		fill(&id, fname);
		fseek(f, fsize-sizeof(ID3V1), SEEK_SET);
		fwrite(&id, sizeof(id), 1, f);
	}
	else {
		// tag not found
		fclose(f);
		if(!fopen(fpath, "ab+"))
			return false;
		memset(&id, 0, sizeof(id));
		fill(&id, fname);
		fseek(f, fsize, SEEK_SET);
		fwrite(&id, sizeof(id), 1, f);
	}
	fclose(f);
	return true;
}

