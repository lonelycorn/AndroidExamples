#include "Drawable.h"
#include "Debug.h"
#include "TextureLoader.h"
#include "lodepng/lodepng.h"
#include <cstdlib>
#include <cassert>

#define LOG_TAG "Drawable"

static GLuint LoadPngFromAsset(AAssetManager *manager, const char *imageFilename) {

    const unsigned int bitDepth = 8;

    GLuint texture_id = 0;
    AAsset *asset = AAssetManager_open(manager, imageFilename, AASSET_MODE_BUFFER);
    const void *assetBuffer = AAsset_getBuffer(asset);
    off_t assetLength = AAsset_getLength(asset);
    uint8_t *out = nullptr;
    unsigned int w = 0;
    unsigned int h = 0;
    if (lodepng_decode_memory(
            &out,
            &w, &h,
            reinterpret_cast<const uint8_t *>(assetBuffer),
            static_cast<size_t>(assetLength),
            LodePNGColorType::LCT_RGBA,
            bitDepth)) {
        LOGE("failed to load asset/tsukuba.png");
        return texture_id;
    }

    // load to OpenGL
    texture_id = LoadTextureBufferRgba8888(out, w, h);

    // release resource
    free(out);
    AAsset_close(asset);
    asset = nullptr;

    return texture_id;
}

TexturedPlane::TexturedPlane(AAssetManager *manager):
    m_texture_id(0) {
    LoadModel(manager);
}

TexturedPlane::~TexturedPlane() {
    if (Initialized()) {
        glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
    }
}

bool TexturedPlane::Initialized() const {
    return (m_texture_id != 0);
}

bool TexturedPlane::Draw() {
    if (!Initialized()) {
        LOGE("TexturedPlane not initialized");
        return false;
    }

    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &(m_vertices[0].XYZ));

    // enable 2D texture
    glEnable(GL_TEXTURE_2D);
    // enable texture state
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    // point to buffer
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &(m_vertices[0].ST));
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // draw stuff
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &(m_triangles[0].indices));

    // disable 2D texture
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_CULL_FACE);

    return true;
}

void TexturedPlane::LoadModel(AAssetManager *manager) {

    // XYZ, ST
    m_vertices[0] = {-0.5f,  -0.5f,  0.0f,   0.0f,  1.0f};
    m_vertices[1] = {-0.5f,   0.5f,  0.0f,   0.0f,  0.0f};
    m_vertices[2] = { 0.5f,   0.5f,  0.0f,   1.0f,  0.0f};
    m_vertices[3] = { 0.5f,  -0.5f,  0.0f,   1.0f,  1.0f};

    // two triangles
    m_triangles[0] = { 0, 2, 1};
    m_triangles[1] = { 2, 0, 3};


    // load PNG texture
    const char *imageFilename = "tsukuba.png";
    m_texture_id = LoadPngFromAsset(manager, imageFilename);
}

Text::Text(AAssetManager *manager):
        m_texture_id(0) {
    LoadModel(manager);
}

Text::~Text() {
    if (Initialized()) {
        glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
    }
}

bool Text::Initialized() const {
    return (m_texture_id != 0);
}

bool Text::Draw() {
    if (!Initialized()) {
        LOGE("Text not initialized");
        return false;
    }

    GenerateTriangles("47Fc");
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &(m_vertices[0].XYZ));

    // enable 2D texture
    glEnable(GL_TEXTURE_2D);
    // enable texture state
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    // point to buffer
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &(m_vertices[0].ST));
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // draw stuff
    glDrawElements(GL_TRIANGLES, 3*m_triangles.size(), GL_UNSIGNED_SHORT, &(m_triangles[0].indices));

    // disable 2D texture
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_CULL_FACE);
    return true;
}

void Text::LoadModel(AAssetManager *manager) {
    // NOTE: m_vertices and m_triangles are generated on-the-fly

    // load PNG texture
    const char *imageFilename = "hex-digits.png";
    m_texture_id = LoadPngFromAsset(manager, imageFilename);
}

void Text::GenerateTriangles(const std::string &s) {
    assert(Initialized());

    // if text never changed
    if (s == m_string) {
        return;
    }

    m_string = s;

    m_vertices.clear();
    m_triangles.clear();
    m_vertices.reserve(m_string.size() * 4); // 4 corners for each quad
    m_triangles.reserve(m_string.size() * 2); // 2 triangles for each quad

    // texture constants
    const GLfloat pixelScale = 1.0f / 128.0f;
    const GLfloat letterTextureWidth = 21 * pixelScale;
    const GLfloat letterTextureHeight = 40 * pixelScale;

    // world display constants
    const GLfloat letterWorldWidth = 0.3f;
    const GLfloat letterWorldHeight = 0.4f;
    //const GLfloat letterWorldVerticalSpace = 0.02f;
    //const GLfloat letterWorldHorizontalSpace = 0.02f;

    // generate all characters in a row
    for (size_t i = 0; i < m_string.size(); ++i) {
        char c = m_string[i];
        int idx = -1;
        if ((c >= '0') && (c <= '9')) {
            idx = c - '0';
        }
        if ((c >= 'A') && (c <= 'F')) {
            idx = c - 'A' + 10;
        }
        if ((c >= 'a') && (c <= 'f')) {
            idx = c - 'a' + 10;
        }
        if (idx < 0) {
            // unknown characters
            continue;
        }
        int row = idx / 6;
        int col = idx % 6;

        // XYZ, ST
        Vertex minmin = {
                i*letterWorldWidth,
                0,
                0,
                col*letterTextureWidth,
                (row+1)*letterTextureHeight
        };
        Vertex minmax = {
                i*letterWorldWidth,
                letterWorldHeight,
                0,
                col*letterTextureWidth,
                row*letterTextureHeight
        };
        Vertex maxmax = {
                (i+1)*letterWorldWidth,
                letterWorldHeight,
                0,
                (col+1)*letterTextureWidth,
                row*letterTextureHeight
        };
        Vertex maxmin = {
                (i+1)*letterWorldWidth,
                0,
                0,
                (col+1)*letterTextureWidth,
                (row+1)*letterTextureHeight
        };

        unsigned short vertexCount = static_cast<unsigned short>(m_vertices.size());

        m_vertices.push_back(minmin);
        m_vertices.push_back(minmax);
        m_vertices.push_back(maxmax);
        m_vertices.push_back(maxmin);

        Triangle t1;
        t1.indices[0] = vertexCount + 0;
        t1.indices[1] = vertexCount + 2;
        t1.indices[2] = vertexCount + 1;
        m_triangles.push_back(t1);

        t1.indices[0] = vertexCount + 2;
        t1.indices[1] = vertexCount + 0;
        t1.indices[2] = vertexCount + 3;
        m_triangles.push_back(t1);
    }
    // transform points
    // NOTE: for now just do translation
    const GLfloat letterWorldX = -0.3f;
    const GLfloat letterWorldY = -0.4f;
    const GLfloat letterWorldZ = 0.2f;
    for (auto &v : m_vertices) {
        v.XYZ[0] += letterWorldX;
        v.XYZ[1] += letterWorldY;
        v.XYZ[2] += letterWorldZ;
    }
}