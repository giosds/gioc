#include <stdio.h>
#define MAX_KEY_LENGTH 13
#define BYTE_LENGTH 256
#define A_CHAR 97
#include <math.h>

/*  This code breaks a Vigenere Cipher.
    The first step determines the length of the key.
    The second step finds the key values.

    This code was not meant to be particularly efficient :).*/

double find_length(int n, FILE *fpIn);
float do_stream(int i, int stride, unsigned char key, FILE *fpIn);
void decipher(unsigned char key[], int key_length, FILE *fpIn);

//  Frequencies of letters in english language, in alphabetical order
double eng_frequencies[] = {.082, .015, .028, .043, .127, .022, .002, .061, .07,
                    .002, .008, .004, .024, .067, .015, .019, .001, .06,
                    .063, .091, .028, .01, .024, .002, .02, .001};
long int txt_length = 0;

void main()
{
    FILE  *fpIn;
    int stride = 0; // Length of the key
    unsigned char keys[MAX_KEY_LENGTH];// Array for storing up to 13 keys {186,31,145,178,83,205,62};
    unsigned char ch;

    // Read cipher text
    fpIn = fopen("../ctext.txt", "r");

    //GET THE CIPHER TEXT LENGTH
    while (fscanf(fpIn, "%c", &ch) != EOF)
        txt_length++;
    txt_length = txt_length/2;
    printf("TEXT LENGTH: %d\n",txt_length);

    /*
        STEP 1: DETERMINE THE KEY LENGTH
    */
    // Tries different lengths of the cypher text, 1-13
    double temp_max_freq = 0;
    double freq = 0;
    for(int i = 1; i < 14; i++)
    {
        // Looks for the maximum sum of squared frequencies for different key lengths.
        // The highest one should be the not-random one
        freq = find_length(i, fpIn);
        if (freq>temp_max_freq)
        {
            temp_max_freq = freq;
            stride = i;
        }
        printf("Best key length: %d\n\n\n", stride);
    }    // LENGTH IS 7

    /*
        STEP 2: FIND A KEY FOR EACH TEXT STREAM
    */
    // Manage each stream
    int h = 0;
    float temp_ratio = 0;
    float max_ratio = 0;
    for(h = 0; h < stride; h++) // For each text stream
    {
        max_ratio = 0;
        for (unsigned char key=0; key<BYTE_LENGTH-1; key++)   //Test a different byte for the xor operation
        {
            temp_ratio = do_stream(h, stride, key, fpIn);
            if (temp_ratio>max_ratio)
            {
                max_ratio = temp_ratio;
                keys[h] = key;
            }
        }
        printf("\nKEY: %d\n", keys[h]);
    }

    // DECRYPT AND PRINT AS PLAIN TEXT
    decipher(keys,stride, fpIn);
    fclose(fpIn);
    return;
}

void decipher(unsigned char keys[], int key_length, FILE *fpIn)
{
    /*  Prints frequencies. Look where the highest ones are */
    unsigned char ch[4]= {0,0};
    int nFound = 0;
    // Every n character, increment its byte counter
    rewind(fpIn);
    for (int i=0; i<txt_length; i++)
    {
        fscanf(fpIn, "%02X", &ch);
        printf("%c", ch[0] ^ keys[i % (key_length)],i); // Add, mod 2 cypher
    };
}

float do_stream(int n, int stride, unsigned char key, FILE *fpIn)
{
    unsigned char ch[4];
    double hifreq = 0;
    unsigned int n_lowercase = 0;
    float ratio;
    int i = 0;

    // Discard sequences where at least one
    // is out of the ascii enghlish text boundaries
    rewind(fpIn);
    while (fscanf(fpIn, "%02X", &ch) != EOF)
    {
        ch[0] = ch[0] ^ key;
        if ((i%stride == n) && (ch[0]<=31 || ch[0]>=128))
            return 0;
        i++;
    }

    printf("FOUND");
    printf("\nstream %d, stride %d, key %d\n", n, stride, key);

    i=0;
    rewind(fpIn);
    while (fscanf(fpIn, "%02X", &ch) != EOF)
    {
        ch[0] = ch[0] ^ key;    // Decrypt

        // If the character is a lowercase letter or a space, increment and print
        if ((i%stride == n) &&((ch[0]>96 && ch[0]<123)||(ch[0]==32)))
        {
            printf("%c", ch[0]);
            n_lowercase++;
        }

        //Assign proper weight to each character, according to frequency tables
        hifreq+= eng_frequencies[ch[0]-A_CHAR];
        i++;
    }

    // Discard the key, if too few of the characters are lowercase letters
    if (n_lowercase<(txt_length/(stride+1)))
    {
        printf("n_lower:%d\n", n_lowercase);
        return 0;
    }

    // Find relative frequencies
    hifreq = hifreq/i;//n_lowercase;
    printf("\nhigh: %f; low: %d ", hifreq, i);
    return hifreq;
}

double find_length(int n, FILE *fpIn)
{
    /*  Prints frequencies. Look where the highest ones are */
    unsigned int ch[2]; // Allow more space to prevent overflow
    unsigned char all_ascii[BYTE_LENGTH] = {0};
    double all_freqs[BYTE_LENGTH] = {0};
    double all_f_add=0;
    int nFound = 0;
    double midvar=0;
    int i=0;

    // Every n characters, increment its byte counter
    rewind(fpIn);
    while (fscanf(fpIn, "%02X", &ch) != EOF)
    {
        if (i%n == 0)
        {
            all_ascii[ch[0]] +=1;
            nFound ++;
        }
        i++;
    }

    // Compute relative frequency and square it, them add all together
    for(int l = 0; l < BYTE_LENGTH; l++)
    {
        midvar = ((double)all_ascii[l])/nFound;
        all_f_add += pow(midvar,2);
    }
    // Print frequency
    printf(">%d: %f ", n, all_f_add);

    return all_f_add;
}
