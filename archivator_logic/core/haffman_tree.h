#ifndef HAFFMAN_TREE_H
#define HAFFMAN_TREE_H 1

#include <stdint.h>

typedef enum
{
    leafType,
    pointType,
    treeType,
} ETreeTypes;

typedef struct TTreePoint
{
    uint64_t freq;
    ETreeTypes type;

    int value;
    struct TTreePoint *left;
    struct TTreePoint *right;
} TTreePoint;

typedef struct
{
    TTreePoint **items;
    int length;
} TTreeArr;

extern TTreePoint *get_tree_point();
extern void delete_tree(TTreePoint *tree);

TTreeArr *get_tree_arr_by_freq(uint64_t *freqs, int used_codes_count);

extern TTreePoint *tree_arr_pop(TTreeArr *arr);
extern void tree_arr_min_append(TTreeArr *arr, TTreePoint *item);

#endif 