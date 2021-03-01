/*Name: Jason Watson
 * Date: 6/8/2019
 * CS 344 Program 4 OTP
 */
/*I basically copy and pasted my enc.c file here and made some modifications to that code*/
/*This program will connect to otp_dec_d and will ask it to decrypt ciphertext using a passed-in ciphertext and key, and otherwise performs exactly like otp_enc,
 * and must be runnable in exactly the same three ways.*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

/*You will need to keep track of various fd's and strings.*/
/*As from previous assignments, use global arrays and variables*/
char *keyName;
char *fileName;
int keyFileDescriptor;
int textFileDescriptor;
int fileLength;

/*Use the given error() function from Brewster's examples*/
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
char *readFileContents(char fileDescriptor);

int main(int argc, char* argv[])
{
    /*set up the client*/
    /*I'm using the source code from Brewster to prepare the client.  These are from the client.c example*/
    int socketFd, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[256];
    /*store the program name for later*/
    char *programName = "DEC_CLIENT";

    int a;
    char *fileContent = malloc(fileLength * sizeof(char*));
    int intBuffer;

    /*use stat to get information about key and text files*/
    struct stat key;
    struct stat text;

    /*Make sure about the correct number of arguments*/
    if (argc == 3)
    {
        fprintf(stderr, "Incorrect number of arguments.\n");
        exit(1);
    }

    /*Assign arguments to fileName and keyName*/
    fileName = argv[1];
    keyName = argv[2];

    /*open argument 1, and assign read only privileges*/
    textFileDescriptor = open(argv[1], O_RDONLY);
    /*check for errors from open() operation*/
    if (textFileDescriptor < 0)
    {
        fprintf(stderr, "couldn't open plaintext file.\n");
        exit(1);
    }
    /*open the key file and assign read only privileges*/
    keyFileDescriptor = open(argv[2], O_RDONLY);
    if (keyFileDescriptor < 0)
    {
        fprintf(stderr, "couldn't open keytext file.\n");
        exit(1);
    }

    if (stat(keyName, &key) < 0)
    {
        fprintf(stderr, "error getting stats of key file.\n");
        exit(1);
    }

    if (stat(fileName, &text) < 0)
    {
        fprintf(stderr, "error getting stats of text file.\n");
        exit(1);
    }
    /*use stat structure to compare sizes between key and text files*/
    /*use st_size here, from program 2*/
    if (key.st_size -1 < text.st_size -1)
    {
        fprintf(stderr, "key file is too short.");
        exit(1);
    }
    else
    {
        /*finally, assign the file length. Cast to integer*/
        fileLength = (int) (text.st_size - 1);
    }

    char serverAccept;
    char charfileLength[128];

    /*read the files and store contents into keyText and plainText*/
    char *keyText = readFileContents('k');
    char *receivedText = malloc(fileLength * sizeof(char*));
    char *plainText = readFileContents('t');

    /* set up server address as from the example server.c file */
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    portNumber = atoi(argv[3]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);

    /*use localhost*/
    serverHostInfo = gethostbyname("localhost");
    if (serverHostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: error. There is no such host\n");
        exit(1);
    }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

    /*set up socket, as from given server.c program*/
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
    {
        error("CLIENT: Error opening socket");
    }

    if (connect(socketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("CLIENT: Error connecting");
    }

    /*authenticate the server*/
    send(socketFd, &(programName[0]), sizeof(char), 0);
    recv(socketFd, &serverAccept, sizeof(char), 0);

    if (serverAccept != 'Y')
    {
	/*should not be able to connect to otp_enc_d.  Exit with return value of 2*/
        close(socketFd);
        fprintf(stderr, "Not a decryption server\n");
        exit(2);
    }

    /*send the data to the server*/
    send(socketFd, &(fileLength), sizeof(fileLength), 0);
    /*send keyText and plaintext*/
    send(socketFd, keyText, fileLength * sizeof(char), 0);
    send(socketFd, plainText, fileLength * sizeof(char), 0);

    /*receive data from the server*/
    recv(socketFd, receivedText, fileLength * sizeof(char), 0);
    printf("%s\n", receivedText);

    /*close out text and key files.  Reassign global text and file descriptors with garbage -1*/
    close(textFileDescriptor);
    close(keyFileDescriptor);
    textFileDescriptor = -1;
    keyFileDescriptor = -1;

    /*free up memory from heap*/
    free(keyText);
    free(plainText);
    free(receivedText);

}

/*readFileContents() function:  This function is mostly similar to the enc.c function, isFielValid().  This function, instead of checking the contents of the file,
 * will just read the file*/
char *readFileContents(char fileDescriptor)
{
    int a;
    int fd;
    /*create a string for the file contents.  This will need to be returned from the function*/
    char *fileContents = malloc(fileLength * sizeof(char*));
    /*temporary buffer for the file contents*/
    int buffer;
    /*I will need a way to distinguish between key files and text files*/
    if (fileDescriptor == 'k')
    {
        /*store global descriptors into fd*/
        fd = keyFileDescriptor;
    }
    else if (fileDescriptor == 't')
    {
        fd = textFileDescriptor;
    }
    /*if file opens, then great.  Otherwise print out an error*/
    if (read(fd, fileContents, fileLength) < 0)
    {
        fprintf(stderr, "Couldn't open plain text file\n");
        exit(1);
    }
    /*return correct file contents*/
    return fileContents;

}
