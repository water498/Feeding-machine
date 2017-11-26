/*
 * name: scheduler
 * autor: Daniel Franze
 *
 * source 1: http://stackoverflow.com/questions/17877368/getopt-passing-string-parameter-for-argument
 * source 2: http://www.jbox.dk/sanos/source/include/pthread.h.html
 * source 3: http://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
 * source 4: http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
 * source 5: http://stackoverflow.com/questions/11573974/write-to-txt-file
 * source 6: http://stackoverflow.com/questions/6154539/how-can-i-wait-for-any-all-pthreads-to-complete
 * source 7: http://stackoverflow.com/questions/26900122/c-program-to-print-current-time
 * source 8: http://stackoverflow.com/questions/3930363/implement-time-delay-in-c
 * source 9: http://stackoverflow.com/questions/5248915/execution-time-of-c-program
 *수정자 : 김상훈 노형섭
 */


#include "feed.h"



struct animal_parameters
{
    int id; 
    char species; // cat = c, dog = d, mouse = m
    int feeding_counter; // 0 will end the thread
    int eating_time;
    int statisfied_time;
};
typedef struct animal_parameters animal_params;

struct start_parameters
{
    int number_of_cats;
    int number_of_dogs;
    int number_of_mice;
    int time_a_cat_is_statisfied;
    int time_a_dog_is_statisfied;
    int time_a_mouse_is_statisfied;
    int how_many_times_a_cat_wants_to_eat;
    int how_many_times_a_dog_wants_to_eat;
    int how_many_times_a_mouse_wants_to_eat;
    int number_of_food_dishes;
    int eating_time_interval_lower_boundary;
    int eating_time_interval_upper_boundary;
    char *output_file;
    int verbose_print;
};
typedef struct start_parameters start_params;

struct statistics_of_species
{
    double cats_min;
    double cats_max;
    double cats_avg;
    double dogs_min;
    double dogs_max;
    double dogs_avg;
    double mice_min;
    double mice_max;
    double mice_avg;
    char *output_file;
    char cats_values_empty;
    char dogs_values_empty;
    char mice_values_empty;
};
typedef struct statistics_of_species statistics;

// Global variables
int finished_animals = 0;
char current_species = '0';
char change_species_mode = 'f'; // f = false, t = true
int all_food_dishes_area_busy = 'f'; // f = false, t = true

// Food dishes
int number_of_food_dishes;
int current_value_of_food_dishes_semaphore;

// No output file, if to many animals exists
char write_output_file = 't'; // f = false, t = true

// Global statistics
statistics stats;

// Global mutex and semaphore
pthread_mutex_t logger_mutex;
pthread_mutex_t finished_animals_mutex;
pthread_mutex_t feeding_machine_mutex;
pthread_mutex_t statistics_mutex;
sem_t food_dishes_semaphore;

// Global condition variable
pthread_cond_t feeding_machine_cv = PTHREAD_COND_INITIALIZER;



void printParams(start_params params)
{
    printf(YELLOW"****************************************************************************\n");
    printf("*                                   PARAMS                                 *\n");
    printf("****************************************************************************\n");
    printf("* Number of cats (Default: 6, Current: %d)                                  \n", params.number_of_cats);
    printf("* Number of dogs (Default: 4, Current: %d)                                  \n", params.number_of_dogs);
    printf("* Number of mice (Default: 2, Current: %d)                                  \n", params.number_of_mice);
    printf("* Time a cat is satisfied (Default: 15, Current: %d)                        \n", params.time_a_cat_is_statisfied);
    printf("* Time a dog is satisfied (Default: 10, Current: %d)                        \n", params.time_a_dog_is_statisfied);
    printf("* Time a mouse is satisfied  (Default: 1, Current: %d)                      \n", params.time_a_mouse_is_statisfied);
    printf("* How many times a cat wants to eat (Default: 5, Current: %d)               \n", params.how_many_times_a_cat_wants_to_eat);
    printf("* How many times a dog wants to eat (Default: 5, Current: %d)               \n", params.how_many_times_a_dog_wants_to_eat);
    printf("* How many times a mouse wants to eat (Default: 5, Current: %d)             \n", params.how_many_times_a_mouse_wants_to_eat);
    printf("* Number of Food Dishes (Default: 2, Current: %d)                           \n", params.number_of_food_dishes);
    printf("* Eating time interval lower boundary (Default: 1, Current: %d)             \n", params.eating_time_interval_lower_boundary);
    printf("* Eating time interval upper boundary (Default: 1, Current: %d)             \n", params.eating_time_interval_upper_boundary);
    printf("* Output file (Default: output_file.txt, Current: %s)                       \n", params.output_file);
    printf("****************************************************************************\n"RESET); 
}
void initMutexAndSemaphore(start_params params)
{
    // Init mutex
    pthread_mutex_init(&logger_mutex, NULL);
    pthread_mutex_init(&finished_animals_mutex, NULL);
    pthread_mutex_init(&feeding_machine_mutex, NULL);
    pthread_mutex_init(&statistics_mutex, NULL);
    // Init Semaphore
    sem_init(&food_dishes_semaphore, 0, (unsigned int)params.number_of_food_dishes);
}

