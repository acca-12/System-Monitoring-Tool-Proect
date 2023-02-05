#include <stdio.h>
#include <utmp.h>
#include <sys/utsname.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdbool.h>

/*
To the kind TA marking my assignment,

        I know that the CPU usage is incredibly low, but this was the formula I was told to use in OH and piazza.

*/

bool checkIfStringIsNumber(char *string){
    /* 
        @brief: checks if char *string is a number 
        @param: char* to a string
        @return: return true iff every character in char* (up to null) is a number
    */


    //goes until delimeter is reached '\0' has value 0
    while(*string){
        //first occurence of non numeric character will return false
        if(!isdigit(*string)){
            return false;
        }
        //goes to next character
        string = string + sizeof(char);
    }
    //code will only reach this if every character is indeed a digit
    return true;
}

void header(int samples, int interval){
    /*
        @brief: Prints the number of int samples  and the int terval of time the user has specified. Also prints the memory usage of the running program
        @param: int samples and int interval: these can be set by the user via flags
    */
        printf("Nbr of samples %d -- every %d sec\n", samples, interval);
        //struct rusage has a field ru_maxrss which returns memory size for a specified who
        struct rusage x;
        //specfying the who to be the program and will set rumaxrss appropiately
        getrusage(RUSAGE_SELF, &x);
        printf(" Memory Usage: %ld KB\n", x.ru_maxrss);
        printf("---------------------------------------\n");
}

void sequentialHeader(int iteration){
    /*
        @brief: Only prints when user specifies sequential header. The header prints the int iteration, as well as the memory usage of the running program.
        @param: int iteration: this specifies what iteration out of samples sequential is on.
    
    */
    struct rusage x;
    getrusage(RUSAGE_SELF, &x);
    printf(" >>> iteration: %d\n", iteration);
    printf(" Memory Usage: %ld KB\n", x.ru_maxrss);
    printf("---------------------------------------\n");
}

void printSystem(int iteration, int samples, double *array, int sequential){
    /*
        @brief: Calculates and prints the used and total physical/virtual memory.
        @param: int iteration: the iteration out of samples the program is currently on. int samples: the number of samples specified by user.
                double *array: used to store and retrieve used and total physical/vritual memory values. int sequential: if user wants sequential, then different print process.
        
        Explanation of printing:
        If sequential is 1 then the program prints newlines until the int iteration, where it will print the physical/virtual memory usage and total,
        and then proceeds to print newlines until int samples - 1
        If sequential is 0 then the program will print the physical/virtual memory usage and total retrieved from *array until int iteration where it will calculate 
        and then print the iteraton's memory usage. From iteration + 1 to samples - 1 inclusive, it will print new lines.

    */
    struct sysinfo memories;
    //Intializes the struct with appropiate values to be able to use
    sysinfo(&memories);
    //Calculates the memory fields in GB (documentation has the fields of struct in bytes). Conversion used: 10 ^ 9 bytes = 1 GB
    double free_physical = memories.freeram/(pow(10.0, 9));
    double free_swap = memories.freeswap/(pow(10.0, 9));
    double total_physical= memories.totalram/(pow(10.0, 9));
    double total_swap = memories.totalswap/(pow(10.0 ,9));
    //Calculates the following memory totals based on piazza post recommendations
    double total_virtual = total_physical + total_swap;
    double physical_used = total_physical - free_physical;
    double virtual_used = total_virtual - free_physical - free_swap;

    //saves values in array to be used for future iterations
    array[iteration * 4 + 0] = physical_used;
    array[iteration * 4 + 1] = total_physical;
    array[iteration * 4 + 2] = virtual_used;
    array[iteration * 4 + 3] = total_virtual;

    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
    if(sequential){
        //if sequential print blank lines except at int iteration. At int iteration print calculated values.
        for(int i = 0; i < samples; i++){
            if(i != iteration){
                printf("\n");
            }else{
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB \n", physical_used, total_physical, virtual_used, total_virtual);
            }
        }
    }else{
        //if not sequential print old values from double *array (this is a 2D array) until int iteration. At int iteration print calculated values.
        for(int i = 0; i < iteration + 1; i++){
            if(i != iteration){
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB \n", array[i * 4 + 0], array[i * 4 + 1], array[i*4 +2], array[i * 4 + 3]);
            }else{
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB \n", physical_used, total_physical, virtual_used, total_virtual);
            }
        }
        //print new line for the rest
        for(int i = iteration + 1; i < samples; i++){
            printf("\n");
        }

    }
    printf("---------------------------------------\n");

}
long int cpuInfo(long int previous, int i){
    /*
        @brief: Calculates change in CPU usage based on the long int previous summation of cpu times excluding idle time.
        @param: long int previous: this is the saved previous value of the previous iteration. This is helpful in finding the percentage use.
                int i: this specifes what iteration program is on.
        

        If int i = -1, then this is baseline iteration and it will only return the summation without printing the percetange use
        If int i != -1, then calculate cpu times without idle and take its absolute difference with long int previous and divide by the latter and multiply by 100
        to calculate and print percentage use relative to last iteration (OH recommendation).
        
        @return: return a long int which is the calculated summation of times (without idle), so it can be used for next iteration.
    */

    //opens file /proc/stat where the CPU time usages can be read
    FILE *f = fopen("/proc/stat", "r");
    char information[255];
    int column = 0;
    long int total_noIdle = 0;
    double percentage = 0;
    if(f != NULL){
            //gets the first line as documentation says these are the total times from all cpus
            if(fgets(information, 255, f) != NULL){
                //reserves space and splits at space
                char *split;
                split = strtok(information, " ");
                //while splittable read the information, only add up to steal 
                while(split != NULL && column < 9){
                    //if number and more improtantly not the number for idle (linux documentation) then add it to long int total_noIdle
                    //do not subtract iowait column, according to https://haydenjames.io/what-is-iowait-and-linux-performance/ wait and idle are different
                    if(checkIfStringIsNumber(split) && column != 4){
                            total_noIdle += strtol(split, NULL, 10);
                    }
                    column++;
                    split = strtok(NULL, " ");
                }
                //if not baseline iteration then proceed (avoids dividing by 0)
                if(i != -1){
                    //after total_noIdle is calculated from /proc/stat calculate percentage based on formula in office hours.
		    percentage = ((fabs((total_noIdle - previous)))/previous) * 100;
                    //function that returns the amount of configured processors in a system
                    printf("CPU cores: %d\n",  get_nprocs_conf());
                    printf("CPU usage : %.2f%%\n", percentage);
                    printf("---------------------------------------\n");
                }
            }
    }else{
        //if file doesn't open, print error message
        fprintf(stderr, "%s", "CPU info cannot be displayed");
        exit(1);
    }
    if(fclose(f)){
        //if file doesn't close, print error
        fprintf(stderr, "%s", "Failed closing file");
        exit(1);
    }
    //return total_noIdle so it can be used for the next iteration.
    return total_noIdle;
}

