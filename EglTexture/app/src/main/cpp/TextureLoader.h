#ifndef EGLTEXTURE_TEXTURELOADER_H
#define EGLTEXTURE_TEXTURELOADER_H

#include <GLES/gl.h>

/// returns the Texture ID. 0 if failed
GLuint LoadTextureFromFileBmp(const char *path);

/// load texture from a buffer, RBG888
GLuint LoadTextureBufferRgb888(const uint8_t *data,
                               size_t width,
                               size_t height);

/// load texture from a buffer, RBGA8888
GLuint LoadTextureBufferRgba8888(const uint8_t *data,
                                 size_t width,
                                 size_t height);

#endif //EGLTEXTURE_TEXTURELOADER_H
