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

#endif /* __FILE_CORE_H__ */
