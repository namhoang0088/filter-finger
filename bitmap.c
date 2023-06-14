#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

#pragma pack(push, 1)

/*được sử dụng để đảm bảo rằng các cấu trúc BITMAPFILEHEADER, 
BITMAPINFOHEADER và RGBQUAD được đóng gói một cách chặt chẽ */

typedef struct {
    unsigned short bfType; // loại tệp
    unsigned int bfSize;   // toàn bộ kích thước tệp (byte)
    unsigned short bfReserved1; // 
    unsigned short bfReserved2;
    unsigned int bfOffBits; // vị trí bắt đầu của dữ liệu hình ảnh
} BITMAPFILEHEADER; // mô tả tiêu đề của tệp bitmap

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;// số lượng mặt phẳng của ảnh
    unsigned short biBitCount; // Số bit được sử dụng cho mỗi pixel của ảnh.
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER; 

typedef struct {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBQUAD; // biểu diễn một màu sắc trong không gian màu RGB (Red-Green-Blue)

#pragma pack(pop)

Bitmap* Bitmap_Load(const char* filePath) {
    FILE* file = fopen(filePath, "rb");
    if (file == NULL) {
        return NULL;
    }

    Bitmap* bitmap = (Bitmap*)malloc(sizeof(Bitmap));
    if (bitmap == NULL) {
        fclose(file);
        return NULL;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    if (fileHeader.bfType != 0x4D42 || infoHeader.biBitCount != 8) {
        fclose(file);
        free(bitmap);
        return NULL;
    }

    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;
    int imageSize = width * height;

    unsigned char* imageData = (unsigned char*)malloc(imageSize * sizeof(unsigned char));
    if (imageData == NULL) {
        fclose(file);
        free(bitmap);
        return NULL;
    }

    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    fread(imageData, sizeof(unsigned char), imageSize, file);

    fclose(file);

    bitmap->Width = width;
    bitmap->Height = height;
    bitmap->Data = imageData;

    return bitmap;
}

void Bitmap_Save(const char* filePath, Bitmap* bitmap) {
    if (bitmap == NULL || bitmap->Data == NULL) {
        return;
    }

    FILE* file = fopen(filePath, "wb");
    if (file == NULL) {
        return;
    }

    int width = bitmap->Width;
    int height = bitmap->Height;
    int imageSize = width * height;

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    memset(&fileHeader, 0, sizeof(BITMAPFILEHEADER));
    memset(&infoHeader, 0, sizeof(BITMAPINFOHEADER));

    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
    fileHeader.bfSize = fileHeader.bfOffBits + imageSize;
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 8;
    infoHeader.biSizeImage = imageSize;

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    // Color table
    for (int i = 0; i < 256; i++) {
        RGBQUAD color;
        color.rgbBlue = i;
        color.rgbGreen = i;
        color.rgbRed = i;
        color.rgbReserved = 0;
        fwrite(&color, sizeof(RGBQUAD), 1, file);
    }

    // Image data
    fwrite(bitmap->Data, sizeof(unsigned char), imageSize, file);

    fclose(file);
}

void Bitmap_Free(Bitmap* bitmap) {
    if (bitmap == NULL) {
        return;
    }

    if (bitmap->Data != NULL) {
        free(bitmap->Data);
    }

    free(bitmap);
}

#define PI 3.14159265

// Kích thước của bộ lọc 
#define KERNEL_SIZE 5

// Mảng hệ số 
double Kernel[KERNEL_SIZE][KERNEL_SIZE] = {
    {0.0004, 0.0033, 0.0067, 0.0033, 0.0004},
    {0.0033, 0.0280, 0.0571, 0.0280, 0.0033},
    {0.0067, 0.0571, 0.1098, 0.0571, 0.0067},
    {0.0033, 0.0280, 0.0571, 0.0280, 0.0033},
    {0.0004, 0.0033, 0.0067, 0.0033, 0.0004}
};

// Áp dụng bộ lọc cho ảnh vân tay
void applyFilter(unsigned char *imageData, int width, int height) {
    int i, j, x, y;
    int offset = KERNEL_SIZE / 2;
    double sum;

    // Tạo bản sao ảnh để lưu kết quả
    unsigned char *filteredImage = malloc(width * height * sizeof(unsigned char));

    // Duyệt qua từng pixel trong ảnh
    for (i = offset; i < height - offset; i++) {
        for (j = offset; j < width - offset; j++) {
            sum = 0.0;

            // Áp dụng bộ lọc tại vị trí (i, j)
            for (x = 0; x < KERNEL_SIZE; x++) {
                for (y = 0; y < KERNEL_SIZE; y++) {
                    sum += Kernel[x][y] * imageData[(i + x - offset) * width + (j + y - offset)];
                }
            }

            // Lưu kết quả vào bản sao ảnh
            filteredImage[i * width + j] = (unsigned char)sum;
        }
    }

    // Sao chép kết quả từ bản sao ảnh vào ảnh gốc
    for (i = 0; i < width * height; i++) {
        imageData[i] = filteredImage[i];
    }

    // Giải phóng bộ nhớ
    free(filteredImage);
}

int main() {
    // Đường dẫn đến ảnh vân tay
    const char* imagePath = "TEST_IMAGE_(3).bmp";

    // Đọc ảnh vân tay từ file BMP
    Bitmap* bitmap = Bitmap_Load(imagePath);
    if (bitmap == NULL) {
        printf("Khong the doc tep BMP.\n");
        return 0;
    }

    // Lấy thông tin chiều rộng và chiều cao của ảnh
    int width = bitmap->Width;
    int height = bitmap->Height;

    // Lấy dữ liệu pixel của ảnh
    unsigned char* imageData = bitmap->Data;

    // Áp dụng bộ lọc cho ảnh vân tay
    applyFilter(imageData, width, height);


    // Lưu ảnh đã được lọc vào tệp BMP
    const char* filteredImagePath = "filtered_fingerprint.bmp";
    Bitmap_Save(filteredImagePath, bitmap);

    // Giải phóng bộ nhớ
    Bitmap_Free(bitmap);

    return 0;
}