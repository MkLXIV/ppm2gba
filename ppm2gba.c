#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define GBA_MAX_CHANNEL_VAL 0x1F

char* const usageMessage =
    "ppm2gba v0.0.2 by MkLXIV\n\n"
    "Converts a PPM image to a C array of image data for use on the GBA.\n"
    "Usage: ppm2gba <input image> <output file> <arrayName>";

static inline uint8_t normalizeColorChannel(uint8_t channelValue) {
    return channelValue * GBA_MAX_CHANNEL_VAL / 0xFF;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        puts(usageMessage);
        return 1;
    }

    printf("Opening image %s...\n", argv[1]);

    FILE* input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("Error: couldn't open file %s\n", argv[1]);
        return 1;
    }

    char headerMagic[3] = {0, 0, 0};
    char* const headerResult = fgets(headerMagic, 3, input);
    if (strcmp("P6", headerResult) != 0) {
        puts("Error: Input failed PPM magic number check");
        fclose(input);
        return 1;
    }

    fseek(input, sizeof(char), SEEK_CUR);
    char resolutionBuffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    fgets(resolutionBuffer, 10, input);

    // Section off the width portion of the resolution
    int widthEndPos = 0;
    while (widthEndPos < 10) {
        if (resolutionBuffer[widthEndPos] == ' ') {
            resolutionBuffer[widthEndPos] = 0;
            break;
        }
        widthEndPos++;
    }

    // Properly null-terminate the height portion of the resolution
    for (int i = widthEndPos + 1; i < 10; i++) {
        if (resolutionBuffer[i] == 10) {
            resolutionBuffer[i] = 0;
            break;
        }
    }

    // Get integer values for the width and height
    int width = atoi(resolutionBuffer);
    int height = atoi(resolutionBuffer + widthEndPos + 1);

    char maxValBuffer[6] = {0, 0, 0, 0, 0, 0};
    fgets(maxValBuffer, 6, input);

    // Null-terminate the max value string
    for (int i = 0; i < 6; i++) {
        if (maxValBuffer[i] == 10) {
            maxValBuffer[i] = 0;
            break;
        }
    }

    int maxVal = atoi(maxValBuffer);
    int totalPixels = width * height;

    printf("Processing image %s (%dx%d @ %d max channel value)...\n", argv[1], width, height, maxVal);

    FILE* output = fopen(argv[2], "w");
    if (output == NULL) {
        printf("Error: Couldn't create destination file %s\n", argv[2]);
        fclose(input);
        return 1;
    }

    // Write "upper" boilerplate
    fputs("#include <stdint.h>\n", output);
    fputs("\n", output);
    fprintf(output, "uint16_t const %s\[%d * %d] = {\n", argv[3], width, height);

    uint8_t readRgb[3] = {0, 0, 0};
    int maxRow = totalPixels / 8;
    for (int i = 0; i < maxRow; i++) {
        fputs("    ", output);
        for (int j = 0; j < 8; j++) {
            // If we're not at the beginning or end of a line,
            // add a space.
            if (j > 0 && j < 8) {
                fputs(" ", output);
            }

            // Normalize the source image's color data to
            // fit within the GBA's RGB555 pixel data
            fread(readRgb, sizeof(uint8_t), 3, input);
            uint8_t redNorm = normalizeColorChannel(readRgb[0]);
            uint8_t greenNorm = normalizeColorChannel(readRgb[1]);
            uint8_t blueNorm = normalizeColorChannel(readRgb[2]);
            uint16_t colorNorm = (redNorm) | (greenNorm << 5) | (blueNorm << 10);

            // Output the converted pixel data
            fprintf(output, "0x%04X,", colorNorm);
            if (feof(input)) {
                break;
            }
        }
        fputs("\n", output);

        // Warn on EOF before the end of image data
        if (feof(input)) {
            puts("Warning: reached end of file before end of image data!");
            break;
        }
    }

    // Write the ending brace
    fputs("};", output);

    puts("All done!");
    fclose(input);
    fclose(output);

    return 0;
}
