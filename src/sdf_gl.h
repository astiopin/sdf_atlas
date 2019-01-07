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
#include "gl_utils.h"


struct SdfVertex {
    F2    pos;        // Vertex position
    F2    par;        // Vertex position in parabola space
    F2    limits;     // Parabolic segment xstart, xend
    float scale;      // Parabola scale relative to world
    float line_width; // Line width in world space
};


struct GlyphUnf {
    UNIFORM_MATRIX( 3, transform_matrix );
};


struct SdfGl {
    
    GLuint fill_prog = 0, line_prog = 0;

    GlyphUnf ufill, uline;

    void init();

    void render_sdf( F2 tex_size, const std::vector<SdfVertex> &fill_vertices, const std::vector<SdfVertex> &line_vertices );
};