void printUser(){
    /*
        @brief: Prints the users and sessions currently logged into the system using utmp
    */
    printf("### Sessions/Users ### \n");
    //intialize a struct utmp pointer to be able to read utmp file
    struct utmp* users;
    //rewinds the file pointer to beginning of utmp file
    setutent();
    //reads a line from current position in utmp file. returns a pointer to a structure containing fields of the line hence we 
    //dont need to allcoate space!
    users = getutent();
    //while it reads an appropiate utmp file then continue
    while(users){
        //if type is user process (normal process), then print appropiate struct fields
        if(users->ut_type == USER_PROCESS){
                 printf(" %s       %s %s\n", users->ut_user, users->ut_line, users->ut_host);
        }
        users = getutent();
    }
    printf("---------------------------------------\n");
}
void printLogistics(){
    /*
        @brief: Prints the logistics of the system, inlcuding system name, machine name, version, release, and architecture
    */

    //intialize utsname struct to use uname
    struct utsname h;
    //uname sets the proper system information fields in parameter's address
    uname(&h);
    printf("### System Information ###\n");
    printf(" System Name = %s\n", h.sysname);
    printf(" Machine Name = %s\n", h.nodename);
    printf(" Version = %s\n", h.version);
    printf(" Release = %s\n", h.release);
    printf(" Architecture = %s\n", h.machine);
    printf("---------------------------------------\n");

}