int randomTime(int min, int max)
{
    srand((unsigned int)time(NULL));
    return rand()%(max-min + 1) + min;
}

void writeToFile(char *output_file, char *text_for_file)
{

    FILE *f = fopen(output_file, "a");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "%s\n", text_for_file);
    fclose(f);
}

char* getTime()
{
    time_t current_time;
    char* c_time_string;

    current_time = time(NULL);
    c_time_string = ctime(&current_time);

    return c_time_string;
}

void wait(int secs)
{
    unsigned int retTime = (unsigned int)(time(0) + secs);
    while (time(0) < retTime);
}

void statisticsOfAllSpeciesHandler(char species, double min, double max, double avg){
    pthread_mutex_lock(&statistics_mutex);
    if((stats.cats_values_empty == 'f') && (species == 'c')){
        if(min < stats.cats_min)
            stats.cats_min = min;
        if(max > stats.cats_max)
            stats.cats_max = max;
        stats.cats_avg = stats.cats_avg + avg;
    } else if((stats.dogs_values_empty == 'f') && (species == 'd')){
        if(min < stats.dogs_min)
            stats.dogs_min = min;
        if(max > stats.dogs_max)
            stats.dogs_max = max;
        stats.dogs_avg = stats.dogs_avg + avg;
    } else if((stats.mice_values_empty == 'f') && (species == 'm')){
        if(min < stats.mice_min)
            stats.mice_min = min;
        if(max > stats.mice_max)
            stats.mice_max = max;
        stats.mice_avg = stats.mice_avg + avg;
    } else if((stats.cats_values_empty == 't') && (species == 'c')){
        stats.cats_min = min;
        stats.cats_max = max;
        stats.cats_avg = avg;
        stats.cats_values_empty = 'f';
    } else if((stats.dogs_values_empty == 't') && (species == 'd')){
        stats.dogs_min = min;
        stats.dogs_max = max;
        stats.dogs_avg = avg;
        stats.dogs_values_empty = 'f';
    } else if((stats.mice_values_empty == 't') && (species == 'm')){
        stats.mice_min = min;
        stats.mice_max = max;
        stats.mice_avg = avg;
        stats.mice_values_empty = 'f';
    }
    pthread_mutex_unlock(&statistics_mutex);
}

void printStatisticsOfAllSpecies(start_params params){
    pthread_mutex_lock(&statistics_mutex);
    stats.cats_avg = stats.cats_avg / (double)params.number_of_cats;
    stats.dogs_avg = stats.dogs_avg / (double)params.number_of_dogs;
    stats.mice_avg = stats.mice_avg / (double)params.number_of_mice;

    printf("**************************************************\n");
    printf("*                   Statistics                   *\n");
    printf("**************************************************\n");
    printf("|      "MAGENTA"Cats"RESET"    |       "BLUE"Dogs"RESET"     |       "CYAN"Mice"RESET"     |\n");
    printf("|--------------|----------------|----------------|\n");
    printf("| "GREEN"Min"RESET": %04.1f    | "GREEN"Min"RESET": %04.1f      | "GREEN"Min"RESET": %04.1f      |\n", 
            stats.cats_min, stats.dogs_min, stats.mice_min);
    printf("| "RED"Max"RESET": %04.1f    | "RED"Max"RESET": %04.1f      | "RED"Max"RESET": %04.1f      |\n", 
            stats.cats_max, stats.dogs_max, stats.mice_max);
    printf("| "CYAN"Avg"RESET": %04.1f    | "CYAN"Avg"RESET": %04.1f      | "CYAN"Avg"RESET": %04.1f      |\n", 
            stats.cats_avg, stats.dogs_avg, stats.mice_avg);
    printf("**************************************************\n");
    pthread_mutex_unlock(&statistics_mutex);
}

