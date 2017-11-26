#include "feed.h"

// Global variables
extern int finished_animals ;
extern char current_species;
extern char change_species_mode; // f = false, t = true
extern int all_food_dishes_area_busy; // f = false, t = true

// Food dishes
extern int number_of_food_dishes;
extern int current_value_of_food_dishes_semaphore;

// No output file, if to many animals exists
extern char write_output_file; // f = false, t = true



// Global mutex and semaphore
extern pthread_mutex_t logger_mutex;
extern pthread_mutex_t finished_animals_mutex;
extern pthread_mutex_t feeding_machine_mutex;
extern pthread_mutex_t statistics_mutex;
extern sem_t food_dishes_semaphore;

// Global condition variable
extern pthread_cond_t feeding_machine_cv;

void destroyMutexAndSemaphore()
{
    // Destroy mutex
    pthread_mutex_destroy(&logger_mutex);
    pthread_mutex_destroy(&finished_animals_mutex);
    pthread_mutex_destroy(&feeding_machine_mutex);
    pthread_mutex_destroy(&statistics_mutex);
    // Destroy semaphore
    sem_destroy(&food_dishes_semaphore);
    // Destroy condition variable
    pthread_cond_destroy(&feeding_machine_cv);
}

