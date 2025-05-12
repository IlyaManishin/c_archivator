#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "haffman_tree.h"
#include "settings.h"

static int int_compare(int n1, int n2)
{
    if (n1 == n2)
        return 0;
    if (n1 > n2)
        return 1;
    return -1;
}

int tree_item_sort_key(const void *t1Ptr, const void *t2Ptr)
{
    TTreePoint *t1 = *(TTreePoint **)t1Ptr;
    TTreePoint *t2 = *(TTreePoint **)t2Ptr;

    if (t1->freq == t2->freq)
    {
        return int_compare(t1->value, t2->value);
    }
    return -int_compare(t1->freq, t2->freq);
}

TTreePoint *get_tree_point()
{
    TTreePoint *item = (TTreePoint *)malloc(sizeof(TTreePoint));
    item->left = NULL;
    item->right = NULL;
    return item;
}

TTreeArr *get_tree_arr_by_freq(uint64_t *freqs, int used_codes_count)
{
    TTreeArr *treeArr = (TTreeArr *)malloc(sizeof(TTreeArr));
    treeArr->items = (TTreePoint **)malloc(used_codes_count * sizeof(TTreePoint *));
    treeArr->length = used_codes_count;

    int count = 0;
    for (int i = 0; i < MAX_CODES_COUNT; i++)
    {
        if (freqs[i] == 0)
            continue;

        TTreePoint *item = get_tree_point();
        item->freq = freqs[i];
        item->value = i;
        item->type = leafType;
        treeArr->items[count] = item;
        count++;
    }
    qsort(treeArr->items, treeArr->length, sizeof(TTreePoint *), tree_item_sort_key);
    return treeArr;
}

TTreePoint *tree_arr_pop(TTreeArr *arr)
{
    TTreePoint *minItem = arr->items[arr->length - 1];
    arr->length--;
    return minItem;
}

static void tree_arr_right_offset(TTreeArr *arr, int startIndex)
{
    for (int i = arr->length; i > startIndex; i--)
    {
        arr->items[i] = arr->items[i - 1];
    }
    arr->length++;
}

void tree_arr_min_append(TTreeArr *arr, TTreePoint *item)
{
    if (arr->length == 0)
    {
        arr->items[0] = item;
        arr->length++;
        return;
    }
    if (arr->items[0]->freq <= item->freq)
    {
        tree_arr_right_offset(arr, 0);
        arr->items[0] = item;
        return;
    }

    for (int i = 1; i < arr->length; i++)
    {
        if (arr->items[i - 1]->freq > item->freq && item->freq >= arr->items[i]->freq)
        {
            tree_arr_right_offset(arr, i);
            arr->items[i] = item;
            return;
        }
    }
    arr->items[arr->length] = item;
    arr->length++;
}

void delete_tree(TTreePoint *tree)
{
    if (tree == NULL)
        return;

    TTreePoint *left = tree->left;
    TTreePoint *right = tree->right;
    free(tree);
    delete_tree(left);
    delete_tree(right);
}