void statisticsHandler(char species, int id, double min, double max, double avg)
{
    char *text_to_output_file = malloc(100 * sizeof(char));
    sprintf(text_to_output_file, "Species: %c | ID: %d | Min: %3.1f | Max: %3.1f | Avg: %3.1f\n", species, id, min, max, avg);
    writeToFile(stats.output_file, text_to_output_file);
    free(text_to_output_file);
}

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

void printHelper(char species, int id, char* text)
{
    time_t rawtime;
    struct tm * timeinfo;
    int current_value_of_food_dishes_semaphore_lokal;

    pthread_mutex_lock(&logger_mutex);
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    printf("["GREEN"%02d"RESET":"GREEN"%02d"RESET":"GREEN"%02d"RESET"] [", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore_lokal);
    int free_food_dishes = number_of_food_dishes - current_value_of_food_dishes_semaphore_lokal;
    int count_used_prints_of_free_food_dishes = 0;
    for (int i = 0; i < number_of_food_dishes; i++) {
        if(i){
            if(free_food_dishes - count_used_prints_of_free_food_dishes){
                printf(":"RED"x"RESET);
                count_used_prints_of_free_food_dishes++;
            } 
            else 
                printf(":-");
        }
        else{
            if(free_food_dishes - count_used_prints_of_free_food_dishes){
                printf(RED"x"RESET);
                count_used_prints_of_free_food_dishes++;
            } 
            else 
                printf("-");
        }

    }

    if(species == 'c'){
        printf("] "MAGENTA"Cat"RESET" (id %03d) %s\n", id, text);
    }else if(species == 'd'){
        printf("] "BLUE"Dog"RESET" (id %03d) %s\n", id, text);
    }else if(species == 'm'){
        printf("] "CYAN"Mouse"RESET" (id %03d) %s\n", id, text);
    }

    pthread_mutex_unlock(&logger_mutex);
}

void *feedingMachineScheduler(start_params *params)
{
    while(finished_animals != (params->number_of_cats + params->number_of_dogs + params->number_of_mice)){

        pthread_mutex_lock(&feeding_machine_mutex);
        change_species_mode = 't';
        if(current_species == '0'){
            current_species = 'c'; // Set cat
        }else if(current_species == 'c'){
            current_species = '0';
            sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
            while(current_value_of_food_dishes_semaphore != number_of_food_dishes){
                pthread_cond_wait(&feeding_machine_cv, &feeding_machine_mutex);
                sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
            }
            current_species = 'd'; // Set dog
        }else if(current_species == 'd'){
            current_species = '0';
            sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
            while(current_value_of_food_dishes_semaphore != number_of_food_dishes){
                pthread_cond_wait(&feeding_machine_cv, &feeding_machine_mutex);
                sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
            }
            current_species = 'm'; // Set mouse
        }else if(current_species == 'm'){
            current_species = '0';
            sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
            while(current_value_of_food_dishes_semaphore != number_of_food_dishes){
                pthread_cond_wait(&feeding_machine_cv, &feeding_machine_mutex);
                sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
            }
            current_species = 'c'; // Set cat
        }
        change_species_mode = 'f';
        pthread_mutex_unlock(&feeding_machine_mutex);

        pthread_cond_broadcast(&feeding_machine_cv);
        

        // Time to dispatch
        if((params->number_of_cats != 0 && current_species == 'c') 
            || (params->number_of_dogs != 0 && current_species == 'd') 
            || (params->number_of_mice != 0 && current_species == 'm'))
            wait(params->eating_time_interval_lower_boundary);
        

    }
    return NULL;
}

