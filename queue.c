#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

char *val(struct list_head *l);

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (new) {
        INIT_LIST_HEAD(new);
    }
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    struct list_head *tmp1, *tmp2;
    list_for_each_safe (tmp1, tmp2, l) {
        list_del(tmp1);
        element_t *tmp = list_entry(tmp1, element_t, list);
        q_release_element(tmp);
    }
    free(l);
}
/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    // create a new element_t
    element_t *new_ele = malloc(sizeof(element_t));
    if (!new_ele)
        return false;

    // init element_t new_ele
    INIT_LIST_HEAD(&new_ele->list);

    size_t len = strlen(s);
    new_ele->value = malloc(sizeof(char) * (len + 1));

    // if allocation failed, free and return
    if (!new_ele->value) {
        free(new_ele);
        return false;
    }
    strncpy(new_ele->value, s, len);
    new_ele->value[len] = '\0';

    list_add(&new_ele->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    // create a new element_t
    element_t *new_ele = malloc(sizeof(element_t));
    if (!new_ele)
        return false;

    // init element_t new_ele
    INIT_LIST_HEAD(&new_ele->list);

    size_t len = strlen(s);
    new_ele->value = malloc(sizeof(char) * (len + 1));
    // if allocation failed, free and return
    if (!new_ele->value) {
        free(new_ele);
        return false;
    }
    strncpy(new_ele->value, s, len);
    new_ele->value[len] = '\0';

    list_add_tail(&new_ele->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_first_entry(head, element_t, list);
    list_del(&node->list);

    if (sp != NULL && bufsize > 1) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return node;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_entry(head->prev, element_t, list);
    list_del(&node->list);

    if (sp != NULL && bufsize) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return node;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    // check if head == NULL or queue is empty
    if (!head || list_empty(head))
        return false;

    // Use fast and slow pointer technique
    // pointer of pointer is to prevent using another pointer to record the
    // previous node
    struct list_head *mid = head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        mid = mid->next;
    }
    //"indir" is the middle list_node, we want to delete this element_t
    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    struct list_head *node = head->next;
    // iterate node and check whether there are duplicates
    // if val(node) == val(node->next), there are duplicates
    // and we have to check whether node->next exists
    while (node != head) {
        if (node->next != head && strcmp(val(node), val(node->next)) == 0) {
            // there are duplicates, iterate to delete the node
            char *tar_val = strdup(val(node));  // copy the target value
            while (node != head && strcmp(val(node), tar_val) == 0) {
                struct list_head *del = node;
                node = node->next;
                // delete del
                list_del(del);
                q_release_element(list_entry(del, element_t, list));
            }
            free(tar_val);
        } else {
            node = node->next;
        }
    }
    return true;
}

char *val(struct list_head *l)
{
    return (list_entry(l, element_t, list))->value;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // Dealing with element_t or list_head is complicated
    // So i decide to swap the value in element_t
    // Use tar to iterate the list
    struct list_head *tar = head->next;
    while (tar != head && tar->next != head) {
        char *val1 = val(tar);
        // Swap
        list_entry(tar, element_t, list)->value =
            list_entry(tar->next, element_t, list)->value;
        list_entry(tar->next, element_t, list)->value = val1;
        // then move tar to next pair
        tar = tar->next->next;
    }
    return;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // use list_move
    struct list_head *tail = head;
    while (tail->next != head) {
        list_move(head->prev, tail);
        tail = tail->next;
    }
    return;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
