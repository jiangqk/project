/*
 *  rbt_road.c
 *
 *  Created on: Nov 28, 2014
 *      Author: buyuanyuan
 *      Fixed : chenzutao
 */

#include "rbt.h"

static RBT_ROAD *rotate_left(RBT_ROAD *node, RBT_ROAD *root)
{
	RBT_ROAD *right = node->right;
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

static RBT_ROAD *rotate_right(RBT_ROAD *node, RBT_ROAD *root)
{
	RBT_ROAD *left = node->left;
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

static void insert_case(RBT_ROAD *node, RBT_ROAD **root)
{
	RBT_ROAD *parent, *gparent, *uncle, *tmp;
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

static RBT_ROAD *delete_case(RBT_ROAD *node, RBT_ROAD *parent, RBT_ROAD *root)
{
	RBT_ROAD *other, *o_left, *o_right;
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

static RBT_ROAD *search_data(long roadID, RBT_ROAD *root, RBT_ROAD **save)
{
	if(root == NULL) {
		return NULL;
	}
	RBT_ROAD *node = root, *parent = NULL;
	while (node) {
		parent = node;
		if (node->roadID > roadID) {
			node = node->left;
		} else if (node->roadID < roadID) {
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

int insert_road_info(RBT_ROAD **root, ROAD_INFO *road, int speed, long time, long long IMEI, int point_speed[], int point_cnt)
{
	RBT_ROAD *parent = NULL, *node = NULL;
	if ((node = search_data(road->roadID, *root, &parent))) {
		int flag1 = person_insert(&(node->person), speed, time, IMEI, road->road_level, point_speed, point_cnt);
		int flag2 = person_speed(node->person, &(node->speed));
		int flag3 = road_insert(&(node->road), road);
                int flag4 = traffic_insert(&(node->road), road, node->speed);
		return (!flag1 && !flag2 && !flag3 && !flag4) ? 0 : -1;
	}

	node = (RBT_ROAD *)malloc(sizeof(RBT_ROAD));
	if(!node) {
		x_printf(E,  "malloc rbt road node error!\n");
		return -1;
	}

        node->roadID = road->roadID;
        node->road = NULL;
	node->person = NULL;
	node->speed = NULL;
	person_insert(&(node->person), speed, time, IMEI, road->road_level, point_speed, point_cnt);
	person_speed(node->person, &(node->speed));
        road_insert(&node->road, road);
        node->parent = parent;
	node->left = node->right = NULL;
	node->color = RED;
	if (parent) {
                if(parent->roadID > road->roadID) {
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

static RBT_ROAD *_delete(RBT_ROAD *root, long roadID)
{
	RBT_ROAD *child, *parent, *old, *left, *node;
	Color color;
	if (!(node = search_data(roadID, root, NULL))) {
		x_printf(D,  "roadID %ld is not exist!\n", roadID);
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
	person_destory(old->person);
	road_destory(old->road);
	free(old->speed), old->speed = NULL;
	free(old), old = NULL;
	if (color == BLACK) {
		root = delete_case(child, parent, root);
	}
	return root;
}

SPEED *get_speed_info(RBT_ROAD *root, long roadID)
{
	RBT_ROAD *parent = NULL, *node = NULL;
	if((node = search_data(roadID, root, &parent))) {
		return node->speed;
	}
        return NULL;
}

char *get_redis_speed_info_value(RBT_ROAD *root, long roadID)
{
	RBT_ROAD *parent = NULL, *node = NULL;
        if((node = search_data(roadID, root, &parent))) {
		return get_road_speed(node->speed, node->road);
	}
        return NULL;
}

char *get_redis_active_user_value(RBT_ROAD *root, long roadID)
{
	RBT_ROAD *parent = NULL, *node = NULL;
        if((node = search_data(roadID, root, &parent))) {
                return get_person_imei(node->person);
        }
        return NULL;
}

int road_delete_imei(RBT_ROAD *root, long roadID, long long IMEI)
{
	RBT_ROAD *parent = NULL, *node = NULL;
        if((node = search_data(roadID, root, &parent))) {
		return person_delete(&(node->person), IMEI);
	}
	return 0;
}

int road_delete(RBT_ROAD **root)
{
	while(*root != NULL) {
                *root = _delete(*root, (*root)->roadID);
        }
	return 0;
}

/*
int main()
{
        RBT_ROAD *root = NULL;
        insert_road_info(&root, 1, 1, 1, 1);
	insert_road_info(&root, 1, 7, 1, 1);
	insert_road_info(&root, 1, 7, 1, 2);
	insert_road_info(&root, 2, 1, 1, 1);
	road_delete_imei(root, 1, 1);
	road_delete_imei(root, 1, 2);
	road_delete_imei(root, 3, 1);
	insert_road_info(&root, 1, 4, 1, 1);
	insert_road_info(&root, 1, 3, 1, 2);
	SPEED *speed = get_speed_info(root, 1);
	char *redis_speed_info = get_redis_speed_info_value(root, 1);
	char *redis_active_user = get_redis_active_user_value(root, 2);
	if(redis_speed_info != NULL) {
		free(redis_speed_info);
	}
	if(redis_active_user != NULL) {
		free(redis_active_user);
	}
	road_delete(&root);
}

*/
