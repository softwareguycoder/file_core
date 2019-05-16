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

#endif /* __FILE_CORE_H__ */
