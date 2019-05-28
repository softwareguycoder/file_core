/*
 * file_core.h
 *
 *  Created on: Jul 26, 2018
 *      Author: bhart
 */

#ifndef __FILE_CORE_H__
#define __FILE_CORE_H__

/**
 * @brief Defines the maximum length for a path name.
 */
#ifndef MAX_PATH
#define MAX_PATH      4096
#endif //MAX_PATH

/**
 * @name CreateDirectory
 * @brief Creates all the directories in a specified path.
 * @param pszPath The directory to create.
 * @remarks Attempts to create the directory(ies) indicated by the specified
 * path.  The caller of this function must have the appropriate access
 * privileges granted by the operating system.  Fails if this is not the
 * case, or if the directory indicated already exists, or if the path is blank.
 * Upon failure, this function will simply give up and return to the caller,
 * except if the mkdir() system call fails; in which case, the function
 * raises an error message to STDERR and exits the application.
 */
void CreateDirectory(const char* pszPath);
void CreateDirIfNotExists(const char* pszPathName);
BOOL DirectoryExists(const char* pszPath);
BOOL FileExists(const char* pszPath);
void ReadAllText(const char* pszPath, char** ppszOutput,
    int *pnFileSize);
void write_all_text(const char* path, const char* content);
void save_text_to_file(const char* path, const char* content_format, ...);
void do_prompt_file_name(const char* prompt, char* path, int path_size);


/**
 * @brief Expands strings a la Bash. Wraps the wordexp function.
 * @param pszPathName Address of a buffer containing the content to be
 * expanded.
 * @param pszBuffer Address of a buffer into which the result should
 * be stored.  The buffer must have sufficient space for the result.
 * @param nBufferSize Length of the output buffer, in bytes.  Must be long
 * enough to hold the expanded filename.
 * @remarks It is sufficient to allocate a buffer of size 255 chars (not
 * including the null-terminator).  This function turns '~/dir' into
 * '/home/user/dir', for example.
 */
void ShellExpand(const char* pszPathName,
    char* pszBuffer, int nBufferSize);

#endif /* __FILE_CORE_H__ */
