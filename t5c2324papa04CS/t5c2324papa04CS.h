/*
nome file		:t5c2324papa04CS.h
nome soluzione	:t5c2324papa04CS.sln
autore			:papa cesare
data			:3/11/2023
scopo			:primi passi con libreria winsock
note tecniche	:

analisi problema/strategie risolutive:

pseudocodice	:

*/

#define DEFAULT_PORT "40001"
#define DEFAULT_BUFLEN_SMALL 256
#define DEFAULT_BUFLEN_NORMAL 1024

#define CHIAVE_CONNESSIONE "PAPAX27OIUM89SUAOSADPOI"
#define PRESENTAZIONE_SERVER "PAPASERVERCVJAOSdiA10239SS"

#define CHAR_SIZE sizeof(CHAR)        // Size of char in bytes
#define CHAR_BITS ((CHAR_SIZE * 8) - 1)   // Size of char in bits - 1

#define ENCRYPTION_KEY "03418561165"
#define ENCRYPTION_KEY_LENGHT 11

#define NMAX_THREADS 32

char rotateLeft(char num, unsigned char rotation)
{
    char DROPPED_MSB;

    // The effective rotation
    rotation %= CHAR_BITS;

    // Loop till rotation becomes 0
    while (rotation--)
    {
        // Get MSB of num before it gets dropped
        DROPPED_MSB = (num >> CHAR_BITS) & 1;

        // Left rotate num by 1 and 
        // Set its dropped MSB as new LSB
        num = (num << 1) | DROPPED_MSB;
    }

    return num;
}

/**
 * Function to rotate bits of a number to right.
 *
 * @num         Number to rotate.
 * @rotation    Number of times to rotate right.
 */
char rotateRight(char num, unsigned char rotation)
{
    char DROPPED_LSB;

    // The effective rotation
    rotation %= CHAR_BITS;


    // Loop till rotation becomes 0
    while (rotation--)
    {
        // Get LSB of num before it gets dropped
        DROPPED_LSB = num & 1;

        // Right shift num by 1 and 
        // Clear its MSB
        num = (num >> 1) & (~(1 << CHAR_BITS));

        // Set its dropped LSB as new MSB
        num = num | (DROPPED_LSB << CHAR_BITS);
    }

    return num;

}

void encryption_algorithm(char* clearText) {
    int i, j;

    for (i = 0, j = 0; i < (int)sizeof(clearText) && clearText[i] != 0; i++, j++) { //scorro la chiave di encriptazione per sommare diversi numeri alle diverse lettere
        if (j == ENCRYPTION_KEY_LENGHT)
            j = 0;
        clearText[i] = rotateRight(clearText[i], ENCRYPTION_KEY[j]);
    }
}

void decryption_algorithm(char* clearText) {
    int i, j;

    for (i = 0, j = 0; i < (int)sizeof(clearText) && clearText[i] != 0; i++, j++) {   ////scorro la chiave di encriptazione per sottrarre diversi numeri alle diverse lettere
        if (j == ENCRYPTION_KEY_LENGHT)
            j = 0;
        clearText[i] = rotateLeft(clearText[i], ENCRYPTION_KEY[j]);
    }
}