void *animal(animal_params *animal_args)
{
    // Measuring vars
    time_t start_time, end_time;
    double waiting_time, min, max, avg;
    char first_round_init = 't'; // t = true, f = false
    int total_number_of_rounds = animal_args->feeding_counter;

    while(animal_args->feeding_counter != 0){

        // Feeding machine scheduling condition check
        pthread_mutex_lock(&feeding_machine_mutex);
        sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
        
        // Measure the waiting time (start)
        time(&start_time);

        while(current_species != animal_args->species ||  current_value_of_food_dishes_semaphore == 0){
            pthread_cond_wait(&feeding_machine_cv, &feeding_machine_mutex);
            sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
        }

        // --> Food dishes area <--
        sem_wait(&food_dishes_semaphore);
            printHelper(animal_args->species, animal_args->id, "eating from a dish");
            pthread_mutex_unlock(&feeding_machine_mutex);
            pthread_cond_broadcast(&feeding_machine_cv);
            
            // Measure the waiting time (end)
            time(&end_time);
            waiting_time = difftime(end_time, start_time);
            if(first_round_init == 'f'){
                if(waiting_time < min)
                    min = waiting_time;
                if(waiting_time > max)
                    max = waiting_time;
                avg = avg + waiting_time;
            }else if(first_round_init == 't'){
                min = waiting_time;
                max = waiting_time;
                avg = waiting_time;
                first_round_init = 'f';
            }

            // A animal eat
            wait(animal_args->eating_time);

            pthread_mutex_lock(&feeding_machine_mutex);
        sem_post(&food_dishes_semaphore);

        printHelper(animal_args->species, animal_args->id, "finished eating from a dish");
        pthread_mutex_unlock(&feeding_machine_mutex);
        pthread_cond_broadcast(&feeding_machine_cv);

        animal_args->feeding_counter--;
        if(animal_args->feeding_counter == 0)
            break;

        // A animal is statisfied
        wait(animal_args->statisfied_time);

        
    }

    // Calc avg
    avg = avg / total_number_of_rounds;
    if(write_output_file == 't')
        statisticsHandler(animal_args->species, animal_args->id, min, max, avg);
    statisticsOfAllSpeciesHandler(animal_args->species, min, max, avg);
    pthread_mutex_lock(&finished_animals_mutex);
    finished_animals++;
    pthread_mutex_unlock(&finished_animals_mutex);

    return NULL;
}

void threadHandler(start_params params)
{
    char cat_specific_char = 'c';
    char dog_specific_char = 'd';
    char mouse_specific_char = 'm';
    int number_of_threads = params.number_of_cats + params.number_of_dogs + params.number_of_mice;
    int number_of_feeding_machines = 1;

    pthread_t animal_threads[number_of_threads];
    animal_params animal_args[number_of_threads];

    pthread_t feeding_machines[number_of_feeding_machines];

    // Init variable before starting
    number_of_food_dishes = params.number_of_food_dishes;
    sem_getvalue(&food_dishes_semaphore, &current_value_of_food_dishes_semaphore);
    stats.output_file = params.output_file;
    stats.cats_values_empty = 't';
    stats.dogs_values_empty = 't';
    stats.mice_values_empty = 't';

    // Starts all Animal-Threads:

    // Feeding machine
    for(int i=0;i<number_of_feeding_machines;i++){
        pthread_create(&feeding_machines[i],NULL,
                       (void *(*)(void *)) feedingMachineScheduler,
                       (void *) &params);
    }

    // Cats
    for(int i=0;i<params.number_of_cats;i++){
        // Init parameters
        animal_args[i].id = i;
        animal_args[i].species = cat_specific_char;
        animal_args[i].feeding_counter = params.how_many_times_a_cat_wants_to_eat;
        animal_args[i].eating_time = randomTime(params.eating_time_interval_lower_boundary, 
                                                params.eating_time_interval_upper_boundary);
        animal_args[i].statisfied_time = params.time_a_cat_is_statisfied;
        // Start thread
        pthread_create(&animal_threads[i],NULL,
                   (void *(*)(void *)) animal,
                   (void *) &animal_args[i]) ;
    }

    // Dogs
    for(int i=params.number_of_cats;i<params.number_of_cats + params.number_of_dogs;i++){
        // Init parameters
        animal_args[i].id = i;
        animal_args[i].species = dog_specific_char;
        animal_args[i].feeding_counter = params.how_many_times_a_dog_wants_to_eat;
        animal_args[i].eating_time = randomTime(params.eating_time_interval_lower_boundary, 
                                                params.eating_time_interval_upper_boundary);
        animal_args[i].statisfied_time = params.time_a_dog_is_statisfied;
        // Start thread
        pthread_create(&animal_threads[i],NULL,
                   (void *(*)(void *)) animal,
                   (void *) &animal_args[i]) ;
    }

    // Mice
    for(int i=params.number_of_cats + params.number_of_dogs;i<number_of_threads;i++){
        // Init parameters
        animal_args[i].id = i;
        animal_args[i].species = mouse_specific_char;
        animal_args[i].feeding_counter = params.how_many_times_a_mouse_wants_to_eat;
        animal_args[i].eating_time = randomTime(params.eating_time_interval_lower_boundary, 
                                                params.eating_time_interval_upper_boundary);
        animal_args[i].statisfied_time = params.time_a_mouse_is_statisfied;
        // Start thread
        pthread_create(&animal_threads[i],NULL,
                   (void *(*)(void *)) animal,
                   (void *) &animal_args[i]) ;
    }

    // Cleaning:

    // Animals
    for(int i=0;i<number_of_threads;i++){
        long *statusp;
        pthread_join(animal_threads[i],(void *)&statusp);
    }

    // Feeding machine
    for(int i=0;i<number_of_feeding_machines;i++){
        long *statusp;
        pthread_join(feeding_machines[i],(void *)&statusp);
    }
}


