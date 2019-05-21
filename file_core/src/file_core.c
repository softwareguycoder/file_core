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
		LogError("The pathname specified is blank, we cannot do anything with this.");

		return 0; /* failed to locate file with NULL path */
	}

	if (path[0] == '\0') {
		LogError("The pathname specified is blank, we cannot do anything with this.");

		return 0;	/* failed to locate file with blank path */
	}

	LogDebug("Determining if file '%s' is accessible...", path);

	if (access(path, F_OK) == 0){
		LogDebug("The file '%s' can be found and we have the correct permissions to access it.", path);
		return 1;	/* we can access the file */
	}

	LogDebug("The file '%s' cannot be located, or we do not have full permissions on the file.", path);

	return 0;	/* we either (a) do not have enough permissions to access the file, or (b) it does not exist */
}

/* Reads all text from the file with path 'path'.  The file must exist and we must have full permissions on it.
 * This function returns a NULL pointer value in the case that the data cannot be read from the file.
 */
char* read_all_text(const char* path) {
	if (path == NULL || path[0] == '\0' || !is_file_accessible(path)){
		LogError("No pathname specified in call to read_all_text, or the file referenced is not accessible.");
		return NULL;	/* Failed to access the file for reading  or it does not exist. */
	}

	LogDebug("Attempting to open the file '%s' for reading...", path);

	char *file_content = NULL;
	int file_size = 0;
	FILE *fp;

	fp = fopen(path, "r");
	if(fp) {
		LogDebug("The file '%s' has been opened.  Attemtping to read the data from it...", path);

		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		rewind(fp);

		file_content = (char*) malloc((sizeof(char) * file_size) + 1);

		long bytes_read = (long)fread(file_content, sizeof(char), file_size, fp);

		LogDebug("Read %lu B from file '%s'.", bytes_read, path);

		fclose(fp);

		LogDebug("File '%s' has been closed.", path);
	} else {
		LogError("Failed to open file '%s' for reading.", path);
	}
	return file_content;
}

void write_all_text(const char* path, const char* content) {
	if (path == NULL || path[0] == '\0') {
		LogError("No pathname specified in call to write_all_text.");
		exit(-1);		/* Cannot write data to a file when no pathname is specified. */
	}

	if (content == NULL || strlen(content) == 0){
		LogError("No content specified for writing to the file '%s'.", path);
		exit(-1);		/* Content must be provided for writing. */
	}

	LogDebug("Attempting to open the file '%s' for writing...", path);

	FILE* fp = fopen(path, "w+");	/* Open the file at path for writing, create if it does not exist. */
	if (fp) {
		LogDebug("Successfully opened '%s' for writing.", path);


		long bytes_written = (long)fwrite(content, sizeof(char), strlen(content), fp);
		LogDebug("Wrote %lu B to the file '%s'.", bytes_written, path);

		fclose(fp);

		LogDebug("File '%s' has been closed.", path);

	} else {
		LogError("ERROR: Failed to open or create the file '%s' for writing.\n", path);
		exit(-1);
	}

	/* Done */
}

/** Shortcut for writing formatted text to a file */
void save_text_to_file(const char* path, const char* content_format, ...) {
	va_list args;
	va_start(args, content_format);

	if (content_format == NULL || content_format[0] == '\0') {
		return;
	}

	char buf[strlen(content_format)+ 1];

	vsprintf(buf, content_format, args);

	write_all_text(path, buf);
}

/** Prompts the user for the name to use for either saving or opening a file.
 *
 * prompt: The prompt to display to the user for opening the file.  Must not be NULL.
 * path: Address of a buffer that will receive the path the user typed.  This buffer must have greater than zero size.
 * path_size: The size, in bytes, of the path buffer.  Should include the NULL terminator.
 *
 * Remarks:
 * 		This function will display a prompt for the filename that a user should use for opening a file for reading,
 * 		or for writing data to an existing file.  The prompt should make it clear to the user that blank input cancels
 * 		the operation (i.e., if the user gets the prompt but does not want to open a file or save data to a file, they
 * 		should be told that they can just press the ENTER key to cancel).
 *
 */
void do_prompt_file_name(const char* prompt, char* path, int path_size) {
	if (path == NULL){
		LogError("Null path variable in do_prompt_file_name.  Required parameter.");
		return;
	}

	if (prompt == NULL || strlen(prompt) == 0){
		LogError("Null prompt variable in do_prompt_file_name.  Required parameter.");
		return;
	}

	if (path_size <= 0){
		LogError("Must supply integer that is a positive number for the path_size.");
		return;
	}

	/* Call the get_line function from the console_core library in order to display the prompt and get
	 * the user's input.  If the user does not specify anything it is probably because the user wants to cancel.
	 */
	int retcode = GetLineFromUser(prompt, path, path_size);

	if (retcode != OK && retcode != EXACTLY_CORRECT) {
		/* No error should be shown here, since blank means the user wants to cancel the operation. */
		return;
	}

	/* If we are here, the buffer pointed to by the path variable should now contain the path to the file
	 * that the user wants to work with.
	 */
}

///////////////////////////////////////////////////////////////////////////////
// CreateDirIfNotExists function

void ShellExpand(const char* pszPathName,
    char* pszBuffer, int nBufferSize) {
  if (IsNullOrWhiteSpace(pszPathName)) {
    return;
  }

  if (pszBuffer == NULL) {
    return;
  }

  if (nBufferSize <= 0) {
    return;
  }

  memset(pszBuffer, 0, nBufferSize);

  wordexp_t p;
  wordexp(pszPathName, &p, 0);
  char **w = p.we_wordv;
  if (p.we_wordc <= 0 || IsNullOrWhiteSpace(w[0])) {
    return;
  }

  strncpy(pszBuffer, w[0], nBufferSize);

  wordfree(&p);
}

void CreateDirIfNotExists(const char* pszPathName) {
	if (IsNullOrWhiteSpace(pszPathName)) {
		return;
	}

	char szExpandedPathName[MAX_PATH + 1];
	ShellExpand(pszPathName, szExpandedPathName, MAX_PATH + 1);

	struct stat st = { 0 };
	if (-1 == stat(szExpandedPathName, &st)) {
		mkdir(szExpandedPathName, 0777);
	}
}
