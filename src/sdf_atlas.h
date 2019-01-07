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

#include "glyph_painter.h"

struct GlyphRect {
    uint32_t codepoint = 0;
    int      glyph_idx = 0;
    float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;    
};

struct SdfAtlas {
    Font *font        = nullptr;
    float tex_width   = 2048.0f;
    float row_height  = 96.0f;
    float sdf_size    = 16.0f;
    int   glyph_count = 0;

    float posx = 0;
    float posy = 0;
    int   max_height = 0;

    std::vector<GlyphRect> glyph_rects;

    void init( Font *font, float tex_width, float row_height, float sdf_size );

    void allocate_codepoint( uint32_t codepoint );

    void allocate_all_glyphs();    

    void allocate_unicode_range( uint32_t start, uint32_t end ); // end is inclusive    
    
    void draw_glyphs( GlyphPainter& gp ) const;

    std::string json( float tex_height, bool flip_texcoord_y = true ) const;
};
