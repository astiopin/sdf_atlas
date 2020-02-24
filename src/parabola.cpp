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

#include "parabola.h"

QbezType qbez_type( F2 np10, F2 np12 ) {
    float d = dot( np10, np12 );
    float dmax = 1.0 - 1e-6f;
    if ( d >= dmax ) return QbezType::TwoLines;
    if ( d <= -dmax ) return QbezType::Line;
    return QbezType::Parabola;
}

Parabola Parabola::from_line( const Float2& p0, const Float2& p2 ) {
    float precision = 1e-16;
    Parabola res;

    Float2 pc      = mix( p0, p2, 0.5f );
    Float2 x_axis  = normalize( p2 - p0 );
    float  ldir    = length( p2 - p0 );
    Float2 y_axis  = perp_left( x_axis );
    float  ylen    = ldir * precision;
    Float2 vertex  = pc + ylen * y_axis;
    float  xlen    = sqrtf( precision );

    res.xstart     = -xlen;
    res.xend       = xlen;
    res.scale      = 0.5f * ldir / xlen;
    res.mat        = Mat2d( x_axis, y_axis, vertex );

    return res;
}


Parabola Parabola::from_qbez( const Float2& p0, const Float2& p1, const Float2& p2 ) {
    Parabola res;

    Float2 pc = mix( p0, p2, 0.5f );
    Float2 yaxis = normalize( pc - p1 );
    Float2 xaxis = perp_right( yaxis );

    Float2 p01 = normalize( p1 - p0 );
    Float2 p12 = normalize( p2 - p1 );
    float cx0 = dot( xaxis, p01 );
    float sx0 = dot( yaxis, p01 );
    float cx2 = dot( xaxis, p12 );
    float sx2 = dot( yaxis, p12 );

    float x0 = sx0 / cx0 * 0.5;
    float x2 = sx2 / cx2 * 0.5;
    float y0 = x0*x0;

    float p02x = dot( p2 - p0, xaxis );
    float scale = p02x / ( x2 - x0 );
    
    Float2 vertex = p0 - F2( y0 * scale ) * yaxis - F2( x0 * scale ) * xaxis;

    res.scale  = scale;
    res.mat    = Mat2d( xaxis, yaxis, vertex );

    if ( x0 < x2 ) {
        res.xstart = x0;
        res.xend   = x2;
    } else {
        res.xstart = x2;
        res.xend   = x0;
    }

    return res;
}

Float2 Parabola::pos( float x ) const {
    return mat[2] + F2( scale * x ) * mat[0] + F2( scale * x * x ) * mat[1];
}

Float2 Parabola::normal( float x ) const {
    return perp_left( dir( x ) );
}

Float2 Parabola::dir( float x ) const {
    return normalize( mat[0] + mat[1] * F2( 2.0f * x ) );
}

F2 Parabola::world_to_par( F2 pos ) const {
    float is = 1.0 / scale;
    F2 dpos = pos - mat[2];
    F2 r0 = dpos * mat[0];
    F2 r1 = dpos * mat[1];
    float v0 = is * ( r0.x + r0.y );
    float v1 = is * ( r1.x + r1.y );
    return F2 { v0, v1 };
}

F2 Parabola::par_to_world( F2 pos ) const {
    return mat[2] + scale * pos.x * mat[0] + scale * pos.y * mat[1];
}
