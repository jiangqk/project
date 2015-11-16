/*
 *  rbt_imei.c
 *
 *  Created on: Nov 28, 2014
 *      Author: buyuanyuan
 *      Fixed : chenzutao
 */

#include "rbt.h"

static RBT_IMEI *rotate_left(RBT_IMEI *node, RBT_IMEI *root)
{
	RBT_IMEI *right = node->right;
	if((node->right = right->left)) {
		right->left->parent = node;
	}
	right->left = node;
	if((right->parent = node->parent)) {
		if(node == node->parent->right) {
			node->parent->right = right;
		} else {
			node->parent->left = right;
		}
	} else {
		root = right;
	}
	node->parent = right;
	return root;
}

static RBT_IMEI *rotate_right(RBT_IMEI *node, RBT_IMEI *root)
{
	RBT_IMEI *left = node->left;
	if((node->left = left->right)) {
		left->right->parent = node;
	}
	left->right = node;
	if((left->parent = node->parent)) {
		if(node == node->parent->right) {
			node->parent->right = left;
		} else {
			node->parent->left = left;
		}
	} else {
		root = left;
	}
	node->parent = left;
	return root;
}

static void insert_case(RBT_IMEI *node, RBT_IMEI **root)
{
	RBT_IMEI *parent, *gparent, *uncle, *tmp;
	while ((parent = node->parent) && parent->color == RED){
		gparent = parent->parent;
		if (parent == gparent->left){
			uncle = gparent->right;
			if (uncle && uncle->color == RED){
				uncle->color = BLACK;
				parent->color = BLACK;
				gparent->color = RED;
				node = gparent;
			}else{
				if (parent->right == node){
					*root = rotate_left(parent, *root);
					tmp = parent;
					parent = node;
					node = tmp;
				}
				parent->color = BLACK;
				gparent->color = RED;
				*root = rotate_right(gparent, *root);
			}
		}else{
			uncle = gparent->left;
			if (uncle && uncle->color == RED){
				uncle->color = BLACK;
				parent->color = BLACK;
				gparent->color = RED;
				node = gparent;
			}else{
				if (parent->left == node){
					*root = rotate_right(parent, *root);
					tmp = parent;
					parent = node;
					node = tmp;
				}
				parent->color = BLACK;
				gparent->color = RED;
				*root = rotate_left(gparent, *root);
			}
		}
	}
	(*root)->color = BLACK;
}

static RBT_IMEI *delete_case(RBT_IMEI *node, RBT_IMEI *parent, RBT_IMEI *root)
{
	RBT_IMEI *other, *o_left, *o_right;
	while((!node || node->color == BLACK) && node != root) {
		if(parent->left == node) {
			other = parent->right;
			if(other->color == RED) {
				other->color = BLACK;
				parent->color = RED;
				root = rotate_left(parent, root);
				other = parent->right;
			}
			if((!other->left || other->left->color == BLACK) &&
					(!other->right || other->right->color == BLACK)) {
				other->color = RED;
				node = parent;
				parent = node->parent;
			} else {
				if(!other->right || other->right->color == BLACK) {
					if((o_left = other->left)) {
						o_left->color = BLACK;
					}
					other->color = RED;
					root = rotate_right(other, root);
					other = parent->right;
				}
				other->color = parent->color;
				parent->color = BLACK;
				if(other->right) {
					other->right->color = BLACK;
				}
				root = rotate_left(parent, root);
				node = root;
				break;
			}
		} else {
			other = parent->left;
			if(other->color == RED) {
				other->color = BLACK;
				parent->color = RED;
				root = rotate_right(parent, root);
				other = parent->left;
			}
			if((!other->left || other->left->color == BLACK) &&
					(!other->right || other->right->color == BLACK)) {
				other->color = RED;
				node = parent;
				parent = node->parent;
			} else {
				if(!other->left || other->left->color == BLACK) {
					if((o_right = other->right)) {
						o_right->color = BLACK;
					}
					other->color = RED;
					root = rotate_left(other, root);
					other = parent->left;
				}
				other->color = parent->color;
				parent->color = BLACK;
				if(other->left) {
					other->left->color = BLACK;
				}
				root = rotate_right(parent, root);
				node = root;
				break;
			}
		}
	}
	if(node) {
		node->color = BLACK;
	}
	return root;
}

static RBT_IMEI *search_data(long long IMEI, RBT_IMEI *root, RBT_IMEI **save)
{
	if(root == NULL) {
		return NULL;
	}
	RBT_IMEI *node = root, *parent = NULL;
	while (node) {
		parent = node;
		if (node->IMEI > IMEI) {
			node = node->left;
		} else if (node->IMEI < IMEI) {
			node = node->right;
		} else {
			return node;
		}
	}
	if (save) {
		*save = parent;
	}
	return NULL;
}

