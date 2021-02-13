/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gles3jni.h"
#include <EGL/egl.h>
#include <string.h>
#include "Transform.h"
#include "Matrix.h"

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define ELEMENT_ATTRIB 2
#define TEXTURE_ATTRIB 3

static const char TRIANGLE_VERTEX_SHADER[] =
        "#version 300 es\n"
        "uniform mat4 u_mvpMatrix;\n"
        "layout(location = 0) in vec4 vPosition;\n"
        "layout(location = 1) in vec4 color;\n"
        "layout(location = 3) in vec2 vTexCoord;\n"
        "out vec4 vColor;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = u_mvpMatrix * vPosition;\n"
        "    TexCoord = vTexCoord;\n"
        "}\n";

static const char TRIANGLE_FRAGMENT_SHADER[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "in vec2 TexCoord;\n"
        "out vec4 outColor;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {\n"
        "   outColor = texture(ourTexture, TexCoord);\n"
        "}\n";

class RendererES3: public Renderer {
public:
    RendererES3();
    virtual ~RendererES3();
    bool init();

private:
    enum {VB_INSTANCE, VB_SCALEROT, VB_OFFSET, VB_COUNT};

    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    virtual float* mapTransformBuf();
    virtual void unmapTransformBuf();
    virtual void setMVPMatrix();
    virtual void simpeTexturing();
    virtual void texturing();
    virtual void draw(unsigned int numInstances);
    virtual void draw_triangle(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
    GLuint mVBO[4];
    GLint mLoc;
    GLfloat mMVPmatrix[4][4];
    GLuint texture;
};

Renderer* createES3Renderer() {
    RendererES3* renderer = new RendererES3;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

RendererES3::RendererES3()
        :   mEglContext(eglGetCurrentContext()),
            mProgram(0),
            mVBState(0){
}

bool RendererES3::init() {
    GLfloat cube_vertices[] = {
            // front
            -0.5, -0.5,  0.5,
            0.5, -0.5,  0.5,
            0.5,  0.5,  0.5,
            -0.5,  0.5,  0.5,
            // back
            -0.5, -0.5, -0.5,
            0.5, -0.5, -0.5,
            0.5,  0.5, -0.5,
            -0.5,  0.5, -0.5
    };

    GLfloat cube_colors[] = {
            // front colors
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,
            1.0, 1.0, 1.0,
            // back colors
            1.0, 1.0, 0.0,
            0.0, 1.0, 1.0,
            1.0, 0.0, 1.0,
            0.0, 0.0, 0.0
    };

    GLubyte cube_elements[] = {
            // front
            0, 1, 2,
            2, 3, 0,
            // right
            1, 5, 6,
            6, 2, 1,
            // back
            7, 6, 5,
            5, 4, 7,
            // left
            4, 0, 3,
            3, 7, 4,
            // bottom
            4, 5, 1,
            1, 0, 4,
            // top
            3, 2, 6,
            6, 7, 3
    };

    GLfloat cube_tex[] = {
            // front
            0.0, 0.0,
            1.0, 0.0,
            1.0,  1.0,
            0.0,  1.0,
            // back
            0.0, 0.0,
            1.0, 0.0,
            1.0,  1.0,
            0.0,  1.0
    };

    esMatrixLoadIdentity(mMVPmatrix);

    mProgram = createProgram(TRIANGLE_VERTEX_SHADER, TRIANGLE_FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    mLoc = glGetUniformLocation(mProgram, "u_mvpMatrix");

    glGenBuffers(4, mVBO);

    // Load the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[0]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer ( POS_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray ( POS_ATTRIB );

    // Load the color data
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);
    glVertexAttribPointer ( COLOR_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray (COLOR_ATTRIB);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBO[2]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
    glVertexAttribPointer ( ELEMENT_ATTRIB, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray (ELEMENT_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO[3]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCords), textureCords, GL_STATIC_DRAW);
    glVertexAttribPointer ( TEXTURE_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray (TEXTURE_ATTRIB);

    texturing();

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

RendererES3::~RendererES3() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
    glDeleteProgram(mProgram);

    glDisableVertexAttribArray (POS_ATTRIB);
    glDisableVertexAttribArray (COLOR_ATTRIB);
    glDisableVertexAttribArray (ELEMENT_ATTRIB);
}

float* RendererES3::mapOffsetBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    return (float*)glMapBufferRange(GL_ARRAY_BUFFER,
                                    0, MAX_INSTANCES * 2*sizeof(float),
                                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3::unmapOffsetBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float* RendererES3::mapTransformBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    return (float*)glMapBufferRange(GL_ARRAY_BUFFER,
                                    0, MAX_INSTANCES * 4*sizeof(float),
                                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3::unmapTransformBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void RendererES3::setMVPMatrix() {
    GLfloat perspectiveMat[4][4];
    GLfloat modelviewMat[4][4];

    esMatrixLoadIdentity(perspectiveMat);
    esMatrixLoadIdentity(modelviewMat);

    float aspect = getAspect();
    esPerspective(perspectiveMat, 60.0f, aspect, 0.0f, 100.0f);

    esTranslate(modelviewMat, 0.0f, 0.0f, 0.0f);
    esRotate(modelviewMat, getAngle(), 1.0f, 1.0f, 1.0f);
    esScale(modelviewMat, 0.5f, 0.5f, 0.5f);

    esMatrixMultiply ( mMVPmatrix, modelviewMat, perspectiveMat );
}

void RendererES3::simpeTexturing() {
    /* 3 x 3 Image,  R G B A Channels RAW Format. */
    GLubyte pixels[9 * 4] =
            {
                    18,  140, 171, 255, /* Some Colour Bottom Left. */
                    143, 143, 143, 255, /* Some Colour Bottom Middle. */
                    255, 255, 255, 255, /* Some Colour Bottom Right. */
                    255, 255, 0,   255, /* Yellow Middle Left. */
                    0,   255, 255, 255, /* Some Colour Middle. */
                    255, 0,   255, 255, /* Some Colour Middle Right. */
                    255, 0,   0,   255, /* Red Top Left. */
                    0,   255, 0,   255, /* Green Top Middle. */
                    0,   0,   255, 255, /* Blue Top Right. */
            };
    /* [includeTextureDefinition] */

    /* [placeTextureInMemory] */
    /* Use tightly packed data. */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Generate a texture object. */
    glGenTextures((GLsizei)1, &texture);

    /* Activate a texture. */
    glActiveTexture(GL_TEXTURE0);

    /* Bind the texture object. */
    glBindTexture(GL_TEXTURE_2D, texture);

    /* Load the texture. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    /* Set the filtering mode. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void RendererES3::texturing() {
    //loadTexture("/data/data/com.android.gles3jni/files/container.bmp", 0, 10, 10);
    const char * texturepath="/data/data/com.android.gles3jni/files/container.bmp";
    unsigned int level=0;
    unsigned int width=512;
    unsigned int height=512;
    GLubyte * theTexture;
    theTexture = (GLubyte *)malloc(sizeof(GLubyte) * width * height * CHANNELS_PER_PIXEL);

    FILE * theFile = fopen(texturepath, "r");
    if(theFile == NULL)
    {
        ALOGE("Failure to load the texture");
        return;
    }

    fread(theTexture, width * height * CHANNELS_PER_PIXEL, 1, theFile);

    /* Use tightly packed data. */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Generate a texture object. */
    glGenTextures((GLsizei)1, &texture);

    /* Activate a texture. */
    glActiveTexture(GL_TEXTURE0);

    /* Bind the texture object. */
    glBindTexture(GL_TEXTURE_2D, texture);

    /* Load the texture. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, theTexture);

    /* Set the filtering mode. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    free(theTexture);
}

void RendererES3::draw(unsigned int numInstances) {
    glUseProgram(mProgram);
    glBindVertexArray(mVBState);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numInstances);
}

void RendererES3::draw_triangle(unsigned int numInstances) {
    glUseProgram(mProgram);

    // get matrix's uniform location and set matrix
    setMVPMatrix();
    glUniformMatrix4fv(mLoc, 1, GL_FALSE, (GLfloat*)(&mMVPmatrix[0][0]));
    //simpeTexturing();

    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size / sizeof(GLubyte), GL_UNSIGNED_BYTE, 0);

}