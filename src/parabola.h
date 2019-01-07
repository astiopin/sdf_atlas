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

#include "mat2d.h"

enum class QbezType {
    Parabola, Line, TwoLines
};


// np10 = normalize( p0 - p1 );
// np12 = normalize( p2 - p1 );
QbezType qbez_type( F2 np10, F2 np12 );


// Calculates parabola parameters of a quadratic Bezier

struct Parabola {
    Mat2d   mat;     // Scale of the parabola transform is stored separately
    float   scale;
    float   xstart;  // Sorted parabola segment endpoints: xstart < xend;
    float   xend;

    static Parabola from_qbez( const Float2& p0, const Float2& p1, const Float2& p2 );

    static Parabola from_line( const Float2& p0, const Float2& p2 );

    Float2 pos( float x ) const;

    Float2 normal( float x ) const;

    Float2 dir( float x ) const;

    Float2 world_to_par( Float2 pos ) const;

    Float2 par_to_world( Float2 pos ) const;
};
