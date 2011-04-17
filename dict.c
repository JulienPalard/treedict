#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "dict.h"

unsigned char P[256] = {35, 182, 251, 233, 217, 70, 220, 155, 93, 110, 163, 43, 204, 48, 186, 57, 254, 147, 159, 2, 201, 154, 191, 151, 249, 219, 229, 75, 231, 79, 114, 45, 171, 122, 65, 142, 64, 194, 202, 225, 68, 9, 86, 52, 32, 39, 54, 247, 99, 253, 252, 38, 177, 3, 115, 60, 73, 205, 248, 83, 12, 135, 25, 208, 63, 187, 29, 59, 69, 230, 152, 87, 255, 102, 138, 36, 238, 133, 221, 66, 136, 192, 18, 92, 111, 129, 197, 210, 125, 131, 76, 162, 6, 0, 62, 160, 4, 11, 175, 51, 224, 166, 183, 34, 128, 40, 105, 178, 188, 165, 153, 113, 181, 26, 156, 172, 232, 47, 53, 139, 141, 42, 195, 77, 239, 246, 245, 103, 22, 101, 121, 144, 112, 241, 94, 244, 223, 41, 81, 23, 46, 98, 240, 90, 214, 89, 80, 234, 97, 1, 127, 28, 10, 55, 56, 130, 203, 24, 222, 17, 227, 58, 7, 211, 169, 50, 193, 20, 117, 213, 143, 212, 37, 100, 170, 150, 31, 82, 185, 137, 108, 27, 215, 124, 237, 218, 106, 123, 119, 196, 179, 180, 250, 67, 14, 140, 228, 44, 242, 216, 95, 198, 226, 107, 132, 120, 74, 206, 236, 199, 243, 146, 13, 200, 15, 16, 21, 161, 85, 158, 30, 5, 157, 72, 145, 189, 19, 104, 49, 149, 173, 8, 207, 116, 164, 190, 184, 96, 168, 235, 71, 174, 148, 84, 167, 61, 33, 134, 88, 78, 91, 126, 209, 176, 118, 109};

unsigned int default_hash(const char *string, unsigned int depth)
{
    unsigned int hash;
    unsigned int i;
    unsigned int len;

    i = depth;
    hash = 1;
    len = strlen(string) + 1;
    srandom(string[0]);
    hash = random() % 256;
    while (i < len)
    {
        hash ^= string[i++];
        hash ^= random() % 256;
        hash = P[hash];
    }
    i = 0;
    while (i < depth)
    {
        hash ^= string[i++ % len];
        hash ^= random() % 256;
        hash = P[hash];
    }
    return hash;
}

node_t *node_new()
{
    return (node_t*)calloc(1, sizeof(node_t));
}

dict_t *dict_new(delete_handler_t delete_handler, hash_function_t hash)
{
    dict_t     *dict;
    node_t     *node;

    dict = (dict_t*)malloc(sizeof(*dict));
    node = node_new();
    if (dict == NULL)
        return NULL;
    if (node == NULL)
    {
        free(dict);
        return NULL;
    }
    if (hash != NULL)
        dict->hash = hash;
    else
        dict->hash = default_hash;
    dict->root = node;
    dict->delete_handler = delete_handler;
    return dict;
}

void insert_inplace(int hash, node_t *node, const char *key, void *value)
{
    node->keys[hash] = key;
    node->values[hash] = value;
}

node_t *push_branch(int hash, dict_t *dict, node_t *node, unsigned int level)
{
    node_t          *new_node;
    unsigned int    new_hash;

    new_node = node_new();
    if (new_node == NULL)
        return NULL;
    new_node->hash = hash;
    new_node->parent = node;
    new_hash = dict->hash(node->keys[hash], level) % DICT_WIDTH;
    new_node->keys[new_hash] = node->keys[hash];
    new_node->values[new_hash] = node->values[hash];
    node->values[hash] = new_node;
    node->keys[hash] = follow;
    return new_node;
}

node_t *node_add(dict_t *dict, node_t *node, const char *key, void *value,
    unsigned int level)
{
    int hash;

    hash = dict->hash(key, level) % DICT_WIDTH;
    if (node->keys[hash] == follow)
    {
        return node_add(dict, node->values[hash], key, value, level + 1);
    }
    else if (node->keys[hash] == NULL)
    {
        insert_inplace(hash, node, key, value);
        return node;
    }
    else
    {
        if (strcmp(node->keys[hash], key) == 0)
        {
            insert_inplace(hash, node, key, value);
            return node;
        }
        else
        {
            if (push_branch(hash, dict, node, level + 1) == NULL)
                return NULL;
            return node_add(dict, node->values[hash], key, value, level + 1);
        }
    }
}

