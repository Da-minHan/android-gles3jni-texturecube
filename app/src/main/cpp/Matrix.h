//
// Created by 한다민 on 2021. 2. 13..
//

#ifndef GLES3JNI_MATRIX_H
#define GLES3JNI_MATRIX_H

#if DYNAMIC_ES3
#include "gl3stub.h"
#else
// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif
#endif

GLfloat cubeVertices[] = {-1.0f,  1.0f, -1.0f, /* Back. */
                          1.0f,  1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f, /* Front. */
                          1.0f,  1.0f,  1.0f,
                          -1.0f, -1.0f,  1.0f,
                          1.0f, -1.0f,  1.0f,
                          -1.0f,  1.0f, -1.0f, /* Left. */
                          -1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f,  1.0f,
                          -1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f, -1.0f, /* Right. */
                          1.0f, -1.0f, -1.0f,
                          1.0f, -1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,
                          -1.0f, 1.0f, -1.0f, /* Top. */
                          -1.0f, 1.0f,  1.0f,
                          1.0f, 1.0f,  1.0f,
                          1.0f, 1.0f, -1.0f,
                          -1.0f, - 1.0f, -1.0f, /* Bottom. */
                          -1.0f,  -1.0f,  1.0f,
                          1.0f, - 1.0f,  1.0f,
                          1.0f,  -1.0f, -1.0f
};

GLfloat textureCords[] = { 1.0f, 1.0f, /* Back. */
                           0.0f, 1.0f,
                           1.0f, 0.0f,
                           0.0f, 0.0f,
                           0.0f, 1.0f, /* Front. */
                           1.0f, 1.0f,
                           0.0f, 0.0f,
                           1.0f, 0.0f,
                           0.0f, 1.0f, /* Left. */
                           0.0f, 0.0f,
                           1.0f, 0.0f,
                           1.0f, 1.0f,
                           1.0f, 1.0f, /* Right. */
                           1.0f, 0.0f,
                           0.0f, 0.0f,
                           0.0f, 1.0f,
                           0.0f, 1.0f, /* Top. */
                           0.0f, 0.0f,
                           1.0f, 0.0f,
                           1.0f, 1.0f,
                           0.0f, 0.0f, /* Bottom. */
                           0.0f, 1.0f,
                           1.0f, 1.0f,
                           1.0f, 0.0f
};
/* [verticesAndTexture] */

GLubyte indicies[] = {0, 3, 2, 0, 1, 3, 4, 6, 7, 4, 7, 5,  8, 9, 10, 8, 11, 10, 12, 13, 14, 15, 12, 14, 16, 17, 18, 16, 19, 18, 20, 21, 22, 20, 23, 22};


#endif //GLES3JNI_MATRIX_H
