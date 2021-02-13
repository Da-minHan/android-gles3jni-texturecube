// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
// ESUtil.c
//
//    A utility library for OpenGL ES.  This library provides a
//    basic common framework for the example applications in the
//    OpenGL ES 3.0 Programming Guide.
//

///
//  Includes
//
#include <string.h>

#include "Transform.h"

#define PI 3.1415926535897932384626433832795f

void esScale ( GLfloat result[][4], GLfloat sx, GLfloat sy, GLfloat sz )
{
    result[0][0] *= sx;
    result[0][1] *= sx;
    result[0][2] *= sx;
    result[0][3] *= sx;

    result[1][0] *= sy;
    result[1][1] *= sy;
    result[1][2] *= sy;
    result[1][3] *= sy;

    result[2][0] *= sz;
    result[2][1] *= sz;
    result[2][2] *= sz;
    result[2][3] *= sz;
}

void
esTranslate ( GLfloat result[][4], GLfloat tx, GLfloat ty, GLfloat tz )
{
    result[3][0] += ( result[0][0] * tx + result[1][0] * ty + result[2][0] * tz );
    result[3][1] += ( result[0][1] * tx + result[1][1] * ty + result[2][1] * tz );
    result[3][2] += ( result[0][2] * tx + result[1][2] * ty + result[2][2] * tz );
    result[3][3] += ( result[0][3] * tx + result[1][3] * ty + result[2][3] * tz );
}

void
esRotate ( GLfloat result[][4], GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
    GLfloat sinAngle, cosAngle;
    GLfloat mag = sqrtf ( x * x + y * y + z * z );

    sinAngle = sinf ( angle * PI / 180.0f );
    cosAngle = cosf ( angle * PI / 180.0f );

    if ( mag > 0.0f )
    {
        GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
        GLfloat oneMinusCos;
        GLfloat rotMat[4][4];

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        rotMat[0][0] = ( oneMinusCos * xx ) + cosAngle;
        rotMat[0][1] = ( oneMinusCos * xy ) - zs;
        rotMat[0][2] = ( oneMinusCos * zx ) + ys;
        rotMat[0][3] = 0.0F;

        rotMat[1][0] = ( oneMinusCos * xy ) + zs;
        rotMat[1][1] = ( oneMinusCos * yy ) + cosAngle;
        rotMat[1][2] = ( oneMinusCos * yz ) - xs;
        rotMat[1][3] = 0.0F;

        rotMat[2][0] = ( oneMinusCos * zx ) - ys;
        rotMat[2][1] = ( oneMinusCos * yz ) + xs;
        rotMat[2][2] = ( oneMinusCos * zz ) + cosAngle;
        rotMat[2][3] = 0.0F;

        rotMat[3][0] = 0.0F;
        rotMat[3][1] = 0.0F;
        rotMat[3][2] = 0.0F;
        rotMat[3][3] = 1.0F;

        esMatrixMultiply (result, rotMat, result );
    }
}

void
esFrustum ( GLfloat result[][4], float left, float right, float bottom, float top, float nearZ, float farZ )
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    GLfloat     frust[4][4];

    if ( ( nearZ <= 0.0f ) || ( farZ <= 0.0f ) ||
         ( deltaX <= 0.0f ) || ( deltaY <= 0.0f ) || ( deltaZ <= 0.0f ) )
    {
        return;
    }

    frust[0][0] = 2.0f * nearZ / deltaX;
    frust[0][1] = frust[0][2] = frust[0][3] = 0.0f;

    frust[1][1] = 2.0f * nearZ / deltaY;
    frust[1][0] = frust[1][2] = frust[1][3] = 0.0f;

    frust[2][0] = ( right + left ) / deltaX;
    frust[2][1] = ( top + bottom ) / deltaY;
    frust[2][2] = - ( nearZ + farZ ) / deltaZ;
    frust[2][3] = -1.0f;

    frust[3][2] = -2.0f * nearZ * farZ / deltaZ;
    frust[3][0] = frust[3][1] = frust[3][3] = 0.0f;

    esMatrixMultiply ( result, frust, result );
}


void
esPerspective ( GLfloat result[][4], float fovy, float aspect, float nearZ, float farZ )
{
    GLfloat frustumW, frustumH;

    frustumH = tanf ( fovy / 360.0f * PI ) * nearZ;
    frustumW = frustumH * aspect;

    esFrustum ( result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}

void
esOrtho ( GLfloat result[][4], float left, float right, float bottom, float top, float nearZ, float farZ )
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    GLfloat    ortho[4][4];

    if ( ( deltaX == 0.0f ) || ( deltaY == 0.0f ) || ( deltaZ == 0.0f ) )
    {
        return;
    }

    esMatrixLoadIdentity ( ortho );
    ortho[0][0] = 2.0f / deltaX;
    ortho[3][0] = - ( right + left ) / deltaX;
    ortho[1][1] = 2.0f / deltaY;
    ortho[3][1] = - ( top + bottom ) / deltaY;
    ortho[2][2] = -2.0f / deltaZ;
    ortho[3][2] = - ( nearZ + farZ ) / deltaZ;

    esMatrixMultiply ( result, ortho, result );
}

