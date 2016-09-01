/**
 * Key1 = Value1
 * Key2 = Value2
 */

#ifndef LEMON_CONF_H_
#define LEMON_CONF_H_

#include <stdbool.h>

#define CONF_KEY_LEN 64
#define CONF_VALUE_LEN 256
#define CONF_STRING_MAX_LEN 1024

// TODO add the 'simple_' prefix

typedef int ConfCompare(const char* s1, const char* s2);

struct ConfNode {
    char key[CONF_KEY_LEN];
    char value[CONF_VALUE_LEN];
    struct ConfNode* next;
};

typedef struct Conf {
    struct ConfNode* head;
    struct ConfNode* cursor;
}Conf;

Conf* conf_create();

void  conf_destroy(Conf* conf);

char* conf_get(Conf* conf, const char* key);

char* conf_get_with_compare(Conf* conf, const char* key, ConfCompare* cmp);

void  conf_set(Conf* conf, const char* key, const char* value);

void  conf_load(Conf* conf, const char* load_file);

#endif /* CONF_H_ */
