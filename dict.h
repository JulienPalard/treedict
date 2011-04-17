#ifndef __DICT_H__
#define __DICT_H__

#define DICT_WIDTH 32

#define GET_BIT(integer, bit) (((integer) >> bit) & 1)

const char *follow = "follow";

typedef struct    s_node
{
    const char    *keys[DICT_WIDTH];
    void          *values[DICT_WIDTH];
    struct s_node *parent;
    unsigned int  hash;
}                 node_t;

typedef unsigned int (*hash_function_t)(const char *key, unsigned int level);
typedef void (*delete_handler_t)(const char *key, void *value);

typedef struct s_dict
{
    hash_function_t hash;
    delete_handler_t delete_handler;
    node_t *root;
}              dict_t;

#endif /* __DICT_H__ */