void
esMatrixLookAt ( GLfloat result[][4],
                 float posX,    float posY,    float posZ,
                 float lookAtX, float lookAtY, float lookAtZ,
                 float upX,     float upY,     float upZ )
{
    float axisX[3], axisY[3], axisZ[3];
    float length;

    // axisZ = lookAt - pos
    axisZ[0] = lookAtX - posX;
    axisZ[1] = lookAtY - posY;
    axisZ[2] = lookAtZ - posZ;

    // normalize axisZ
    length = sqrtf ( axisZ[0] * axisZ[0] + axisZ[1] * axisZ[1] + axisZ[2] * axisZ[2] );

    if ( length != 0.0f )
    {
        axisZ[0] /= length;
        axisZ[1] /= length;
        axisZ[2] /= length;
    }

    // axisX = up X axisZ
    axisX[0] = upY * axisZ[2] - upZ * axisZ[1];
    axisX[1] = upZ * axisZ[0] - upX * axisZ[2];
    axisX[2] = upX * axisZ[1] - upY * axisZ[0];

    // normalize axisX
    length = sqrtf ( axisX[0] * axisX[0] + axisX[1] * axisX[1] + axisX[2] * axisX[2] );

    if ( length != 0.0f )
    {
        axisX[0] /= length;
        axisX[1] /= length;
        axisX[2] /= length;
    }

    // axisY = axisZ x axisX
    axisY[0] = axisZ[1] * axisX[2] - axisZ[2] * axisX[1];
    axisY[1] = axisZ[2] * axisX[0] - axisZ[0] * axisX[2];
    axisY[2] = axisZ[0] * axisX[1] - axisZ[1] * axisX[0];

    // normalize axisY
    length = sqrtf ( axisY[0] * axisY[0] + axisY[1] * axisY[1] + axisY[2] * axisY[2] );

    if ( length != 0.0f )
    {
        axisY[0] /= length;
        axisY[1] /= length;
        axisY[2] /= length;
    }

    memset ( result, 0x0, sizeof ( GLfloat ) * 16 );

    result[0][0] = -axisX[0];
    result[0][1] =  axisY[0];
    result[0][2] = -axisZ[0];

    result[1][0] = -axisX[1];
    result[1][1] =  axisY[1];
    result[1][2] = -axisZ[1];

    result[2][0] = -axisX[2];
    result[2][1] =  axisY[2];
    result[2][2] = -axisZ[2];

    // translate (-posX, -posY, -posZ)
    result[3][0] =  axisX[0] * posX + axisX[1] * posY + axisX[2] * posZ;
    result[3][1] = -axisY[0] * posX - axisY[1] * posY - axisY[2] * posZ;
    result[3][2] =  axisZ[0] * posX + axisZ[1] * posY + axisZ[2] * posZ;
    result[3][3] = 1.0f;
}


void
esMatrixMultiply ( GLfloat result[][4], GLfloat srcA[][4], GLfloat srcB[][4] )
{
    GLfloat    tmp[4][4];
    int         i;

    for ( i = 0; i < 4; i++ )
    {
        tmp[i][0] =  ( srcA[i][0] * srcB[0][0] ) +
                     ( srcA[i][1] * srcB[1][0] ) +
                     ( srcA[i][2] * srcB[2][0] ) +
                     ( srcA[i][3] * srcB[3][0] ) ;

        tmp[i][1] =  ( srcA[i][0] * srcB[0][1] ) +
                     ( srcA[i][1] * srcB[1][1] ) +
                     ( srcA[i][2] * srcB[2][1] ) +
                     ( srcA[i][3] * srcB[3][1] ) ;

        tmp[i][2] =  ( srcA[i][0] * srcB[0][2] ) +
                     ( srcA[i][1] * srcB[1][2] ) +
                     ( srcA[i][2] * srcB[2][2] ) +
                     ( srcA[i][3] * srcB[3][2] ) ;

        tmp[i][3] =  ( srcA[i][0] * srcB[0][3] ) +
                     ( srcA[i][1] * srcB[1][3] ) +
                     ( srcA[i][2] * srcB[2][3] ) +
                     ( srcA[i][3] * srcB[3][3] ) ;
    }

    memcpy ( result, tmp, sizeof ( GLfloat ) * 16 );
}

void
esMatrixLoadIdentity ( GLfloat result[][4] )
{
    memset ( result, 0x0, sizeof ( GLfloat ) * 16 );
    result[0][0] = 1.0f;
    result[1][1] = 1.0f;
    result[2][2] = 1.0f;
    result[3][3] = 1.0f;
}