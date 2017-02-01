/*
 * string.h
 *
 *  Created on: Mar 30, 2013
 *      Author: duoln
 */

#ifndef SIMPLE_STRING_TOOL_H_
#define SIMPLE_STRING_TOOL_H_

#include "collection/array_list.h"

#include <stdbool.h>

#define STRING_TOOL_STR_LEN 2048

char        simple_string_at(const char* self, int index);

int         simple_string_index_of(const char* self, const char* str);

int         simple_string_last_index_of(const char* self, const char* str);

bool        simple_string_start_with(const char* self, const char* target);

bool        simple_string_end_with(const char* self, const char* target);

int         simple_string_substr(const char* self, int begin_index, int end_index, /*out*/char new_str[]);

char*       simple_string_dup(const char* self);

int         simple_string_cmp_ignore_case(const char* str1, const char* str2);

char*       simple_string_to_upper(char* self);

char*       simple_string_to_lower(char* self);

void        simple_string_trim(const char* self, char c, char buf[]);

void        simple_string_left_trim(const char* self, char c, char buf[]);

void        simple_string_right_trim(const char* self, char c, char buf[]);

ArrayList*  simple_string_split(const char* self, const char* regex);

#endif /* STRING_H_ */
