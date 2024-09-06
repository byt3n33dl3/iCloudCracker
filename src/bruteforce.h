#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#include <stdio.h>

#define MAX_SIZE 30         // only used to avoid errors
// do not use MAX_SIZE to constrain the algorithm to a certain password length. use the while loop in bruteforce.cpp
#define CHAR_COUNT 92       // size of the charset you want to use (number of possible chars for the password)

void getGuess();

#endif // BRUTEFORCE_H
