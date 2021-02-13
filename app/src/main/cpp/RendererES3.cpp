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

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define ELEMENT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "layout(location = " STRV(POS_ATTRIB) ") in vec2 pos;\n"
    "layout(location=" STRV(COLOR_ATTRIB) ") in vec4 color;\n"
    "layout(location=" STRV(SCALEROT_ATTRIB) ") in vec4 scaleRot;\n"
    "layout(location=" STRV(OFFSET_ATTRIB) ") in vec2 offset;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);\n"
    "    gl_Position = vec4(sr*pos + offset, 0.0, 1.0);\n"
    "    vColor = color;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 vColor;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vColor;\n"
    "}\n";

static const char TRIANGLE_VERTEX_SHADER[] =
        "#version 300 es\n"
        "uniform mat4 u_mvpMatrix;\n"
        "layout(location = 0) in vec4 vPosition;\n"
        "layout(location = 1) in vec4 color;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "    gl_Position = u_mvpMatrix * vPosition;\n"
        "    vColor = color;\n"
        "}\n";

static const char TRIANGLE_FRAGMENT_SHADER[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vColor;\n"
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
    virtual void draw(unsigned int numInstances);
    virtual void draw_triangle(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
    GLuint mVBO[2];
    GLint mLoc;
    GLfloat mMVPmatrix[4][4];
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
    GLfloat vVertices[] = {  0.5f,  0.5f, 0.0f,
                             -0.5f, -0.5f, 0.0f,
                             0.5f, -0.5f, 0.0f,
                             -0.5f, 0.5f, 0.0f,
    };
    GLfloat vColors[] = {  1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 1.0f,
                           0.0f, 1.0f, 1.0f
    };
    GLubyte vIndices[] = {0, 1, 3,
                          0, 1, 2
    };

    mProgram = createProgram(TRIANGLE_VERTEX_SHADER, TRIANGLE_FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    mLoc = glGetUniformLocation(mProgram, "u_mvpMatrix");

    glGenBuffers(2, mVBO);

    // Load the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);
    glVertexAttribPointer ( POS_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray ( POS_ATTRIB );

    // Load the color data
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vColors), vColors, GL_STATIC_DRAW);
    glVertexAttribPointer ( COLOR_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray (COLOR_ATTRIB);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vIndices), vIndices, GL_STATIC_DRAW);
    glVertexAttribPointer ( ELEMENT_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray (ELEMENT_ATTRIB);

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
    esMatrixLoadIdentity(mMVPmatrix);

    float aspect = getAspect();
    esPerspective(mMVPmatrix, 60.0f, aspect, 0.1f, 20.0f);
    esTranslate(mMVPmatrix, 0.0f, 0.0f, 0.1f);
    //esRotate(mMVPmatrix, 20.0f, 0.0f, 0.0f, 1.0f);
    esScale(mMVPmatrix, 0.5f, 0.5f, 0.5f);

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

    glDrawArrays ( GL_TRIANGLES, 0, 3 );

    esTranslate(mMVPmatrix, 0.5f, 0.5f, 0.5f);
    //esRotate(mMVPmatrix, 60.0f, 1.0f, 0.0f, 0.0f);

    glUniformMatrix4fv(mLoc, 1, GL_FALSE, (GLfloat*)(&mMVPmatrix[0][0]));
    glDrawArrays ( GL_TRIANGLES, 0, 3 );
}