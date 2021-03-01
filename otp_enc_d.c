/*Name: Jason Watson
 * Date: 6/8/2019
 * CS 344 Program 4 OTP
 * */

/*This program will run in background as a daemon.  Upon execution, otp_enc_d must output an error if it cannot run due to a network error.
 * Its function is to perform the actual encoding.  This program will listen on a particular port/socket, assigned when it is first ran. 
 * When a connection is made, otp_enc_d must all accept() to generate the socket used for actual communication, and then use a separate process to handle
 * the rest of the transaction, which will occur in the newly accepted socket*/
/*must support up to five concurrent connections running at the same time*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>



/*Use the same global background pid variables as from smallsh.c*/

int backgroundPidNumber;
pid_t backgroundPidHolder[5];

/*Use the same method for terminating as from smallsh.c*/
void killServer(int sig);
void error(const char *msg);
/*create function to encrypt data*/
char *toEncrypt(char *key, char *text);

int main(int argc, char *argv[])
{

    /*The very first thing I need to do, and this is similar to what I did in Smallsh.c, is initialize the global pid holders*/

    int a;
    backgroundPidNumber = -1;
    for (a = 0; a < 5; a++)
    {
	backgroundPidHolder[a] = -1;
    }

    /*First, I will need to call signal handler and pass in my killServer function.*/
    /*I used something similar to this in smallsh.c*/
    signal(SIGINT, killServer);

    /*I will need buffers for my text and for my key.  I think giving them a size of 256000 is good enough for what is required for the assignment*/
    char bufferText[256000];
    char bufferKey[256000];
    /*You will also need a buffer for the actual encrypted file.*/
    char *encryptBuffer;
    /*I'm using the source code from Brewster to prepare the socket.  These are from the server.c example*/
    char clientType, charResponse;
    /*add a means to store the file length*/
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead, fileLength;
    socklen_t sizeofClientInfo;
    char buffer[256];
    struct sockaddr_in serverAddress, clientAddress;

    /*I need to check the # of arguments.  Exit if not enough is given, i.e. below 2 arguments*/
    if (argc < 2)
    {
        fprintf(stderr, "Enter the correct number of arguments\n");
        exit(1);
    }
    /*More code from the server.c example*/
    /*This is to set up the address structure for the server process*/
    /*First, clear out the address structure*/
    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    /*Get the port number from the argument, and convert it from string to integer*/
    portNumber = atoi(argv[1]);
    /*Create a network capable socket*/
    serverAddress.sin_family = AF_INET;
    /*Store the port number in correct format*/
    serverAddress.sin_port = htons(portNumber);
    /*Any address is allowed for connection to this process*/
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    /*set up the socket*/
    /*create the socket*/
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    /*check for errors.  Use Brewster's error() function from the example*/
    if (listenSocketFD < 0)
    {
        error("Error opening socket");
    }
    /*Enable the socket to begin listening.  Check for errors.  Use the example code*/
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("Error on binding");
    }
    /*flip the socket on -- it can now receive up to 5 connections*/
    listen(listenSocketFD, 5);
    /*get the size of the address for the client that will connect*/
    sizeofClientInfo = sizeof(clientAddress);

    /*Use a while loop to keep the server listening*/
    while (true)
    {
        /*Accept the connection*/
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeofClientInfo);
	/*check for errors.  Use Brewster's error() function here*/
        if (establishedConnectionFD < 0)
        {
            error("Error on accept");
        }
	/*create the fork() with the new process id*/
	/*Use the same setup from Smallsh.c and from the lecture on processes*/
        pid_t spawnPid = fork();
	/*switch statement here*/
        switch(spawnPid)
        {
	    /*First, if something goes wrong...*/
            case -1:
		/*be sure to write to stderr*/
                fprintf(stderr, "Child fork error.");
                exit(1);
            case 0:
		/*Now, the child process stuff...*/
                /*check the client for the 'E' encryption type.*/
		/*receive the data*/
                recv(establishedConnectionFD, &clientType, sizeof(char), 0);
		/*If the client should receive something incorrect, then, you will need to initialize charResponse to N for no, and then send response back to server*/
                if (clientType != 'E')
                {
                    charResponse = 'N';
                    send(establishedConnectionFD, &charResponse, sizeof(char), 0);
                    fprintf(stderr, "Invalid client connection.\n");
		    /*You will need to exit?*/
                    exit(1);
                }
                else
                {
		    /*Otherwise, send back an affirmitive.  Use Y for yes*/
                    charResponse = 'Y';
                    send(establishedConnectionFD, &charResponse, sizeof(char), 0);
                }
		/*begin to receive the data here.  First, receive the file length*/
                recv(establishedConnectionFD, &fileLength, sizeof(fileLength), 0);
		/*clear the key and text buffers*/
                memset(bufferKey, '\0', sizeof(bufferKey));
                memset(bufferText, '\0', sizeof(bufferText));

		/*receive the data and store in the two buffers*/
                recv(establishedConnectionFD, bufferKey, fileLength * sizeof(char), 0);
                recv(establishedConnectionFD, bufferText, fileLength * sizeof(char), 0);
		/*encrypt the buffers.*/
                encryptBuffer = toEncrypt(bufferKey, bufferText);

		/*send the encryption to the client*/
                send(establishedConnectionFD, encryptBuffer, fileLength * sizeof(char), 0);
		/*close the existing socket which is connected to the client*/
		close(establishedConnectionFD);
		/*close the listening socket*/
		close(listenSocketFD);

                exit(0);

            default:
		/*stuff for the parent to do...*/
		/*All that the parent really needs to do is add the process id to the backgroundPidHolder array.*/
                backgroundPidHolder[++(backgroundPidNumber)] = spawnPid;

        }
    }





    return 0;
}

/*killServer() function: This function does what it says.  It kills the server.  I did something similar in Smallsh.c
 * I will pass this function into the signal handler at the beginning of main() to catch the SIGINT.*/
void killServer(int sig)
{
    int a;
    for (a = 0; a < backgroundPidNumber + 1; a++)
    {
	/*kill each of the 5 background processes*/
        kill(backgroundPidHolder[a], SIGINT);
    }
}

/*Brewster's error function from the example*/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*Here, it is important to manipulate ASCII integers*/
char *toEncrypt(char* key, char* text)
{
    int a;
    /*I will need to store the length of the text buffer*/
    int length = strlen(text);
    /*I will also need some temporary buffers for the key and text*/
    int temporaryKey;
    int temporaryText;
    /*Create a holder for the encrypted data*/
    char encryptionHolder[256000];
    /*initialize the encryptionHolder with '\0'*/
    memset(encryptionHolder, '\0', sizeof(encryptionHolder));

    /*Build a for loop here that will step through the text and encrypt the data*/
    for (a = 0; a < length; a++)
    {
	/*If there is a blan space in the text, then store a '@' in place of the blank text*/
        if (text[a] == ' ')
        {
            encryptionHolder[a] = '@';
        }
        else
        {
	    /*store key element into temporarykey.  Cast to integer type*/
            temporaryKey = (int)key[a];
	    /*store text element into temporarytext*/
            temporaryText = (int)text[a];
	    /*combine key and message using modular addition*/
            encryptionHolder[a] = (char)(temporaryText + (temporaryKey % 3));
        }

    }
    /*return the encrypted string back to the program.  Here, I need to research the best way to do this*/
    /*strdup() returns a pointer to a null-terminated byte string, a copy of the original.  This seems like the best option to return string*/
    /*www.tek-tips.com/viewthread.cfm?qid=1471051*/
    return strdup(encryptionHolder);
}