int main(int argc, char **argv){
    /*
        @brief: Handles flags and based on flags turned on, call the appropiate functions to display the desired information
        @param: int argc: the number of arguments(flags) specified by user, including execution file
                char **argv: pointer to a pointer of character arrays (array of strings) that represents the flags specified by user
    */

    //default settings without any flags
    int samples = 10;
    int time = 1;
    int user = 1;
    int systemShow = 1;
    int sequential = 0;
    int numInts = 0;
    int setTime = 0;
    int setSamples = 0;
    //if there are more than 1 argument then there are flags, check what user inputted
    if(argc > 1){
        for(int i = 1; i < argc; i++){
            //if only wants to show user information then we turn off systemShow
            if(strcmp(argv[i], "--user") == 0){
                systemShow  = 0;
            //if only wants to show system infomration then we turn off user
            }else if(strcmp(argv[i], "--system") == 0){
                user = 0;
            //if sequential flag called, then we turn it on
            }else if(strcmp(argv[i], "--sequential") == 0){
                sequential = 1;
            //if the first 10 characters of the flag matches --samples= and then 11 isnt a null character. then check the characters after =
            }else if((strncmp(argv[i], "--samples=",10) == 0) && argv[i][10] != '\0'){
                char *traverse = argv[i];
                //jump to 10th index
                traverse = traverse + (10 * sizeof(char));
                //check if each character is a number
                if(checkIfStringIsNumber(traverse)){
                    //set samples
                    samples = atoi(traverse);
                }else{
                    //invalid flag, doesn't contain all numbers after =
                    fprintf(stderr, "%s", "Invalid flag\n");
                    exit(1);
                }
                //if this reaches, then valid and we can confirm --samples=X was a flag
                setSamples = 1;
            //same logic as samples, but with time
            }else if((strncmp(argv[i], "--tdelay=",9) == 0) && argv[i][9] != '\0'){
                char *traverse = argv[i];
                traverse = traverse + (9 * sizeof(char));
                if(checkIfStringIsNumber(traverse)){
                    time = atoi(traverse);
                }else{
                    fprintf(stderr, "%s", "Invalid flag\n");
                    exit(1);
                }
                //if this reaches, then valid and we can confirm --time=X was a flag
                setTime = 1;
            //checks if the first character of flag is a number, this is for the positional arguments
            }else if((isdigit(*argv[i])) && numInts == 0){
                char *traverse = argv[i];
                //checks if all characters are numbers, if all numbers, set to samples, otherwise error
                if(checkIfStringIsNumber(traverse)){
                    samples = atoi(traverse);
                }else{
                    fprintf(stderr, "%s", "Invalid flag\n");
                    exit(1);
                }
                //increment the number of integers and number of integers in sequence
                numInts++;
            //Also for positional argument. Since numInts default is 0, this won't run until the previous elseif implying at least 3 arguments, meaning you can check the previous.
            }else if(isdigit(*argv[i]) && isdigit(*argv[i-1]) && numInts == 1){
                char *traverse = argv[i];
                //checks if all characters are numbers, if all numbers, set to time, otherwise error
                if (checkIfStringIsNumber(traverse)){
                    time = atoi(traverse);
                }else{
                    fprintf(stderr, "%s", "Invalid flag\n");
                    exit(1);
                }
                //increments numInts and integerSequence
                numInts++;
            //any other flag is invalid
            }else{
                fprintf(stderr, "%s", "Invalid flag\n");
                exit(1);
            }


        } 
        //if only one positional argument, then invalid
        if(numInts == 1){
                fprintf(stderr, "%s", "Invalid flag\n");
                exit(1);
            }
        //if both positional arguments, and set time or sample, invalid
        if(numInts == 2 && (setSamples || setTime)){
                fprintf(stderr, "%s", "Invalid flag\n");
                exit(1);
        }
        //typing both --user and --system is equivalent to normal, so invalid
        if(systemShow == 0 && user == 0){
                fprintf(stderr, "%s", "Showing user and system can be called normally\n");
                exit(1);

        }
    }
    //allocate space array(2D) for system memory info to be stored
    double *arr = malloc(samples * 4 * (sizeof(double)));
    //clears screen
    printf("\e[H\e[2J\e[3J");
    //baseline calcuation for cpu Usage
    long int prev_CPU = cpuInfo(0, -1);
    //repeat printing process for a total of int samples.
    for(int i = 0; i < samples; i++){
        if(!sequential){
            //if not sequential then we can clear after each iteration, as well as print the normal header
            printf("\e[H\e[2J\e[3J");
            header(samples, time);
	    }else{ 
            //otherwise print sequential header, and dont clear
            sequentialHeader(i);
	    }
        //if user wants to see memory info and cpu info, print it
        if(systemShow){
            printSystem(i, samples, arr, sequential);
            prev_CPU = cpuInfo(prev_CPU, i);
	    }
        //if user wants to see user info print it
        if(user){
            printUser();
	    }
        //sleep for the interval before calculating next information
        sleep(time);
    }
    //print logistics last
    printLogistics();

    return 0;
}
