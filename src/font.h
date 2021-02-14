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
#include <cstdint>
#include <unordered_map>
#include "float2.h"
#include "mat2d.h"


struct Glyph {    
    enum CharType {
        Lower = 1, Upper = 2, Punct = 4, Space = 8, Other = 0
    } char_type = Other;

    float advance_width     = 0.0f;
    float left_side_bearing = 0.0f;

    F2 min = F2{ 0.0f };
    F2 max = F2{ 0.0f };

    int command_start = 0;
    int command_count = 0;

    bool is_composite = false;

    int components_start = 0;
    int components_count = 0;
};


struct GlyphCommand {
    enum Type {
        MoveTo, LineTo, BezTo, ClosePath
    } type = MoveTo;

    F2 p0 = F2{ 0.0f };
    F2 p1 = F2{ 0.0f };
};


struct GlyphComponent {
    int   glyph_idx;
    Mat2d transform;
};


struct Font {
    // Kerning map: ( left_codepoint << 16 & right_codepoint ) -> kerning advance distance
    std::unordered_map<uint32_t, float>  kern_map;

    // Glyph map: codepoint -> glyph index 
    std::unordered_map<uint32_t, int>    glyph_map;

    // Glyph array
    std::vector<Glyph>                   glyphs;

    // Array of glyph display commands
    std::vector<GlyphCommand>            glyph_commands;

    // Array of composite glyph indices
    std::vector<GlyphComponent>          glyph_components;

    // Font metrics in em
    float em_ascent, em_descent, em_line_gap;

    // Font metrics relative to ascent (ascent == 1.0)
    float ascent, descent, line_gap;

    // Glyph maximum bounding box
    F2    glyph_min, glyph_max;

    bool load_ttf_file( const char *filename );

    bool load_ttf_mem( const uint8_t *ttf );

    // Find glyph index by codepoint
    int glyph_idx( uint32_t codepoint ) const {
        auto iter = glyph_map.find( codepoint );
        return iter == glyph_map.end() ? -1 : iter->second;
    }

    int kern_advance( uint32_t cp1, uint32_t cp2 );
};
