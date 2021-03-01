/*Name: Jason Watson
 * Date: 6/9/2019
 * CS 344 Program 4 OTP keygen.c file
 * */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


/*This simple file creates a key file of a specified length.  The characters in the file generated will
 * be any of 27 characters.*/
int main(int argc, char* argv[])
{
    /*Make sure a key length has been entered by the user*/
    /*be sure to print to stderr*/
    if (argc == 1)
    {
        fprintf(stderr, "Enter a key length\n");
        exit(1);
    }

    int a;
    /*I will need a vessel to store the final encryption key*/
    /*Use a standard char* array*/
    char *encryptionKey;
    /*make the key length be what was entered in argument, plus 1 for end of line*/
    int keyLength = atoi(argv[1]) + 1;
    /*I will need some memory from the heap for the encryptionKey*/
    encryptionKey = malloc(sizeof(char) * keyLength);

    /*Because I'm going to just use rand(), I will need a random seed*/
    srand(time(NULL));
    /*Store the length of the key into the keyLength variable*/

    for (a = 0; a < keyLength; a++)
    {
	/*add random spaces to the key*/	
        if ( (rand() % (10) + 1) == 3 || (rand() % (10) + 1) == 6)
        {
            encryptionKey[a] = ' ';
        }
        else
        {
	    /*add random ASCII to encryption key*/
            encryptionKey[a] = (rand() % (90 + 1 - 65) + 65);
        }

    }
    /*You will need to add an end of line to encryptionKey to make it a string*/
    encryptionKey[a] = '\0';
    /*print out the string*/
    printf("%s", encryptionKey);
    /*free the data from the heap*/
    free(encryptionKey);

    return 0;
}

