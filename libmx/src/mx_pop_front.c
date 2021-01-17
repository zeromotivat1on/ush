#include "../inc/libmx.h"

void mx_pop_front(t_list **head){
    if(*head == NULL) return;
    if((*head)->next == NULL){
        free(*head);
        *head = NULL;
        return;
    }
    t_list *new_head = (*head)->next;
    free(*head);
    (*head) = NULL;
    *head = new_head;
}
