#ifndef BITMAP_H
#define BITMAP_H

typedef struct {
    int Width;
    int Height;
    unsigned char* Data;
} Bitmap;

Bitmap* Bitmap_Load(const char* filePath);
void Bitmap_Save(const char* filePath, Bitmap* bitmap);
void Bitmap_Free(Bitmap* bitmap);

#endif  /* BITMAP_H */