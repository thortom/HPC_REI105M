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
