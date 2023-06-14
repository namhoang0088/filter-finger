#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#pragma pack(push, 1)
typedef struct {
    unsigned char signature[2];
    unsigned int fileSize;
    unsigned int reserved;
    unsigned int dataOffset;
    unsigned int headerSize;
    unsigned int width;
    unsigned int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    unsigned int xResolution;
    unsigned int yResolution;
    unsigned int numColors;
    unsigned int importantColors;
} BMPHeader;
#pragma pack(pop)

float normalize_pixel(float x, float v0, float v, float m, float m0) {
    float dev_coeff = sqrt((v0 * pow((x - m), 2)) / v);
    return (x > m) ? m0 + dev_coeff : m0 - dev_coeff;
}

void normalize_image(unsigned char *image, int width, int height, float m0, float v0) {
    float m = 0.0;
    float v = 0.0;

    for (int i = 0; i < width * height; i++) {
        float gray = (image[i * 3] + image[i * 3 + 1] + image[i * 3 + 2]) / 3.0;
        m += gray;
        v += pow(gray, 2);
    }

    m /= (width * height);
    v = v / (width * height) - pow(m, 2);

    for (int i = 0; i < width * height; i++) {
        float gray = (image[i * 3] + image[i * 3 + 1] + image[i * 3 + 2]) / 3.0;
        float normalized_gray = normalize_pixel(gray, v0, v, m, m0);
        image[i * 3] = (unsigned char)normalized_gray;
        image[i * 3 + 1] = (unsigned char)normalized_gray;
        image[i * 3 + 2] = (unsigned char)normalized_gray;
    }
}

int main() {
    FILE *file = fopen("101_1.bmp", "rb");
    if (file == NULL) {
        printf("Failed to open the image file.\n");
        return 1;
    }

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, file);

    int width = header.width;
    int height = header.height;

    unsigned char *image = (unsigned char *)malloc(width * height * 3);
    fread(image, width * height * 3, 1, file);

    fclose(file);

    float m0 = 150.0;
    float v0 = 500.0;

    normalize_image(image, width, height, m0, v0);

    file = fopen("u000_fp001_000.bmp", "wb");
    if (file == NULL) {
        printf("Failed to create the normalized image file.\n");
        free(image);
        return 1;
    }

    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(image, width * height * 3, 1, file);

    fclose(file);
    free(image);

    return 0;
}