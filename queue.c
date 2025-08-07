#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *cur, *safe;
    list_for_each_entry_safe(cur, safe, head, list) {
        q_release_element(cur);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add(&new_element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add_tail(&new_element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_first_entry(head, element_t, list);
    if (sp && e->value) {
        size_t len = bufsize - 1;
        memcpy(sp, e->value, len);
        sp[len] = '\0';  // Ensure null termination
    }

    list_del(&e->list);
    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_last_entry(head, element_t, list);
    if (sp && e->value) {
        size_t len = bufsize - 1;
        memcpy(sp, e->value, len);
        sp[len] = '\0';  // Ensure null termination
    }

    list_del(&e->list);
    return e;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int count = 0;
    struct list_head *cur;
    list_for_each(cur, head)
        count++;
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head **indirect = &head->next;
    struct list_head *slow = head->next;
    struct list_head *fast = head->next;

    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        indirect = &(*indirect)->next;
        slow = slow->next;
    }

    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    if (list_is_singular(head))
        return true;  // Nothing to delete

    element_t *cur, *safe, *tmp = NULL;
    list_for_each_entry_safe(cur, safe, head, list) {
        if (&safe->list != head && strlen(cur->value) == strlen(safe->value) &&
            strncmp(cur->value, safe->value, strlen(cur->value)) == 0) {
            tmp = safe;
            list_del(&cur->list);
            q_release_element(cur);
        } else if (tmp) {
            // If we found duplicates, we need to delete the last one
            list_del(&tmp->list);
            q_release_element(tmp);
            tmp = NULL;  // Reset for next iteration
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    q_reverseK(head, q_size(head));
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_is_singular(head) || k == 1)
        return;
    struct list_head *start = head, *end = head->next;
    while (end != head) {
        int i = 1;
        while (end != head && i < k) {
            end = end->next;
            i++;
        }
        if (end == head || i < k)
            break;
        struct list_head *tmp = end->next;
        start->next->prev = NULL;
        while (end) {
            struct list_head *node = end->prev;
            start->next = end;
            end->prev = start;
            start = start->next;
            end = node;
        }
        start->next = tmp;
        tmp->prev = start;
        end = tmp;
    }
}

static int cmp(struct list_head *a, struct list_head *b, bool descend)
{
    const element_t *A = list_entry(a, element_t, list);
    const element_t *B = list_entry(b, element_t, list);
    int ret = strcmp(A->value, B->value);
    return descend ? -ret : ret;
}


struct list_head *mergeTwo(struct list_head *left,
                           struct list_head *right,
                           bool descend)
{
    struct list_head *tmp = NULL;
    struct list_head **indirect = &tmp;
    for (struct list_head **node = NULL; left && right; *node = (*node)->next) {
        node = cmp(left, right, descend) <= 0 ? &left : &right;
        *indirect = *node;
        indirect = &(*indirect)->next;
    }
    if (left)
        *indirect = left;
    if (right)
        *indirect = right;

    return tmp;
}

struct list_head *mergesort(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return head;
    struct list_head *fast = head, *slow = head;
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    fast = slow;
    slow->prev->next = NULL;
    struct list_head *left = mergesort(head, descend),
                     *right = mergesort(fast, descend);
    return mergeTwo(left, right, descend);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return;
    head->prev->next = NULL;
    head->next = mergesort(head->next, descend);
    struct list_head *cur = head->next, *prev = head;
    while (cur) {
        cur->prev = prev;
        prev = cur;
        cur = cur->next;
    }
    head->prev = prev;
    prev->next = head;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    element_t *cur;
    const char *min = NULL;
    struct list_head *node = head->prev;

    while (node != head) {
        cur = list_entry(node, element_t, list);
        node = node->prev;

        if (!min || strcmp(cur->value, min) <= 0) {
            min = cur->value;
        } else {
            list_del(&cur->list);
            q_release_element(cur);
        }
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    element_t *cur;
    const char *max = NULL;
    struct list_head *node = head->prev;

    while (node != head) {
        cur = list_entry(node, element_t, list);
        node = node->prev;

        if (!max || strcmp(cur->value, max) >= 0) {
            max = cur->value;
        } else {
            list_del(&cur->list);
            q_release_element(cur);
        }
    }

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *cur;
    queue_contex_t *target = list_first_entry(head, queue_contex_t, chain);
    struct list_head *merged = target->q;

    list_for_each(cur, head) {
        queue_contex_t *ctx = list_entry(cur, queue_contex_t, chain);
        if (ctx == target)
            continue;

        const struct list_head *q = ctx->q;
        if (!q || list_empty(q))
            continue;

        element_t *e, *safe;
        list_for_each_entry_safe(e, safe, q, list) {
            list_del(&e->list);
            list_add_tail(&e->list, merged);
        }
    }

    q_sort(merged, descend);
    return q_size(merged);
}
