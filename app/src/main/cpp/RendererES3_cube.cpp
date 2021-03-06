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
#include "Shaders.h"

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define ELEMENT_ATTRIB 2
#define TEXTURE_ATTRIB 3

struct image {
    const char * path;
    GLuint width;
    GLuint height;
    GLuint channels_per_pixel;
};

struct model {
    GLint loc;
    GLfloat MVPmatrix[4][4];
    GLuint texture;
    struct image data;
};

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
    virtual void setMVPMatrix(GLfloat MVPmatrix[][4], GLfloat perspectiveMat[][4], GLfloat modelviewMat[][4]);
    virtual void simpeTexturing();
    virtual void cubeTexturing(struct model * m);
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

    struct model mCube;
    struct model mSkybox;
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

    esMatrixLoadIdentity(mCube.MVPmatrix);
    esMatrixLoadIdentity(mSkybox.MVPmatrix);

    mCube.data.path = "/data/data/com.android.gles3jni/files/container.bmp";
    mCube.data.width = mCube.data.height = 512;
    mCube.data.channels_per_pixel = 4;

    mSkybox.data.path = "/data/data/com.android.gles3jni/files/container.bmp";
    mSkybox.data.width = mSkybox.data.height = 512;
    mSkybox.data.channels_per_pixel = 4;
}

bool RendererES3::init() {
    mProgram = createProgram(TRIANGLE_VERTEX_SHADER, TRIANGLE_FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    mCube.loc = glGetUniformLocation(mProgram, "u_mvpMatrix");
    mSkybox.loc = glGetUniformLocation(mProgram, "u_mvpMatrix");

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

    cubeTexturing(&mCube);
    cubeTexturing(&mSkybox);

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

void RendererES3::setMVPMatrix(GLfloat MVPmatrix[][4], GLfloat perspectiveMat[][4], GLfloat modelviewMat[][4]) {
    float aspect = getAspect();
    esPerspective(perspectiveMat, 80.0f, aspect, 1.0f, 100.0f);
/*
    esMatrixLookAt ( modelviewMat,
            0.0f, 0.0f, 2.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f);
*/
    //esTranslate(modelviewMat, 0.0f, 0.0f, -1.0f);
    //esRotate(modelviewMat, getAngle(), 1.0f, 1.0f, 1.0f);
    //esScale(modelviewMat, 0.5f, 0.5f, 0.5f);

    esMatrixMultiply ( MVPmatrix, modelviewMat, perspectiveMat );
}

void RendererES3::simpeTexturing() {
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

void RendererES3::cubeTexturing(struct model * m) {
    GLubyte * image = (GLubyte *)malloc(sizeof(GLubyte) * m->data.width * m->data.height * m->data.channels_per_pixel);
    FILE * image_file = fopen(m->data.path, "r");

    if(image_file == NULL)
    {
        ALOGE("Failure to load the texture");
        return;
    }

    fread(image, m->data.width * m->data.height * m->data.channels_per_pixel, 1, image_file);

    /* Use tightly packed data. */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Generate a texture object. */
    glGenTextures((GLsizei)1, &(m->texture));

    /* Activate a texture. */
    glActiveTexture(GL_TEXTURE0);

    /* Bind the texture object. */
    glBindTexture(GL_TEXTURE_CUBE_MAP, m->texture);

    /* Load the texture. */
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, theTexture);
    for(GLuint i = 0; i < 6; i++)
    {
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGBA, m->data.width, m->data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    free(image);
}

void RendererES3::draw(unsigned int numInstances) {
    glUseProgram(mProgram);
    glBindVertexArray(mVBState);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numInstances);
}

void RendererES3::draw_triangle(unsigned int numInstances) {
    int size;
    GLfloat perspectiveMat[4][4], modelviewMat[4][4];

    esMatrixLoadIdentity(perspectiveMat);
    esMatrixLoadIdentity(modelviewMat);

    glUseProgram(mProgram);

    //glFrontFace(GL_CW);
    //glDepthMask(GL_FALSE);
    //glDepthFunc(GL_LEQUAL);

    // get matrix's uniform location and set matrix
    // cube
    esTranslate(modelviewMat, 0.0f, 0.0f, -5.0f);
    esRotate(modelviewMat, getAngle(), 1.0f, 1.0f, 1.0f);
    esScale(modelviewMat, 0.5f, 0.5f, 0.5f);
   // esMatrixLookAt ( modelviewMat,
     //                0.0f, 0.0f, -1.0f,
       //              0.0f, 0.0f, -2.0f,
         //            0.0f, 1.0f, 0.0f);
    setMVPMatrix(mCube.MVPmatrix, perspectiveMat, modelviewMat);
    glUniformMatrix4fv(mCube.loc, 1, GL_FALSE, (GLfloat*)(&(mCube.MVPmatrix[0][0])));

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size / sizeof(GLubyte), GL_UNSIGNED_BYTE, 0);

    // skybox cube
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    esMatrixLoadIdentity(perspectiveMat);
    esMatrixLoadIdentity(modelviewMat);
    esTranslate(modelviewMat, 0.0f, 0.0f, -5.0f);
    esScale(modelviewMat, 10.0f, 10.0f, 10.0f);
    /*esMatrixLookAt ( modelviewMat,
                     0.0f, 0.0f, -1.0f,
                     0.0f, 0.0f, -2.0f,
                     0.0f, 1.0f, 0.0f);
*/
    setMVPMatrix(mSkybox.MVPmatrix, perspectiveMat, modelviewMat);
    glUniformMatrix4fv(mSkybox.loc, 1, GL_FALSE, (GLfloat*)(&(mSkybox.MVPmatrix[0][0])));

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size / sizeof(GLubyte), GL_UNSIGNED_BYTE, 0);
    //glDisable(GL_CULL_FACE);
}