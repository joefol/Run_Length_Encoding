# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <fcntl.h>
# include <string.h>
# include <stdbool.h>

int main (int argc, char* argv[]) {
    
    // check for not enough/too many command line arguments
    if (argc > 5) {
        printf("Error: Too many command line arguments! Expected: rle <input file> <output file> <compression length> <mode>\n");
        exit(-1);
    } else if (argc < 5) {
        printf("Error: Not enough command line arguments! Expected: rle <input file> <output file> <compression length> <mode>\n");
        exit(-1);
    }

    char* inputFile = argv[1]; // just creating new pointer with new name (not new data)
	char* outputFile = argv[2];
    int compressionLength = atoi(argv[3]);
    int mode = atoi(argv[4]);

    // check for compression length less than 1
    if (compressionLength < 1) { 
        printf("Compression length cannot be less than 1\n");
        exit(-1);
    }
    
    // opening file 
    // open() system call
    int inFD = open(inputFile, O_RDONLY); // inFD is file descriptor(fd)
    if (inFD == -1) {
        perror("Could not open input file");
        exit(-1);
    }

    int bufferSize = 1024;
    char buffer[bufferSize]; // 1024 bytes of free space to prevent from overwriting - buffer = common term for a place to store stuff

    int outFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outFD == -1) {
        perror("Could not open output file");
        exit(-1);
    }

    // check what the mode is (0 for compression, 1 for decompression, error message neither)
    if (mode == 0) {

        // initializing buffers to store patterns for comparison
        char originalPattern[bufferSize];
        char newPattern[bufferSize];

        int bytesRead = read(inFD, originalPattern, compressionLength); 
        originalPattern[bytesRead] = '\0';
        if (bytesRead == -1) {
            perror("Could not read from input file");
            exit(-1);
        }

        int patternRepCount = 1;

        while (bytesRead > 0) { // while not at end of input file
            bytesRead = read(inFD, newPattern, compressionLength); // get pattern from next K bytes
            
            int comparisonResult = strncmp(originalPattern, newPattern, compressionLength);
            if (comparisonResult == 0 && patternRepCount < 255) {
                patternRepCount++;
            } else {
                
                unsigned char counter = patternRepCount;
                int writeRepResult = write( outFD, &counter, 1 );
                if( writeRepResult == -1 ){
                    perror("Could not write binary data");
                    exit(-1);
                }

                int writePatternResult = write(outFD, originalPattern, compressionLength);
                if (writePatternResult == -1) {
                    perror("Could not write to output file");
                    exit(-1);
                }
                
                strncpy (originalPattern, newPattern, compressionLength); 
                patternRepCount = 1;
            }
        }
        
        unsigned char counter = 1;
        int writeRepResult = write( outFD, &counter, 1);
        if( writeRepResult == -1 ){
            perror("Could not write binary data");
            exit(-1);
        }
        
        int writePatternResult = write(outFD, originalPattern, 1);
        if (writePatternResult == -1) {
            perror("Could not write to output file");
            exit(-1);
        }
        
    } else if (mode == 1) {
        
        int bytesRead = 1; // initializing bytesRead to start loop
        unsigned char charReps;
        char pattern[compressionLength];

        while (bytesRead > 0) { // while not at end of file
            bytesRead = read(inFD, &charReps, 1); // binary data being read into charReps
            if (bytesRead == 0) {
                break;
            }
            
            bytesRead = read(inFD, pattern, compressionLength);
            
            int writeResult;
            for (int i = 0; i < charReps; i++) {
                writeResult = write(outFD, pattern, bytesRead);
            }
        }
        
    } else {
        char buffer[] = "Mode cannot be anything other than 0 or 1\n";
        write(STDOUT_FILENO, buffer, 42);
        exit(-1);
    } 

}