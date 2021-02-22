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

#include "sdf_atlas.h"

#include <algorithm>
#include <unordered_set>
#include <iostream>
#include <sstream>

void SdfAtlas::init( Font *font, float tex_width, float row_height, float sdf_size ) {
    this->font = font;

    glyph_rects.clear();

    this->tex_width  = tex_width;
    this->row_height = row_height;
    this->sdf_size   = sdf_size;
    glyph_count = 0;
    posx = 0.0f;
    posy = 0.0f;
    max_height = row_height + sdf_size * 2.0f;
}

void SdfAtlas::allocate_codepoint( uint32_t codepoint ) {
    int glyph_idx = font->glyph_idx( codepoint );
    if ( glyph_idx == -1 ) return;
    if ( glyph_idx == 0 ) return;
    const Glyph& g = font->glyphs[ glyph_idx ];
    if ( g.command_count <= 2 ) return;
    
    float fheight = font->ascent - font->descent;
    float scale = row_height / fheight;
    float rect_width = ( g.max.x - g.min.x ) * scale + sdf_size * 2.0f;
    float row_and_border = row_height + sdf_size * 2.0f;

    if ( ( posx + rect_width ) > tex_width ) {
        posx = 0.0f;
        
        posy = ceil( posy + row_and_border );
        max_height = ceil( posy + row_and_border );
    }

    GlyphRect gr;
    gr.codepoint = codepoint;
    gr.glyph_idx = glyph_idx;
    gr.x0 = posx;
    gr.x1 = posx + rect_width;
    gr.y0 = posy;
    gr.y1 = posy + row_and_border;

    glyph_rects.push_back( gr );

    posx = ceil( posx + rect_width );
    glyph_count++;
}

void SdfAtlas::allocate_all_glyphs() {
    for ( auto kv : font->glyph_map ) {
        allocate_codepoint( kv.first );
    }
}

void SdfAtlas::allocate_unicode_range( uint32_t start, uint32_t end ) {
    for ( uint32_t ucp = start; ucp <= end; ++ucp ) {
        allocate_codepoint( ucp );
    }
}

void SdfAtlas::draw_glyphs( GlyphPainter& gp ) const {
    float fheight = font->ascent - font->descent;
    float scale = row_height / fheight;
    float baseline = -font->descent * scale;
    
    for ( size_t iglyph = 0; iglyph < glyph_rects.size(); ++iglyph ) {
        const GlyphRect& gr = glyph_rects[ iglyph ];
        float left = font->glyphs[ gr.glyph_idx ].left_side_bearing * scale;
        F2 glyph_pos = F2 { gr.x0, gr.y0 + baseline } + F2 { sdf_size - left, sdf_size };
        gp.draw_glyph( font, gr.glyph_idx, glyph_pos, scale, sdf_size );
    }
}

std::string SdfAtlas::json( float tex_height, bool flip_texcoord_y ) const {
    float fheight = font->ascent - font->descent;
    float scaley = row_height / tex_height / fheight; 
    float scalex = row_height / tex_width / fheight;   

    const Glyph& gspace = font->glyphs[ font->glyph_idx( ' ' ) ];
    const Glyph& gx     = font->glyphs[ font->glyph_idx( 'x' ) ];
    const Glyph& gxcap  = font->glyphs[ font->glyph_idx( 'X' ) ];
    
    std::unordered_set<uint32_t> codepoints;
    for ( size_t igr = 0; igr < glyph_rects.size(); ++igr ) {
        codepoints.insert( glyph_rects[igr].codepoint );
    }

    std::stringstream ss;
    ss << "{" << std::endl;
    ss << "    ix: " << sdf_size / tex_width << ", " << std::endl;
    ss << "    iy: " << sdf_size / tex_height << ", " << std::endl;
    ss << "    row_height: " << ( row_height + 2.0f * sdf_size ) / tex_height << ", " << std::endl;
    ss << "    aspect: " <<  tex_width / tex_height << ", " << std::endl;
    ss << "    ascent: " << font->ascent * scaley << ", " << std::endl;
    ss << "    descent: " << fabsf( font->descent * scaley ) << ", " << std::endl;
    ss << "    line_gap: " << font->line_gap * scaley << ", " << std::endl;
    ss << "    cap_height: " << gxcap.max.y * scaley  << ", " << std::endl;
    ss << "    x_height: " << gx.max.y * scaley  << ", " << std::endl;
    ss << "    space_advance: " << gspace.advance_width * scalex << ", " << std::endl << std::endl;
    
    ss << "    chars: { " << std::endl;

    for ( size_t igr = 0; igr < glyph_rects.size(); ++igr ) {
        const GlyphRect& gr = glyph_rects[ igr ];
        const Glyph& g = font->glyphs[ gr.glyph_idx ];
        float tcy0 = gr.y0 / tex_height;
        float tcy1 = gr.y1 / tex_height;

        if ( flip_texcoord_y ) {
            tcy0 = 1.0 - gr.y1 / tex_height;
            tcy1 = 1.0 - gr.y0 / tex_height;
        }

        char ucp[32];
        snprintf( ucp, 32, "    \"\\u%04x\": {", gr.codepoint );
        ss << ucp << std::endl;
        ss << "        codepoint: " << gr.codepoint << "," << std::endl;
        ss << "        rect: [";
        ss << gr.x0 / tex_width << ", " << tcy0 << ", ";
        ss << gr.x1 / tex_width << ", " << tcy1 << "]," << std::endl;
        ss << "        bearing_x: " << g.left_side_bearing * scalex << "," << std::endl;
        ss << "        advance_x: " << g.advance_width * scalex << "," << std::endl;
        ss << "        flags: " << (int)g.char_type << std::endl;
        ss << "    }";
        if ( igr != glyph_rects.size() - 1 ) ss << ",";
        ss << std::endl;
    }

    ss << "    }, // end chars" << std::endl;

    ss << "    kern: {" << std::endl;

    for ( auto kv : font->kern_map ) {
        uint32_t kern_pair = kv.first;
        float kern_value = kv.second * scalex;

        int kern_first_idx = ( kern_pair >> 16 ) & 0xffff;        
        int kern_second_idx = kern_pair & 0xffff;

        auto it1 = font->cp_map.find( kern_first_idx );
        auto it2 = font->cp_map.find( kern_second_idx );

        if ( it1 == font->cp_map.end() || it2 == font->cp_map.end() ) {
            continue;
        }

        const std::vector<uint32_t>& v1 = it1->second;
        const std::vector<uint32_t>& v2 = it2->second;

        for ( uint32_t kern_first : v1 ) {
            for ( uint32_t kern_second : v2 ) {
                bool first_found = codepoints.find( kern_first ) != codepoints.end();
                bool second_found = codepoints.find( kern_second ) != codepoints.end();
                if ( first_found && second_found ) {
                    char uckern[ 64 ];
                    snprintf( uckern, 64, "        \"\\u%04x\\u%04x\" : ", kern_first, kern_second );
                    ss << uckern << kern_value << "," << std::endl;
                }
            }
        }
    }

    ss << "    } // end kern" << std::endl;

    ss << "}; // end font" << std::endl;    
    
    return ss.str();
}
