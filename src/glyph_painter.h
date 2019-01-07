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

#include <vector>

#include "float2.h"
#include "sdf_gl.h"
#include "font.h"


struct FillPainter {
    std::vector<SdfVertex> vertices;

    F2 fan_pos = F2( 0.0f );
    F2 prev_pos = F2( 0.0f );

    void move_to( F2 p0 );

    void line_to( F2 p1 );

    void qbez_to( F2 p1, F2 p2 );

    void close();
};


struct LinePainter {
    std::vector<SdfVertex> vertices;

    F2 start_pos = F2( 0.0f );    
    F2 prev_pos;

    void move_to( F2 p0 );

    void line_to( F2 p1, float line_width );

    void qbez_to( F2 p1, F2 p2, float line_width );

    void close( float line_width );
};


struct GlyphPainter {
    
    FillPainter fp;

    LinePainter lp;
    
    void draw_glyph( const Font *font, int glyph_index, F2 pos, float scale, float sdf_size );

    void clear() {
        fp.vertices.clear();
        lp.vertices.clear();
    }
};
