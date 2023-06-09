#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
    float xmin;
    float xmax;
    float ymin;
    float ymax;
} MBR;

typedef struct node
{
    int is_leaf;
    int count;
    MBR *mbrs;
    struct node **child;
    struct node *parent;
} NODE;

typedef struct tree
{
    int min_child;
    int max_child;
    NODE *root;
} RTree;

int overlaps(MBR mbr1, MBR mbr2)
{
    return !(mbr1.xmax < mbr2.xmin || mbr1.xmin > mbr2.xmax || mbr1.ymax < mbr2.ymin || mbr1.ymin > mbr2.ymax);
}

void search_rtree(NODE *node, MBR *search_rect, int *result, int *count)
{
    int i;

    if (node->is_leaf)
    {
        // Search leaf node
        for (i = 0; i < node->count; i++)
        {
            if (overlaps(node->mbrs[i], *search_rect))
            {
                result[*count] = i;
                (*count)++;
            }
        }
    }
    else
    {
        // Search subtree
        for (i = 0; i < node->count; i++)
        {
            if (overlaps(node->mbrs[i], *search_rect))
            {
                search_rtree(node->child[i], search_rect, result, count);
            }
        }
    }
}

int create_node(int min_child, int max_child, int is_leaf, NODE *parent)
{
    NODE *node = (NODE *)malloc(sizeof(NODE));
    node->is_leaf = is_leaf;
    node->count = 0;
    node->mbrs = (MBR *)malloc(sizeof(MBR) * max_child);
    node->child = (NODE **)malloc(sizeof(NODE *) * max_child);
    node->parent = parent;

    for (int i = 0; i < max_child; i++)
    {
        node->mbrs[i].xmin = 0;
        node->mbrs[i].xmax = 0;
        node->mbrs[i].ymin = 0;
        node->mbrs[i].ymax = 0;
        node->child[i] = NULL;
    }

    return node;
}

void adjust_tree(NODE *leafNode, NODE *newLeaf)
{
}

NODE *chooseLeaf(NODE *node, MBR *mbr);

void insert(RTree *rtree, MBR *mbr)
{
    NODE *leafNode = chooseLeaf(rtree->root, mbr);
    if (leafNode->count < rtree->max_child)
    {
        leafNode->mbrs[leafNode->count] = *mbr;
        leafNode->count++;
    }
    else
    {
        NODE *newLeaf = split_node(leafNode, mbr);
        adjust_tree(leafNode, newLeaf);
        // checking if the node has split
        if (leafNode == rtree->root)
        {
            NODE *newRoot = create_node(0, 2, 1, NULL);
            newRoot->child[0] = leafNode; // Assign resulting nodes as children of new root
            newRoot->child[1] = newLeaf;
            update_mbr(newRoot); // Update MBR of new root
            rtree->root = newRoot;
        }
    }
}

float calculateEnlargement(MBR mbr1, MBR mbr2)
{
    float area = (mbr2.xmax - mbr2.xmin) * (mbr2.ymax - mbr2.ymin) - (mbr1.xmax - mbr1.xmin) * (mbr1.ymax - mbr1.ymin);
    return area;
}
float calculateArea(MBR mbr)
{
    return (mbr.xmax - mbr.xmin) * (mbr.ymax - mbr.ymin);
}

NODE *chooseLeaf(NODE *node, MBR *mbr) // It take a pointer to root node and returns a pointer to leaf node where the new entry must be placed.
{
    NODE *currentNode = node;
    while (!currentNode->is_leaf)
    {
        int bestIndex = 0;
        float bestEnlargement = RLIM_INFINITY;
        for (int i = 0; i < currentNode->count; i++)
        {
            float enlargement = calculateEnlargement(currentNode->mbrs[i], *mbr);
            if (enlargement < bestEnlargement)
            {
                bestEnlargement = enlargement;
                bestIndex = i;
            }
            else if (enlargement == bestEnlargement)
            {
                float currentArea = calculateArea(currentNode->mbrs[bestIndex]);
                float newArea = calculateArea(currentNode->mbrs[i]);
                if (newArea < currentArea)
                {
                    bestIndex = i;
                }
            }
        }
        currentNode = currentNode->child[bestIndex];
    }
    return currentNode;
}
