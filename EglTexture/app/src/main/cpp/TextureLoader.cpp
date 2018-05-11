#include "Debug.h"
#include <cstdio>
#include <cstdlib>
#include <string>

#include <GLES/gl.h>

#define LOG_TAG "TEXTURE_LOADER"
#define BMP_HEADER_SIZE 54


static GLuint LoadTextureBuffer(
        const uint8_t *data,
        size_t width,
        size_t height,
        GLuint dataFormat,
        GLuint dataType) {

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID); // generate 1 texture name
    LOGI("generated texture: %u", textureID);

    // "Bind" the newly created texture to the target:
    // all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    // will load image to GL_TEXTURE_2D, which is bound to textureID
    glTexImage2D(GL_TEXTURE_2D, // target
                 0, // level of the mipmap
                 GL_RGBA, // internal format
                 width, // width
                 height, // height
                 0, // border. MUST BE 0
                 dataFormat,
                 dataType,
                 data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /*
    // bilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    */
    /*
   // enable trilinear filtering ...
   // FIXME: not available in gles1
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   // ... which requires mipmaps. Generate them automatically.
   glGenerateMipmap(GL_TEXTURE_2D);
   */

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // Return the ID of the texture we just created
    return textureID;
}

GLuint LoadTextureFileBmp(const char *imagepath) {

    LOGI("Reading image %s", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[BMP_HEADER_SIZE];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char *data;

    // Open the file
    FILE *file = fopen(imagepath, "rb");
    if (!file) {
        LOGE("Failed to open %s", imagepath);
        return 0;
    }

    // Read the header, i.e. the BMP_HEADER_SIZE first bytes

    // If less than BMP_HEADER_SIZE bytes are read, problem
    if (fread(header, 1, BMP_HEADER_SIZE, file) != BMP_HEADER_SIZE) {
        LOGE("%s is not a valid BMP file", imagepath);
        fclose(file);
        return 0;
    }
    // A BMP files always begins with "BM"
    if ((header[0] != 'B') || (header[1] != 'M')) {
        LOGE("%s is not a valid BMP file", imagepath);
        fclose(file);
        return 0;
    }
    // Make sure this is a 24bpp file
    if (*(int *) &(header[0x1E]) != 0) {
        LOGE("%s is not a valid BMP file", imagepath);
        fclose(file);
        return 0;
    }
    if (*(int *) &(header[0x1C]) != 24) {
        LOGE("%s is not a valid BMP file", imagepath);
        fclose(file);
        return 0;
    }

    // Read the information about the image
    dataPos = *(int *) &(header[0x0A]);
    imageSize = *(int *) &(header[0x22]);
    width = *(int *) &(header[0x12]);
    height = *(int *) &(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0) {
        LOGI("deducing image size");
        imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
    }
    if (dataPos == 0) {
        LOGI("deducing data start position");
        dataPos = BMP_HEADER_SIZE;  // The BMP header is done that way
    }
    LOGI("dataPos=%u, imageSize=%u, width=%u, height=%u", dataPos, imageSize, width, height);

    // Create a buffer
    data = new unsigned char[imageSize];

    // Read the actual data from the file into the buffer
    fseek(file, dataPos, SEEK_SET);
    fread(data, 1, imageSize, file);

    // Convert BGR to RGB. for 24-bit bitmaps, the ordering is BGR.
    for (unsigned int idx = 0; idx < imageSize; idx += 3) {
        // BGR -> RGB: swap R and B
        std::swap(data[idx], data[idx+2]);
    }

    // Everything is in memory now, the file can be closed.
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID = LoadTextureBuffer(data, width, height, GL_RGB, GL_UNSIGNED_BYTE);

    // OpenGL has now copied the data. Free our own version
    delete[] data;

    return textureID;
}

/// load texture from a buffer, RBG888
GLuint LoadTextureBufferRgb888(const uint8_t *data,
                               size_t width,
                               size_t height) {
    return LoadTextureBuffer(data, width, height, GL_RGB, GL_UNSIGNED_BYTE);
}

/// load texture from a buffer, RBGA8888
GLuint LoadTextureBufferRgba8888(const uint8_t *data,
                                 size_t width,
                                 size_t height) {
    return LoadTextureBuffer(data, width, height, GL_RGBA, GL_UNSIGNED_BYTE);
}