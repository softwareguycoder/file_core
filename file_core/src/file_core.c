/*
 * file_core.c
 *
 *  Created on: Jul 26, 2018
 *      Author: bhart
 */

// Contains routines to work with files
#include "stdafx.h"
#include "file_core.h"

#include "file_core_symbols.h"

///////////////////////////////////////////////////////////////////////////////
// Publicly-exposed functions

///////////////////////////////////////////////////////////////////////////////
// CloseFile function

void CloseFile(FILE** fppFile) {
  if (fppFile == NULL) {
    return; // Required parameter
  }

  if (*fppFile == NULL) {
    return; // Required parameter - or file is already closed
  }

  fclose(*fppFile);
  *fppFile = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// CreateDirectory function

void CreateDirectory(const char* pszPath) {
  if (IsNullOrWhiteSpace(pszPath)) {
    return;
  }

  /* Be sure to expand the path name string just like Bash would */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszPath, szExpandedPathName, MAX_PATH + 1);

  /* Be sure the directory does not already exist */
  if (DirectoryExists(szExpandedPathName)) {
    return;
  }

  /* Raise an exception if mkdir call fails */
  if (OK != mkdir(szExpandedPathName, 0777)) {
    perror("CreateDirectory");
    exit(EXIT_FAILURE);
  }
}

///////////////////////////////////////////////////////////////////////////////
// CreateDirIfNotExists function

void CreateDirIfNotExists(const char* pszPath) {
  /* If the path name provided is blank, then
   * there is nothing to do. */
  if (IsNullOrWhiteSpace(pszPath)) {
    return;
  }

  /* Be sure to expand the path name string just like Bash would */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszPath, szExpandedPathName, MAX_PATH + 1);

  /* Check whether the specified directory exists.  If it does not,
   * then try to create it. */
  if (!DirectoryExists(szExpandedPathName)) {
    CreateDirectory(szExpandedPathName);
  }
}

///////////////////////////////////////////////////////////////////////////////
// DirectoryExists function

BOOL DirectoryExists(const char* pszPath) {
  /* If the path is blank, then we have nothing to do. */
  if (IsNullOrWhiteSpace(pszPath)) {
    return FALSE;
  }

  /* Be sure to expand the path name string just like Bash would */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszPath, szExpandedPathName, MAX_PATH + 1);

  /* Check for the existence of the specified directory using
   * the stat syscall. */
  struct stat st = { 0 };
  return stat(szExpandedPathName, &st) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// FileExists function

BOOL FileExists(const char* pszPath) {
  /* If the path is blank, then we have nothing to do. */
  if (IsNullOrWhiteSpace(pszPath)) {
    return FALSE;
  }

  /* Be sure to expand the path name string just like Bash would */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszPath, szExpandedPathName, MAX_PATH + 1);

  /* Use the access syscall to determine whether the file exists. */
  return access(szExpandedPathName, F_OK) == OK;
}

///////////////////////////////////////////////////////////////////////////////
// GetCurrentWorkingDirectory function

void GetCurrentWorkingDirectory(char* pszCurrentWorkingDir, int nBufferSize) {
  memset(pszCurrentWorkingDir, 0, nBufferSize);

  char szResult[nBufferSize];
  memset(szResult, 0, nBufferSize);

  if (getcwd(szResult, nBufferSize) == NULL) {
    fprintf(stderr, "ERROR: Could not access current working directory.\n");
    exit(EXIT_FAILURE);
  }

  strncpy(pszCurrentWorkingDir, szResult, nBufferSize);
}

void GetHomeDirectoryPath(char** ppszHomePath) {
  if (ppszHomePath == NULL) {
    return;
  }

  *ppszHomePath = getenv(HOME_ENVIRONMENT_VARIABLE_NAME);
  if (!IsNullOrWhiteSpace(*ppszHomePath)) {
    return;
  }

  struct passwd pwent = { 0 };
  struct passwd *pwentp;
  char buf[1024];

  if (OK != getpwuid_r(getuid(), &pwent, buf, sizeof(buf), &pwentp)) {
    fprintf(stderr, "ERROR: Failed to get user home directory path.\n");
    exit(EXIT_FAILURE);
  }

  *ppszHomePath = pwentp->pw_dir;
}

///////////////////////////////////////////////////////////////////////////////
// ReadAllText function

void ReadAllText(const char* pszPath, char** ppszOutput,
    int *pnFileSize) {

  const int CHUNK_SIZE = 1024; /* chunk size, in bytes */

  char szBuffer[CHUNK_SIZE + 1];
  memset(szBuffer, 0, CHUNK_SIZE + 1);

  char szExpandedFileName[MAX_PATH + 1];
  memset(szExpandedFileName, 0, MAX_PATH + 1);

  int nBytesRead = 0;
  int nTotalBytesRead = 0;

  /* Nothing to do if the pathname is blank. */
  if (IsNullOrWhiteSpace(pszPath)) {
    return;
  }

  /* Expand the file name string a la Bash */
  ShellExpand(pszPath, szExpandedFileName, MAX_PATH + 1);

  if (!FileExists(szExpandedFileName)) {
    ThrowFileNotFoundException("ReadAllText", pszPath, NULL);
  }

  if (ppszOutput == NULL) {
    fprintf(stderr, "ReadAllText: Missing required parameter 'output'.\n");
    exit(EXIT_FAILURE);
    return;
  }

  FILE* fp = fopen(szExpandedFileName, "r");
  if (fp == NULL) {
    fprintf(stderr, "ERROR: Failed to open %s for reading.\n",
        szExpandedFileName);

    CloseFile(&fp);
    exit(EXIT_FAILURE);
    return;
  }

  *ppszOutput = (char*) malloc((CHUNK_SIZE + 1) * sizeof(char));
  if (ppszOutput == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate memory.\n");
    CloseFile(&fp);
    exit(EXIT_FAILURE);
    return;
  }
  memset(*ppszOutput, 0, (CHUNK_SIZE + 1) * sizeof(char));

  while ((nBytesRead = fread(szBuffer, sizeof(char), CHUNK_SIZE, fp)) != 0) {
    nTotalBytesRead += nBytesRead;
    strcat(*ppszOutput, szBuffer);

    *ppszOutput = (char*) realloc(*ppszOutput,
        sizeof(char) * (nTotalBytesRead + CHUNK_SIZE + 1));

    memset(szBuffer, 0, CHUNK_SIZE + 1);
  }

  CloseFile(&fp);

  *ppszOutput = (char*) realloc(*ppszOutput,
      (nTotalBytesRead + 1) * sizeof(char));
  (*ppszOutput)[nTotalBytesRead] = '\0';

  *pnFileSize = nTotalBytesRead;

  return;
}