int imei_insert(RBT_IMEI **root, ROAD_INFO *road, long long IMEI, int speed, long time)
{
	RBT_IMEI *parent = NULL, *node = NULL;
	if ((node = search_data(IMEI, *root, &parent))) {
		if(road_insert(&(node->road), road) == -1) {
			return -1;
		}
		node->speed = speed;
		node->time = time;
		return 0;
	}

	node = (RBT_IMEI *)malloc(sizeof(RBT_IMEI));
	if(!node) {
		x_printf(E,  "rbt imei node malloc error!\n");
		return -1;
	}

        node->IMEI = IMEI;
	node->road = NULL;
	if(road_insert(&(node->road), road) == -1) {
		return -1;
	}
	node->speed = speed;
	node->time = time;
        node->parent = parent;
	node->left = node->right = NULL;
	node->color = RED;
	if (parent) {
                if(parent->IMEI > IMEI) {
                        parent->left = node;
                }
                else {
                        parent->right = node;
                }

	} else {
		*root = node;
	}
	insert_case(node, root);

        return 0;
}

static RBT_IMEI *_delete(RBT_IMEI *root, long IMEI)
{
	RBT_IMEI *child, *parent, *old, *left, *node;
	Color color;
	if (!(node = search_data(IMEI, root, NULL))) {
		return root;
	}
	old = node;
	if (node->left && node->right) {
		node = node->right;
		while ((left = node->left) != NULL) {
			node = left;
		}
		child = node->right;
		parent = node->parent;
		color = node->color;
		if (child) {
			child->parent = parent;
		}
		if (parent) {
			if (parent->left == node) {
				parent->left = child;
			} else {
				parent->right = child;
			}
		} else {
			root = child;
		}
		if (node->parent == old) {
			parent = node;
		}
		node->parent = old->parent;
		node->color = old->color;
		node->right = old->right;
		node->left = old->left;
		if (old->parent) {
			if (old->parent->left == old) {
				old->parent->left = node;
			} else {
				old->parent->right = node;
			}
		} else {
			root = node;
		}
		old->left->parent = node;
		if(old->right) {
			old->right->parent = node;
		}
	} else {
		if (!node->left) {
			child = node->right;
		}
		else if (!node->right) {
			child = node->left;
		}
		parent = node->parent;
		color = node->color;
		if (child) {
			child->parent = parent;
		}
		if (parent) {
			if (parent->left == node) {
				parent->left = child;
			} else {
				parent->right = child;
			}
		} else {
			root = child;
		}
	}
	road_destory(old->road);
	free(old), old = NULL;
	if (color == BLACK) {
		root = delete_case(child, parent, root);
	}
	return root;
}

ROAD_INFO *get_imei_road(RBT_IMEI *root, long long IMEI)
{
	RBT_IMEI *parent = NULL, *node = NULL;
	if((node = search_data(IMEI, root, &parent))) {
		return node->road;
	}
	return NULL;
}

char *get_redis_road_info_value(RBT_IMEI *root, long long IMEI)
{
	RBT_IMEI *parent = NULL, *node = NULL;
        if((node = search_data(IMEI, root, &parent))) {
		return get_road_id(node->road);
	}
	return NULL;
}

int imei_destory(RBT_IMEI **root)
{
	while(*root != NULL) {
                *root = _delete(*root, (*root)->IMEI);
        }
	return 0;
}

/*
int main()
{
        RBT_IMEI *root = NULL;
        ROAD_INFO *road_info = (ROAD_INFO *)malloc(sizeof(ROAD_INFO));
        //IMEI,speed, time,{roadID, roadName, cityCode, length}
        road_info->roadID = 76135181;
        road_info->roadName = "长宁路";
        road_info->length = 182;
        road_info->cityCode = 310105;

        long long IMEI = 199563123936856;
        int speed = 85;
        long time = 1407210986;

	imei_insert(&root, IMEI, speed, time, road_info);
        ROAD_INFO *imei_road_info = get_imei_road(root, IMEI);
	x_printf(D,  "roadID:%ld, roadName:%s, length:%d, cityCode:%ld\n", road_info->roadID, road_info->roadName, road_info->length, road_info->cityCode);
	imei_insert(&root, IMEI+2, speed * 2, time + 3, road_info);
        char *road_id = get_road_info(root, IMEI);
	x_printf(D,  "road_info:%s\n",road_id);
	x_printf(D,  "roadID:%ld, roadName:%s, length:%d, cityCode:%ld\n", road_info->roadID, road_info->roadName, road_info->length, road_info->cityCode);
	imei_insert(&root, IMEI, speed +15, time + 5, road_info);
	free(road_info);
	free(road_id);
        imei_destory(&root);
}

*/
