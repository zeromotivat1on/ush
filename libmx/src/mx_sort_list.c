#include "../inc/libmx.h"

t_list *mx_sort_list(t_list *lst, bool (*cmp)(void *, void *)){
    int size  = mx_list_size(lst);
    if(lst && cmp)
       for (int i = 0; i < size - 1; ++i){
            t_list *new_temp = lst;
            while(new_temp->next){
                if (cmp(new_temp->data, new_temp->next->data)) {
                    void *temp = new_temp->data;
                    new_temp->data = new_temp->next->data;
                    new_temp->next->data = temp;
                }
                new_temp = new_temp->next;
            }
       }
    return lst;
}