///////////////////////////////////////////////////////////////////////////////
// SetCurrentWorkingDirectory function

BOOL SetCurrentWorkingDirectory(const char* pszDirectoryPath) {
  if (IsNullOrWhiteSpace(pszDirectoryPath)) {
    return FALSE;
  }

  /* Be sure to expand the path name string just like Bash would */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszDirectoryPath, szExpandedPathName, MAX_PATH + 1);

  if (!DirectoryExists(szExpandedPathName)) {
    return FALSE;
  }

  if (OK != chdir(szExpandedPathName)) {
    fprintf(stderr, "ERROR: Failed to set the current directory to '%s'.\n",
        szExpandedPathName);
    return FALSE;
  }

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// WriteAllText function

void WriteAllText(const char* pszPath, const char* pszContent,
    BOOL bOverwrite, int* pnBytesWritten) {
  /* Can't proceed if the pathname is blank */
  if (IsNullOrWhiteSpace(pszPath)) {
    return;
  }

  /* If there is nowhere to store the number of bytes written,
   * can't proceed. */
  if (pnBytesWritten == NULL) {
    return;
  }

  /* Expand the path name a la Bash */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszPath, szExpandedPathName, MAX_PATH + 1);

  /* If the file already exists, and the overwrite flag is set, then
   * delete the file from the file system first.  */
  if (FileExists(szExpandedPathName) && bOverwrite) {
    remove(szExpandedPathName);
  }

  /* If the file exists, but content is blank (regardless of the state of
   * the overwrite flag), then delete the file using the remove syscall. */
  if (FileExists(szExpandedPathName)
      && IsNullOrWhiteSpace(pszContent)) {
    remove(szExpandedPathName);
    return;
  }

  const char* FILE_MODE = bOverwrite ? "w+" : "a+";
  FILE* fp = fopen(szExpandedPathName, FILE_MODE);
  if (fp == NULL) {
    CloseFile(&fp);
    ThrowFileAccessFailedException("WriteAllText",
        szExpandedPathName, NULL);
  }

  *pnBytesWritten = fprintf(fp, "%s", pszContent);
  if (*pnBytesWritten < 0) {
    CloseFile(&fp);
    ThrowFileAccessFailedException("WriteAllText",
        szExpandedPathName, NULL);
  }

  CloseFile(&fp);
}

///////////////////////////////////////////////////////////////////////////////
// WriteFormattedTextToFile function

void WriteFormattedTextToFile(BOOL bOverwrite, int* pnBytesWritten,
    const char* pszPath, const char* pszContentFormat, ...) {
  /* Can't proceed if the pathname is blank */
  if (IsNullOrWhiteSpace(pszPath)) {
    return;
  }

  /* If there is nowhere to store the number of bytes written,
   * can't proceed. */
  if (pnBytesWritten == NULL) {
    return;
  }

  /* Expand the path name a la Bash */
  char szExpandedPathName[MAX_PATH + 1];
  memset(szExpandedPathName, 0, MAX_PATH + 1);
  ShellExpand(pszPath, szExpandedPathName, MAX_PATH + 1);

  /* If the file already exists, and the overwrite flag is set, then
   * delete the file from the file system first.  */
  if (FileExists(szExpandedPathName) && bOverwrite) {
    remove(szExpandedPathName);
  }

  /* If the file exists, but content is blank, and overwrite flag is set,
   * then delete the file using the remove syscall */
  if (FileExists(szExpandedPathName)
      && IsNullOrWhiteSpace(pszContentFormat)
      && bOverwrite) {
    remove(szExpandedPathName);
    return;
  }

  va_list args;
  va_start(args, pszContentFormat);

  const int CONTENT_SIZE = strlen(pszContentFormat) + 1;
  char szBuffer[CONTENT_SIZE];
  memset(szBuffer, 0, CONTENT_SIZE);

  vsprintf(szBuffer, pszContentFormat, args);

  WriteAllText(pszPath, szBuffer, bOverwrite, pnBytesWritten);
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
  if (path == NULL) {
    LogError(
        "Null path variable in do_prompt_file_name.  Required parameter.");
    return;
  }

  if (prompt == NULL || strlen(prompt) == 0) {
    LogError(
        "Null prompt variable in do_prompt_file_name.  Required parameter.");
    return;
  }

  if (path_size <= 0) {
    LogError(
        "Must supply integer that is a positive number for the path_size.");
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
// ShellExpand function

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
  if (p.we_wordv == NULL) {
    return;
  }
  char **w = p.we_wordv;
  if (p.we_wordc <= 0 || IsNullOrWhiteSpace(w[0])) {
    return;
  }

  strncpy(pszBuffer, w[0], nBufferSize);

  wordfree(&p);
}

