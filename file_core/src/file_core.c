/*
 * file_core.c
 *
 *  Created on: Jul 26, 2018
 *      Author: bhart
 */

// Contains routines to work with files

#include "stdafx.h"
#include "file_core.h"

// Checks whether the file at the specified path exists.  Returns nonzero if the file
// exists; zero if it does not.
int is_file_accessible(const char* path) {
	if (path == NULL){
		log_error("The pathname specified is blank, we cannot do anything with this.");

		return 0; /* failed to locate file with NULL path */
	}

	if (path[0] == '\0') {
		log_error("The pathname specified is blank, we cannot do anything with this.");

		return 0;	/* failed to locate file with blank path */
	}

	log_debug("Determining if file '%s' is accessible...", path);

	if (access(path, F_OK) == 0){
		log_debug("The file '%s' can be found and we have the correct permissions to access it.", path);
		return 1;	/* we can access the file */
	}

	log_debug("The file '%s' cannot be located, or we do not have full permissions on the file.", path);

	return 0;	/* we either (a) do not have enough permissions to access the file, or (b) it does not exist */
}

/* Reads all text from the file with path 'path'.  The file must exist and we must have full permissions on it.
 * This function returns a NULL pointer value in the case that the data cannot be read from the file.
 */
char* read_all_text(const char* path) {
	if (path == NULL || path[0] == '\0' || !is_file_accessible(path)){
		log_error("No pathname specified in call to read_all_text, or the file referenced is not accessible.");
		return NULL;	/* Failed to access the file for reading  or it does not exist. */
	}

	log_debug("Attempting to open the file '%s' for reading...", path);

	char *file_content = NULL;
	int file_size = 0;
	FILE *fp;

	fp = fopen(path, "r");
	if(fp) {
		log_debug("The file '%s' has been opened.  Attemtping to read the data from it...", path);

		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		rewind(fp);

		file_content = (char*) malloc((sizeof(char) * file_size) + 1);

		long bytes_read = (long)fread(file_content, sizeof(char), file_size, fp);

		log_debug("Read %lu B from file '%s'.", bytes_read, path);

		fclose(fp);

		log_debug("File '%s' has been closed.", path);
	} else {
		log_error("Failed to open file '%s' for reading.", path);
	}
	return file_content;
}

void write_all_text(const char* path, const char* content) {
	if (path == NULL || path[0] == '\0') {
		log_error("No pathname specified in call to write_all_text.");
		exit(-1);		/* Cannot write data to a file when no pathname is specified. */
	}

	if (content == NULL || strlen(content) == 0){
		log_error("No content specified for writing to the file '%s'.", path);
		exit(-1);		/* Content must be provided for writing. */
	}

	log_debug("Attempting to open the file '%s' for writing...", path);

	FILE* fp = fopen(path, "w+");	/* Open the file at path for writing, create if it does not exist. */
	if (fp) {
		log_debug("Successfully opened '%s' for writing.", path);


		long bytes_written = (long)fwrite(content, sizeof(char), strlen(content), fp);
		log_debug("Wrote %lu B to the file '%s'.", bytes_written, path);

		fclose(fp);

		log_debug("File '%s' has been closed.", path);

	} else {
		log_error("ERROR: Failed to open or create the file '%s' for writing.\n", path);
		exit(-1);
	}

	/* Done */
}
