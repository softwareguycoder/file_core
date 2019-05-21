/*
 * file_core.h
 *
 *  Created on: Jul 26, 2018
 *      Author: bhart
 */

#ifndef __FILE_CORE_H__
#define __FILE_CORE_H__

int is_file_accessible(const char* path);
char* read_all_text(const char* path);
void write_all_text(const char* path, const char* content);
void save_text_to_file(const char* path, const char* content_format, ...);
void do_prompt_file_name(const char* prompt, char* path, int path_size);
void CreateDirIfNotExists(const char* pszPathName);

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
