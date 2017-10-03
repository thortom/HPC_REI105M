#ifndef PERSON_H_INCLUDED
#define PERSON_H_INCLUDED
#include "environment.h"

typedef int Gender;
#define male 1
#define female 2

typedef struct PersonIdentity {
    int rank;
    Gender gender;
    int group_rank;
    int pepperoni;
    int pizza_size;
    int is_window_open;
} Person;

void Person_constructor(Person *me, int rank, int gender,
                            int group_rank);
int get_pepperoni_count(Person *me);
bool done_with_pizza(Person *me);
void take_bite_of_pizza(Person *me);
void open_window(Person *me);
void close_window(Person *me);
void answer_phone(Person *me);
char* get_group_name(Gender gender);
void get_my_name(Person *me, char* my_name);
void print_info(Person *me);

#endif
