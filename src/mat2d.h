/*
 * Copyright (c) 2019 Anton Stiopin astiopin@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "float2.h"

struct Mat2d {
    Float2 mat[3];

    Mat2d( float a = 0.0f ) {
        mat[0][0] = a;    mat[0][1] = 0.0f;
        mat[1][0] = 0.0f; mat[1][1] = a;
        mat[2][0] = 0.0f; mat[2][1] = 0.0f;
    }


    Mat2d( float m00, float m01,
          float m10, float m11, 
          float m20, float m21  ) {
        mat[0][0] = m00; mat[0][1] = m01;
        mat[1][0] = m10; mat[1][1] = m11;
        mat[2][0] = m20; mat[2][1] = m21;
    }

    Mat2d( Float2 v0, Float2 v1, Float2 v2 ) {
        mat[0] = v0;
        mat[1] = v1;
        mat[2] = v2;
    }

    Float2& operator[]( size_t index ) {
        return mat[index];
    }    

    const Float2& operator[]( size_t index ) const {
        return mat[index];
    }
    
    float* ptr() {
        return mat[0].ptr();
    }
};


inline Mat2d operator*( const Mat2d& a, const Mat2d& b ) {
    Mat2d res;
    res[0] = a[0] * F2 { b[0][0] } + a[1] * F2 { b[0][1] };
    res[1] = a[0] * F2 { b[1][0] } + a[1] * F2 { b[1][1] };
    res[2] = a[0] * F2 { b[2][0] } + a[1] * F2 { b[2][1] } + a[2];

    return res;
}

inline Float2 operator*( const Mat2d& m, const Float2& v ) {
    Float2 res;
    res = m[0] * F2{ v[0] } + m[1] * F2{ v[1] } + m[2];
    return res;
}

inline Mat2d operator*( const Mat2d& m, float a ) {
    Mat2d res;
    F2 a2 { a };
    res[0] = m[0] * a2;
    res[1] = m[1] * a2;
    res[2] = m[2] * a2;
    return res;
}

inline float det( const Mat2d& m ) {
    return m[0][0] * m[1][1] - m[1][0] * m[0][1];
}


inline Mat2d invert( const Mat2d& m ) {
    Mat2d res;

    const float invdet = 1.0f / det( m );

    res[0][0] = invdet *  m[1][1];  res[0][1] = invdet * -m[0][1];
    res[1][0] = invdet * -m[1][0];  res[1][1] = invdet *  m[0][0];

    res[2][0] = invdet * ( m[1][0] * m[2][1] - m[2][0] * m[1][1] );
    res[2][1] = invdet * ( m[0][1] * m[2][0] - m[0][0] * m[2][1] );
    
    return res;
}

inline Mat2d screen_matrix( Float2 screen_size ) {
    float sw = screen_size.x;
    float sh = screen_size.y;
    Mat2d scr_matrix( 2.0 / sw, 0,   0, 2.0 / sh,   -1.0, -1.0 );
    return scr_matrix;
}

