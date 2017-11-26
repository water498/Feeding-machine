#include "feed.h"

void printHelp()
{
    printf(YELLOW"****************************************************************************\n");
    printf("*                                   HELP                                   *\n");
    printf("****************************************************************************\n");
    printf("* --cn #arg: number of cats (Default 6)                                    *\n");
    printf("* --dn #arg: number of dogs (Default 4)                                    *\n");
    printf("* --mn #arg: number of mice (Default 2)                                    *\n");
    printf("* --ct #arg: time a cat is satisfied (Default 15)                          *\n");
    printf("* --dt #arg: time a dog is satisfied (Default 10)                          *\n");
    printf("* --mt #arg: time a mouse is satisfied  (Default 1)                        *\n");
    printf("* --ce #arg: how many times a cat wants to eat (Default 5)                 *\n");
    printf("* --de #arg: how many times a dog wants to eat (Default 5)                 *\n");
    printf("* --me #arg: how many times a mouse wants to eat (Default 5)               *\n");
    printf("* --dish #arg: number of Food Dishes (Default 2)                           *\n");
    printf("* --e #arg: eating time interval lower boundary (Default 1)                *\n");
    printf("* --E #arg: eating time interval upper boundary (Default 1)                *\n");
    printf("*                                                                          *\n");
    printf("* --file #arg: file to flush data into.                                    *\n");
    printf("* --v Verbose print Statements                                             *\n");
    printf("* --h Help output                                                          *\n");
    printf("****************************************************************************\n"RESET);
    exit(0);
}

