#include "string_tool.h"
#include "assert.h"

#include "collection/array_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char simple_string_at(const char* self, int index)
{
    ASSERT(index < strlen(self), "The index out of range.");
    return self[index];
}

int simple_string_index_of(const char* src, const char* target)
{
    int src_size = strlen(src);
    int target_size = strlen(target);
    if (src_size < target_size)
    {
        return -1;
    }
    if (target_size == 0)
    {
        return 0;
    }
    int targetOffset = 0, sourceOffset = 0, fromIndex = 0, targetCount = target_size, sourceCount = src_size;
    const char* tmp = src;
    char first = target[targetOffset];
    int max = sourceOffset + (sourceCount - targetCount);
    int i = 0;
    for (i = sourceOffset + fromIndex; i <= max; i++)
    {
        /* Look for first character. */
        if (tmp[i] != first)
        {
            while (++i <= max && tmp[i] != first)
                ;
        }

        /* Found first character, now look at the rest of v2 */
        if (i <= max)
        {
            int j = i + 1;
            int end = j + targetCount - 1;
            int k = 0;
            for (k = targetOffset + 1; j < end && tmp[j] == target[k]; j++, k++)
                ;

            if (j == end)
            {
                /* Found whole string. */
                return i - sourceOffset;
            }
        }
    }
    return -1;
}

int simple_string_last_index_of(const char* src, const char* target)
{
    const char* source = src;

    int sourceCount = strlen(src);
    int targetCount = strlen(target);

    int fromIndex = sourceCount;
    int rightIndex = sourceCount - targetCount;

    int targetOffset = 0;
    int sourceOffset = 0;

    if (fromIndex < 0)
    {
        return -1;
    }
    if (fromIndex > rightIndex)
    {
        fromIndex = rightIndex;
    }
    /* Empty string always matches. */
    if (targetCount == 0)
    {
        return fromIndex;
    }

    int strLastIndex = targetOffset + targetCount - 1;
    char strLastChar = target[strLastIndex];
    int min = sourceOffset + targetCount - 1;
    int i = min + fromIndex;

    startSearchForLastChar: while (1)
    {
        while (i >= min && source[i] != strLastChar)
        {
            i--;
        }
        if (i < min)
        {
            return -1;
        }
        int j = i - 1;
        int start = j - (targetCount - 1);
        int k = strLastIndex - 1;

        while (j > start)
        {
            if (source[j--] != target[k--])
            {
                i--;
                goto startSearchForLastChar;
            }
        }
        return start - sourceOffset + 1;
    }
    return 0;
}

bool simple_string_start_with(const char* src, const char* target)
{
    size_t len = strlen(target);
    if (len > strlen(src))
    {
        return false;
    }
    return strncmp(src, target, len) == 0 ? true : false;
}

bool simple_string_end_with(const char* src, const char* target)
{
    size_t len = strlen(target);
    size_t src_size = strlen(src);
    if (len > src_size)
    {
        return false;
    }
    //move pointer
    char* tmp = (char*) (src + (src_size - len));
    return strncmp(tmp, target, len) == 0 ? true : false;
}

int simple_string_substr(const char* src, int begin_index, int end_index, /*out*/char new_str[])
{
    ASSERT(strlen(src) >= end_index, "end_index out of range.");
    int size = (end_index - begin_index);
    if (size < 0 && (strlen(src) - 1) < end_index)
    {
        return -1;
    }
    char* tmp = (char*) (src + begin_index);
    strncpy(new_str, tmp, size);
    return 0;
}

int simple_string_cmp_ignore_case(const char* str1, const char* str2)
{
    ASSERT(
            strlen(str1) < STRING_TOOL_STR_LEN,
            "Length(%d) of input str1 more than STRING_TOOL_STR_LEN(%d).", strlen(str1), STRING_TOOL_STR_LEN);
    ASSERT(
            strlen(str2) < STRING_TOOL_STR_LEN,
            "Length(%d) of input str2 more than STRING_TOOL_STR_LEN(%d).", strlen(str2), STRING_TOOL_STR_LEN);
    char buf1[STRING_TOOL_STR_LEN] = { 0 };
    char buf2[STRING_TOOL_STR_LEN] = { 0 };
    strcpy(buf1, str1);
    strcpy(buf2, str2);
    char* s1 = simple_string_to_upper(buf1);
    char* s2 = simple_string_to_upper(buf2);
    return strcmp(s1, s2);
}

char* simple_string_to_upper(char* self)
{
    char* tmp = (char*) self;
    while (*tmp != '\0')
    {
        *tmp = toupper(*tmp);
        tmp++;
    }
    return self;
}

char* simple_string_to_lower(char* self)
{
    char* tmp = (char*) self;
    while (*tmp != '\0')
    {
        *tmp = tolower(*tmp);
        tmp++;
    }
    return self;
}

void simple_string_left_trim(const char* self, char c, char buf[])
{
    char* s = (char*)self;
    while (*s != 0 && *s == c)
    {
        s++;
    }
    strcpy(buf, s);
}

void simple_string_right_trim(const char* self, char c, char buf[])
{
    strcpy(buf, self);
    char *p = buf, *q = buf;
    while (*p != 0)
    {
        if (*p != c)
        {
            q = p;
            q++;
        }
        p++;
    }
    if (q != self)
    {
        *q = 0;
    }
}

void simple_string_trim(const char* self, char c, char buf[])
{
    char tmp_buf[STRING_TOOL_STR_LEN] = {0};
    simple_string_left_trim(self, c, tmp_buf);
    simple_string_right_trim(tmp_buf, c, buf);
}

ArrayList* simple_string_split(const char* self, const char* regex)
{
    ArrayList* values = arraylist_new(-1);
    char* buffer = strdup(self);
    char* buf = buffer;
    char* ptr = NULL;
    char* sub = NULL;
    while ((sub = strtok_r(buf, regex, &ptr)) != NULL)
    {
        char* value = malloc(sizeof(strlen(sub) + 1));
        bzero(value, strlen(sub) + 1);
        memcpy(value, sub, strlen(sub));
        arraylist_add(values, value);
        buf = NULL;
    }
    free(buffer);
    return values;
}

