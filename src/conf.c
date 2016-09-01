/*
 * conf.c
 *
 *  Created on: Mar 23, 2013
 *      Author: redchen
 */

#include "conf.h"
#include "assert.h"
#include "string_tool.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define CONF_SPACE_SYMBOL (' ')
#define CONF_NEWLINE_SYMBOL ('\n')

Conf* conf_new()
{
    Conf* self = malloc(sizeof(Conf));
    bzero(self, sizeof(Conf));
    self->head = NULL;
    return self;
}

static void conf_node_destroy(Conf* conf)
{
    if (conf->head != NULL)
    {
        struct ConfNode* cur = conf->head;
        struct ConfNode* tmp;
        while ((tmp = cur->next) != NULL)
        {
            free(cur);
            cur = tmp;
        }
        free(cur);
    }

}

static int conf_default_cmp(const char* s1, const char* s2)
{
    return strcmp(s1, s2);
}

char* conf_get(Conf* conf, const char* key)
{
    return conf_get_with_compare(conf, key, conf_default_cmp);
}

char* conf_get_with_compare(Conf* conf, const char* key, ConfCompare* cmp)
{
    for (struct ConfNode* cursor = conf->head; cursor != NULL; cursor = cursor->next)
    {
        if (cmp(key, cursor->key) == 0)
        {
            return cursor->value;
        }
    }
    return NULL;
}

void conf_set(Conf* conf, const char* key, const char* value)
{
    int key_len = strlen(key);
    int value_len = strlen(value);
    ASSERT(key_len < CONF_KEY_LEN,
            "Input key (%s) length (%d) more than CONF_KEY_LEN(%d)", key, key_len, CONF_KEY_LEN);
    ASSERT(value_len < CONF_VALUE_LEN,
            "Input value (%s) length (%d) more than CONF_VALUE_LEN(%d)", value, value_len, CONF_VALUE_LEN);

    struct ConfNode* node = malloc(sizeof(struct ConfNode));
    bzero(node, sizeof(struct ConfNode));

    strcpy(node->key, key);
    strcpy(node->value, value);

    node->next = conf->head;
    conf->head = node;
}

void conf_load(Conf* conf, const char* load_file)
{
    //clear 
    conf_node_destroy(conf);

    FILE* fp = fopen(load_file, "r");
    ASSERT(fp != NULL, "Open the %s is failed ! error msg : %s", load_file, strerror(errno));

    char line_buf[CONF_STRING_MAX_LEN] =  { 0 };
    char result_buf[CONF_STRING_MAX_LEN] =  { 0 };
    while (!feof(fp))
    {
        bzero(line_buf, CONF_STRING_MAX_LEN);
        bzero(result_buf, CONF_STRING_MAX_LEN);

        fgets(line_buf, CONF_STRING_MAX_LEN, fp);

        simple_string_trim(line_buf, CONF_SPACE_SYMBOL, result_buf);
        simple_string_trim(line_buf, CONF_NEWLINE_SYMBOL, result_buf);

        if (simple_string_start_with(result_buf, "#") || strlen(result_buf) <= 0)
        {
            continue;
        }
        int index = 0;

        ASSERT((index = simple_string_index_of(result_buf, "=")) >= 0, "Input line format error :'%s'.", result_buf);

        char tmp_key[CONF_STRING_MAX_LEN] = { 0 };
        char key[CONF_STRING_MAX_LEN] = { 0 };
        char tmp_value[CONF_STRING_MAX_LEN] = { 0 };
        char value[CONF_STRING_MAX_LEN] = { 0 };

        simple_string_substr(result_buf, 0, index, tmp_key);
        simple_string_substr(result_buf, index + 1, strlen(result_buf), tmp_value);

        simple_string_trim(tmp_key, CONF_SPACE_SYMBOL, key);
        simple_string_trim(tmp_value, CONF_SPACE_SYMBOL, value);
        simple_string_trim(key, CONF_NEWLINE_SYMBOL, tmp_key);
        simple_string_trim(value, CONF_NEWLINE_SYMBOL, tmp_value);

        conf_set(conf, tmp_key, tmp_value);
    }
    fclose(fp);
}

Conf* conf_create()
{
    Conf* self = malloc(sizeof(Conf));
    bzero(self, sizeof(Conf));
    self->head = self->cursor = NULL;
    return self;
}

void conf_destroy(Conf* conf)
{
    conf_node_destroy(conf);
    free(conf);
}