void runSequence(start_params params)
{
    // Init mutex and semaphore
    initMutexAndSemaphore(params);

    // Debug output (console)
    if(params.verbose_print == 1)
        printParams(params);
    
    // Starts threads
    threadHandler(params);

    // Print statistics
    printStatisticsOfAllSpecies(params);

    // Clean up mutex and semaphore
    destroyMutexAndSemaphore();
}

// Program start
int main (int argc, char *argv[])
{
    start_params params;

    // Init defaults
    params.number_of_cats = 6;
    params.number_of_dogs = 4;
    params.number_of_mice = 2;
    params.time_a_cat_is_statisfied = 15;
    params.time_a_dog_is_statisfied = 10;
    params.time_a_mouse_is_statisfied = 1;
    params.how_many_times_a_cat_wants_to_eat = 5;
    params.how_many_times_a_dog_wants_to_eat = 5;
    params.how_many_times_a_mouse_wants_to_eat = 5;
    params.number_of_food_dishes = 2;
    params.eating_time_interval_lower_boundary = 1;
    params.eating_time_interval_upper_boundary = 1;
    params.output_file = "output_file.txt";
    params.verbose_print = 0; // 1=true


    struct option long_options[] =
    {
        {"cn", required_argument, NULL, 'a'},
        {"dn", required_argument, NULL, 'b'},
        {"mn", required_argument, NULL, 'c'},
        {"ct", required_argument, NULL, 'd'},
        {"dt", required_argument, NULL, 'x'},
        {"mt", required_argument, NULL, 'f'},
        {"ce", required_argument, NULL, 'g'},
        {"de", required_argument, NULL, 'z'},
        {"me", required_argument, NULL, 'i'},
        {"e", required_argument, NULL, 'e'},
        {"E", required_argument, NULL, 'E'},
        {"dish", required_argument, NULL, 'j'},
        {"file", required_argument, NULL, 'k'},
        {"v", no_argument, NULL, 'v'},
        {"h", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "a:b:c:d:x:f:g:z:i:j:k:e:E:vh", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'a':
                params.number_of_cats = atoi(optarg);
                break;
            case 'b':
                params.number_of_dogs = atoi(optarg);
                break;
            case 'c':
                params.number_of_mice = atoi(optarg);
                break;
            case 'd':
                params.time_a_cat_is_statisfied = atoi(optarg);
                break;
            case 'x':
                params.time_a_dog_is_statisfied = atoi(optarg);
                break;
            case 'f':
                params.time_a_mouse_is_statisfied = atoi(optarg);
                break;
            case 'g':
                params.how_many_times_a_cat_wants_to_eat = atoi(optarg);
                break;
            case 'z':
                params.how_many_times_a_dog_wants_to_eat = atoi(optarg);
                break;
            case 'i':
                params.how_many_times_a_mouse_wants_to_eat = atoi(optarg);
                break;
            case 'j':
                params.number_of_food_dishes = atoi(optarg);
                break;
            case 'k':
                params.output_file = optarg;
                break;
            case 'e':
                params.eating_time_interval_lower_boundary = atoi(optarg);
                break;
            case 'E':
                params.eating_time_interval_upper_boundary = atoi(optarg);
                break;
            case 'v':
                params.verbose_print = 1;
                break;
            case 'h':
                printHelp();
                break;
        }
    }
    
    // Set global var false
    if((params.number_of_cats > 10) || (params.number_of_dogs > 10) || (params.number_of_mice > 10))
        write_output_file = 'f';

    runSequence(params);

    // End of Program
    return 0;
}
