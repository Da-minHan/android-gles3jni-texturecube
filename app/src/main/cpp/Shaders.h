//
// Created by dm93.han on 2021-02-16.
//

#ifndef ANDROID_GLES3JNI_TEXTURECUBE_SHADERS_H
#define ANDROID_GLES3JNI_TEXTURECUBE_SHADERS_H

static const char TRIANGLE_VERTEX_SHADER[] =
        "#version 300 es\n"
        "uniform mat4 u_mvpMatrix;\n"
        "layout(location = 0) in vec3 vPosition;\n"
        "layout(location = 1) in vec4 color;\n"
        "layout(location = 3) in vec2 vTexCoord;\n"
        "out vec4 vColor;\n"
        "out vec3 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = u_mvpMatrix * vec4(vPosition, 1.0f);\n"
        "    TexCoord = vPosition;\n"
        "}\n";

static const char TRIANGLE_FRAGMENT_SHADER[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "in vec3 TexCoord;\n"
        "out vec4 outColor;\n"
        "uniform samplerCube ourTexture;\n"
        "void main() {\n"
        "   outColor = texture(ourTexture, TexCoord);\n"
        "}\n";

#endif //ANDROID_GLES3JNI_TEXTURECUBE_SHADERS_H