void *node_get(dict_t *dict, node_t *node, const char *key,
              unsigned int level)
{
    int hash;

    hash = dict->hash(key, level) % DICT_WIDTH;
    if (node->keys[hash] == follow)
    {
        return node_get(dict, node->values[hash], key, level + 1);
    }
    else if (node->keys[hash] == NULL)
    {
        return NULL;
    }
    else
    {
        if (strcmp(node->keys[hash], key) == 0)
        {
            return node->values[hash];
        }
        else
        {
            return NULL;
        }
    }
}

void free_node(node_t *node)
{
    unsigned int hash;

    if (node->parent == NULL)
        return ;
    for (hash = 0; hash < DICT_WIDTH; ++hash)
        if (node->keys[hash] != NULL)
            return ;
    node->parent->keys[node->hash] = NULL;
    free_node(node->parent);
    free(node);
}

unsigned int node_rm(dict_t *dict, node_t *node, const char *key,
                     unsigned int level)
{
    int hash;

    hash = dict->hash(key, level) % DICT_WIDTH;
    if (node->keys[hash] == follow)
    {
        return node_rm(dict, node->values[hash], key, level + 1);
    }
    else if (node->keys[hash] == NULL)
    {
        return 0;
    }
    else
    {
        if (strcmp(node->keys[hash], key) == 0)
        {
            if (dict->delete_handler != NULL)
                dict->delete_handler(node->keys[hash], node->values[hash]);
            node->keys[hash] = NULL;
            free_node(node);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

node_t *dict_add(dict_t *dict, const char *key, void *value)
{
    return node_add(dict, dict->root, key, value, 0);
}

void *dict_get(dict_t *dict, const char *key)
{
    return node_get(dict, dict->root, key, 0);
}

unsigned int dict_rm(dict_t *dict, const char *key)
{
    return node_rm(dict, dict->root, key, 0);
}

void dump_node(node_t *node, unsigned int level, unsigned int unused[DICT_WIDTH],
               unsigned int link[DICT_WIDTH], unsigned int data[DICT_WIDTH],
               unsigned int levels[DICT_WIDTH])
{
    unsigned int i;

    levels[level] += 1;
    for (i = 0; i < level * 4; ++i)
        printf(" ");
    for (i = 0; i < DICT_WIDTH; ++i)
    {
        if (node->keys[i] == follow)
        {
            link[level] += 1;
            printf("v");
        }
        else if (node->keys[i] == NULL)
        {
            unused[level] += 1;
            printf(".");
        }
        else
        {
            data[level] += 1;
            printf("#");
        }
    }
    printf("\n");
    for (i = 0; i < DICT_WIDTH; ++i)
        if (node->keys[i] == follow)
            dump_node(node->values[i], level + 1, unused, link, data, levels);
}

void dump_dict(dict_t *dict)
{
    unsigned int unused[DICT_WIDTH];
    unsigned int total_unused;
    unsigned int link[DICT_WIDTH];
    unsigned int total_links;
    unsigned int data[DICT_WIDTH];
    unsigned int total_datas;
    unsigned int levels[DICT_WIDTH];
    unsigned int level;

    total_datas = 0;
    total_links = 0;
    total_unused = 0;
    for (level = 0; level < DICT_WIDTH; ++level)
    {
        levels[level] = 0;
        unused[level] = 0;
        link[level] = 0;
        data[level] = 0;
    }
    dump_node(dict->root, 0, unused, link, data, levels);
    for (level = 0; levels[level] != 0; level++)
    {
        total_unused += unused[level];
        total_datas += data[level];
        total_links += link[level];
        printf("Level %i: %i\t", level, levels[level]);
        printf("Unused:%d, Links:%d, Data:%d\t", unused[level], link[level], data[level]);
        printf("Usage: %.2f%%\n", (100.0 * (data[level] + link[level]))
                                     / (unused[level] + link[level] + data[level]));
    }
    printf("Total : Unused:%d, Links:%d, Data:%d\t", total_unused, total_links, total_datas);
    printf("Usage: %.2f%%\n", (100.0 * (total_datas + total_links))
                                     / (total_unused + total_links + total_datas));
}



/* ========================================================================== */


void delete_handler(const char *key, void *value)
{
    value = value;
    free((void*)key);
}

void        test_with_urandom()
{
    char    *string;
    char    *key;
    size_t  size;
    ssize_t str_length;
    dict_t  *dict;
    size_t  count;

    count = 4000;
    string = NULL;
    dict = dict_new(NULL, NULL);
    size = 0;
    while ((str_length = getline(&string, &size, stdin)) != -1)
    {
        while (str_length > 0 && (string[str_length - 1] == '\n'
                                  || string[str_length - 1] == '\r'))
        {
            string[str_length - 1] = '\0';
            str_length -= 1;
        }
        key = strdup(string);
        dict_add(dict, key, key);
        count -= 1;
        if (count == 0)
        {
            dump_dict(dict);
            exit(EXIT_SUCCESS);
        }
    }
    if (string != NULL)
        free(string);
}


int main(void)
{
    test_with_urandom();
    return EXIT_SUCCESS;
}
