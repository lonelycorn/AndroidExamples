#ifndef EGLTEXTURE_DRAWABLE_H
#define EGLTEXTURE_DRAWABLE_H


#include <android/asset_manager.h>
#include <GLES/gl.h>
#include <string>
#include <vector>

#pragma pack(push, 1)
struct Vertex {
    /// 3D coordinates
    GLfloat XYZ[3];
    /// texture coordinates
    GLfloat ST[2];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Triangle {
    GLushort indices[3];
};
#pragma pack(pop)

// interface
class Drawable {
public:
    virtual ~Drawable() {}
    virtual bool Draw() = 0;
    virtual bool Initialized() const = 0;
};

class TexturedPlane: public Drawable {
public:
    explicit TexturedPlane(AAssetManager *manager);
    virtual ~TexturedPlane();
    virtual bool Initialized() const override;
    virtual bool Draw() override;
private:
    void LoadModel(AAssetManager *manager);
    GLuint m_texture_id;
    Vertex m_vertices[4];
    Triangle m_triangles[2];
};

class Text: public Drawable {
public:
    explicit Text(AAssetManager *manager);
    virtual ~Text();
    virtual bool Initialized() const override;
    virtual bool Draw() override;
private:
    void LoadModel(AAssetManager *manager);
    // generate m_vertices and m_triangles from string
    void GenerateTriangles(const std::string &s);

    GLuint m_texture_id;

    std::string m_string;
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;
};


#endif //EGLTEXTURE_DRAWABLE_H
