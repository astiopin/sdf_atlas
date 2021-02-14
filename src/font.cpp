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

#include "font.h"
#include <cassert>
#include <cwctype>
#include <iostream>


// Convert high-endian TTF values to low-endian
// TODO support for high-endian architectures

inline uint16_t ttf_u16( const uint8_t *p ) { return p[0] * 256 + p[1]; }
inline uint32_t ttf_u32( const uint8_t *p ) { return ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3]; }

inline int16_t ttf_i16( const uint8_t *p ) { return p[0] * 256 + p[1]; }
inline int32_t ttf_i32( const uint8_t *p ) { return ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3]; }


inline bool check_tag( const uint8_t *d, const char *tag ) {
    bool match = d[0] == tag[0] &&
                 d[1] == tag[1] &&
                 d[2] == tag[2] &&
                 d[3] == tag[3];
    return match;
}


inline bool is_font( const uint8_t *ttf ) {
    bool res = check_tag( ttf, "1\0\0\0" )
        || check_tag( ttf, "\0\1\0\0" )
        || check_tag( ttf, "typ1" )
        || check_tag( ttf, "OTTO" );
    return res;
}


static const uint8_t* find_table( const uint8_t *ttf, const char *tag ) {
    uint32_t num_tables = ttf_u16( ttf + 4 );
    const uint8_t *table = ttf + 12;

    for ( uint32_t itbl = 0; itbl < num_tables; ++itbl ) {
        if ( check_tag( table, tag ) ) {
            uint32_t offset = ttf_u32( table + 8 );
            return ttf + offset;
        }
        table += 16;
    }

    return nullptr;
}



// Reading mappings from codepoint to glyph index

static bool fill_cmap( Font& font, const uint8_t *ttf ) {
    const uint8_t *cmap = find_table( ttf, "cmap" );
    if ( !cmap ) return false;

    uint32_t num_tables = ttf_u16( cmap + 2 );
    const uint8_t *imap = nullptr;
    
    for ( size_t itbl = 0; itbl < num_tables; ++itbl ) {
        const uint8_t *enc_table = cmap + 4 + 8 * itbl;
        uint16_t platform = ttf_u16( enc_table );
        uint16_t encoding = ttf_u16( enc_table + 2 );
        uint32_t offset   = ttf_u32( enc_table + 4 );

        if ( platform == 0 ) {     // Unicode
            imap = cmap + offset;
            break;
        }
        if ( platform == 3 ) {     // MS
            if ( encoding == 1 || encoding == 10 ) {
                imap = cmap + offset;
                break;
            }
        }
    }
    if ( !imap ) return false;

    uint16_t format = ttf_u16( imap );

    if ( format == 0 ) {
        const uint8_t *idx_data = imap + 6;
        for ( uint32_t i = 1; i < 256; ++i ) {
            int idx = (int) ( idx_data[i] );
            font.glyph_map.insert( { i, idx } );
        }
        return true;
        
    } else if ( format == 4 ) {
        uint32_t  seg_count = ttf_u16( imap + 6 ) >> 1;
        const uint8_t  *end_code = imap + 7 * 2;
        const uint8_t  *start_code = end_code + 2 + seg_count * 2;
        const uint8_t  *offset = imap + 8 * 2 + seg_count * 2 * 3;
        const uint8_t  *delta = imap + 8 * 2 + seg_count * 2 * 2;
        
        for ( uint32_t iseg = 0; iseg < seg_count; iseg++ ) {
            uint32_t seg_start = ttf_u16( start_code + iseg * 2 );
            uint32_t seg_end = ttf_u16( end_code + iseg * 2 );
            uint32_t seg_offset = ttf_u16( offset + iseg * 2 );
            int32_t  seg_delta = ttf_i16( delta + iseg * 2 );

            if ( seg_offset == 0 ) {
                for ( uint32_t cp = seg_start; cp <= seg_end; ++cp ) {
                    int32_t idx   = (uint16_t) ( cp + seg_delta );
                    font.glyph_map.insert( { cp, idx } );
                }
            } else {
                for ( uint32_t cp = seg_start; cp <= seg_end; ++cp ) {
                    uint32_t item = cp - seg_start;
                    int32_t idx = ttf_i16( offset + iseg * 2 + seg_offset + item * 2 );
                    font.glyph_map.insert( { cp, idx } );
                }
            }
        }
        return true;
        
    } else if ( format == 6 ) {
        uint32_t       first    = ttf_u16( imap + 6 );
        uint32_t       count    = ttf_u16( imap + 8 );
        const uint8_t *idx_data = imap + 10;

        for ( uint32_t i = 0; i < count; ++i ) {
            uint32_t idx = ttf_u16( idx_data + i * 2 );
            font.glyph_map.insert( { i + first, idx } );
        }
        return true;
        
    } else if ( format == 10 ) {
        uint32_t        first_char = ttf_u32( imap + 12 );
        uint32_t        num_chars  = ttf_u32( imap + 16 );
        const uint8_t  *idx_data = imap + 20;

        for ( uint32_t i = 0; i < num_chars; ++i ) {
            uint32_t idx = ttf_u16( idx_data + i * 2 );
            font.glyph_map.insert( { i + first_char, idx } );
        }
        return true;
        
    } else if ( format == 12 ) {
        uint32_t       ngroups = ttf_u32( imap + 12 );
        const uint8_t *sm_group = imap + 16;

        for ( uint32_t i = 0; i < ngroups; ++i ) {
            uint32_t start_code = ttf_u32( sm_group );
            uint32_t end_code = ttf_u32( sm_group + 4 );
            uint32_t start_idx = ttf_u32( sm_group + 8 );

            for ( uint32_t icode = start_code; icode <= end_code; ++icode ) {
                uint32_t idx = start_idx + icode - start_code;
                font.glyph_map.insert( { icode, idx } );
            }

            sm_group += 12;
        }
        return true;
        
    } else if ( format == 13 ) {
        uint32_t       ngroups = ttf_u32( imap + 12 );
        const uint8_t *sm_group = imap + 16;

        for ( uint32_t i = 0; i < ngroups; ++i ) {
            uint32_t start_code = ttf_u32( sm_group );
            uint32_t end_code = ttf_u32( sm_group + 4 );
            uint32_t glyph_idx = ttf_u32( sm_group + 8 );
            
            for ( uint32_t icode = start_code; icode <= end_code; ++icode ) {
                font.glyph_map.insert( { icode, glyph_idx } );
            }

            sm_group += 12;
        }
        return true;
    }
    
    return false;
}



// Glyph offset in 'glyf' table

inline int glyph_loc_offset( int glyph_idx, bool is_loc32, const uint8_t *loca ) {
    uint32_t off0, off1;
    if ( is_loc32 ) {
        off0 = ttf_u32( loca + glyph_idx * 4 );
        off1 = ttf_u32( loca + glyph_idx * 4 + 4 );
    } else {
        off0 = ttf_u16( loca + glyph_idx * 2 ) * 2;
        off1 = ttf_u16( loca + glyph_idx * 2 + 2 ) * 2;
    }
    if ( off0 == off1 ) return -1;
    else return off0;
}



// Display list for simple (non composite) glyph

static void glyph_shape_simple( Glyph& glyph, std::vector<GlyphCommand>& commands, const uint8_t *glyph_loc, float scale ) {
    int num_contours = ttf_i16( glyph_loc );

    if ( num_contours < 0 ) return;

    // Indices for the last point of each countour
    const uint8_t *end_pts = glyph_loc + 10;
    // Size of the byte code instructions, skipping this
    size_t   icount = ttf_u16( end_pts + num_contours * 2 );
    // Number of control points
    size_t   num_pts = ttf_u16( end_pts + num_contours * 2 - 2 ) + 1;

    const uint8_t *flag_array = end_pts + num_contours * 2 + 2 + icount;

    glyph.command_start = commands.size();

    const uint8_t *fpos   = flag_array;
    int            fcount = num_pts;
    size_t         xbytes = 0;

    // Calculating offsets of point coordinate tables 
    while ( fcount > 0 ) {
        uint8_t flag    = fpos[0];
        uint8_t frepeat = fpos[1];
        size_t  xsize = ( flag & 0x02 ) ?  1  :  ( flag & 0x10 ) ? 0 : 2;

        if ( flag & 0x08 ) {
            fcount -= frepeat + 1;
            fpos += 2;
            xbytes += xsize * ( frepeat + 1 );
        } else {
            fcount--;
            fpos++;
            xbytes += xsize;
        }
    }
    
    const uint8_t *xcoord = fpos;
    const uint8_t *ycoord = xcoord + xbytes;

    // Flag bits:
    // 0x01 - on-curve, ~0x01 - off-curve
    // Two consecutive off-curve points assume on-curve point between them    
    //
    // 0x02 - x-coord is 8-bit unsigned integer
    //       0x10 - positive, ~0x10 - negative
    // ~0x02 - x-coord is 16-bit signed integer
    // ~0x02 & 0x10 - x-coord equals x-coord of the previous point
    // 
    // 0x04 - y-coord is 8-bit unsigned integer
    //       0x20 - positive, ~0x20 - negative
    // ~0x04 - y-coord is 16-bit signed integer
    // ~0x04 & 0x20 - y-coord equals y-coord of the previous point
    //
    // 0x08 - repeat flag N times, read next byte for N

    // Current contour point coordinates
    F2 cur_pos { 0.0f };
    // Previous contour point coordinates
    F2 prev_pos { 0.0f };

    bool prev_on_curve = true;  // previous point was on-curve
    bool on_curve = true;       // current point is on-curve
    
    size_t  iflag = 0; // next flag index
    uint8_t flag  = 0; // current flag value

    size_t gc_contour_start_idx = 0;          // Index of the first control point of the contour
    bool   contour_starts_off_curve = false;
    bool   new_contour   = true;              // Current command starts new contour

    size_t  icontour = 0; // Next contour starting index

    GlyphCommand command;
    command.type = GlyphCommand::MoveTo;
    command.p0 = F2{ 0.0f };
    command.p1 = F2{ 0.0f };

    // Filling glyph display list

    for ( size_t ipoint = 0; ipoint < num_pts; ++ipoint ) {
        if ( ipoint == iflag ) {
            flag = flag_array[0];
            size_t frepeat = flag_array[1];

            if ( flag & 0x08 ) {
                // Repeat flag
                iflag = ipoint + frepeat + 1;
                flag_array += 2;
            } else {
                // Do not repeat flag
                iflag = ipoint + 1;
                flag_array++;
            }
        }

        prev_on_curve = on_curve;
        on_curve = flag & 0x01;

        prev_pos = cur_pos;

        if ( flag & 0x02 ) {
            // X-coord is 8 bit value
            float dx = xcoord[0];
            cur_pos.x += ( flag & 0x10 ) ? dx : -dx; // X-coord sign
            xcoord++;
        } else {
            if ( !( flag & 0x10 ) ) {
                // X-coord is 16 bit value
                cur_pos.x += ttf_i16( xcoord );
                xcoord += 2;
            }
        }

        if ( flag & 0x04 ) {
            // Y-coord is 8-bit value
            float dy = ycoord[0];
            cur_pos.y += ( flag & 0x20 ) ? dy : -dy; // Y-coord sign
            ycoord++;
        } else {
            if ( !( flag & 0x20 ) ) {
                // Y-coord is 16-bit value
                cur_pos.y += ttf_i16( ycoord );
                ycoord += 2;
            }
        }

        if ( new_contour ) {
            // Push MoveTo command if starting new contour
            contour_starts_off_curve = !on_curve;
            gc_contour_start_idx = commands.size();
            command.type = GlyphCommand::MoveTo;
            command.p0 = scale * cur_pos;
            command.p1 = F2{ 0.0f };

            commands.push_back( command );
            
            icontour = ttf_u16( end_pts );
            end_pts += 2;
            new_contour = false;
        } else {
            if ( on_curve ) {
                if ( prev_on_curve ) {
                    // Normal (non smooth) control point, pushing LineTo
                    command.p0 = scale * cur_pos;
                    command.p1 = F2{ 0.0f };
                    command.type = GlyphCommand::LineTo;
                    commands.push_back( command );
                } else {
                    // Normal control point, pushing BezTo
                    command.p0 = scale * prev_pos;
                    command.p1 = scale * cur_pos;
                    command.type = GlyphCommand::BezTo;
                    commands.push_back( command );
                }
            } else {
                if ( !prev_on_curve ) {
                    // Smooth curve, inserting control point in the middle
                    F2 mid_cp = 0.5f * ( prev_pos + cur_pos );
                    command.p0 = scale * prev_pos;
                    command.p1 = scale * mid_cp;
                    command.type = GlyphCommand::BezTo;
                    commands.push_back( command );
                }
            }
        }

        // Closing contour
        if ( icontour == ipoint && ipoint > 0 ) {
            if ( contour_starts_off_curve ) {
                if ( on_curve ) {
                    // Contour starts off-curve, contour start to current point
                    commands[ gc_contour_start_idx ].p0 = scale * cur_pos;
                } else {
                    // Contour starts and ends off-curve,
                    // calculating contour starting point, setting first MoveTo P0,
                    // and closing contour with BezTo
                    
                    F2 cpos = scale * cur_pos;
                    F2 next_cp = commands[ gc_contour_start_idx + 1 ].p0; // First BezTo off-curve CP
                    F2 pos = 0.5f * ( cpos + next_cp );                   // Contour start point
                    commands[ gc_contour_start_idx ].p0 = pos;

                    command.p0 = cpos;
                    command.p1 = pos;
                    command.type = GlyphCommand::BezTo;
                    commands.push_back( command );
                }
            } else {
                if ( !on_curve ) {
                    // Contour ends off-curve, closing contour with BezTo to contour starting point
                    
                    F2 start_pos = commands[ gc_contour_start_idx ].p0;

                    command.p0 = scale * cur_pos;
                    command.p1 = start_pos;
                    command.type = GlyphCommand::BezTo;
                    commands.push_back( command );
                }
            }
            // Pushing ClosePath command
            command.type = GlyphCommand::ClosePath;
            command.p0 = F2{ 0.0f };
            command.p1 = F2{ 0.0f };
            commands.push_back( command );
            new_contour = true;
        }
    }

    glyph.command_count = commands.size() - glyph.command_start;
}


// Composite glyphs will have a display list of all their subglyphs combined with transformation applied

static void glyph_commands_composite( Font& font, int glyph_idx ) {
    Glyph &glyph = font.glyphs[ glyph_idx ];
    if ( !glyph.is_composite ) return;
    glyph.command_start = font.glyph_commands.size();
    glyph.command_count = 0;

    for ( int icomp = glyph.components_start; icomp < glyph.components_start + glyph.components_count; ++icomp ) {
        GlyphComponent& gcomp = font.glyph_components[ icomp ];
        const Glyph& cglyph = font.glyphs[ gcomp.glyph_idx ];
        const Mat2d& tr = gcomp.transform;

        for ( int icommand = cglyph.command_start; icommand < cglyph.command_start + cglyph.command_count; ++icommand ) {
            const GlyphCommand& gcommand = font.glyph_commands[ icommand ];
            GlyphCommand new_command;
            new_command.type = gcommand.type;
                
            switch ( gcommand.type ) {
            case GlyphCommand::MoveTo:
            case GlyphCommand::LineTo:
                new_command.p0 = tr * gcommand.p0;
                break;
            case GlyphCommand::BezTo:
                new_command.p0 = tr * gcommand.p0;
                new_command.p1 = tr * gcommand.p1;
                break;
            case GlyphCommand::ClosePath:                
                break;
            }
            font.glyph_commands.push_back( new_command );
        }
    }

    glyph.command_count = font.glyph_commands.size() - glyph.command_start;
}


// Reading glyph display list or subglyphs of a composite glyph.

static void glyph_shape( Font& font, int glyph_idx, bool is_loc32, const uint8_t *loca, const uint8_t *glyf, float scale ) {
    Glyph &glyph = font.glyphs[ glyph_idx ];

    int glyph_offset = glyph_loc_offset( glyph_idx, is_loc32, loca );
    if ( glyph_offset < 0 ) return;    
    
    const uint8_t *glyph_loc = glyf + glyph_offset;
    int num_contours = ttf_i16( glyph_loc );

    float minx = ttf_i16( glyph_loc + 2 );
    float miny = ttf_i16( glyph_loc + 4 );
    float maxx = ttf_i16( glyph_loc + 6 );
    float maxy = ttf_i16( glyph_loc + 8 );
    
    glyph.min = scale * F2{ minx, miny };
    glyph.max = scale * F2{ maxx, maxy };

    // Simple glyph
    if ( num_contours > 0 ) {
        glyph_shape_simple( glyph, font.glyph_commands, glyph_loc, scale );

    // Composite glyph
    } else if ( num_contours < 0 ) {
        glyph.is_composite = true;
        glyph.components_start = font.glyph_components.size();

        bool next_comp = true;
        const uint8_t *pos = glyph_loc + 10;

        while( next_comp ) {
            uint16_t flags = ttf_u16( pos );
            uint32_t comp_glyph_idx = ttf_u16( pos + 2 );
            pos += 4;
            
            Mat2d gtr { 1.0f };

            // Component position
            if ( flags & 2 ) {
                if ( flags & 1 ) {
                    gtr[2][0] = ttf_i16( pos ) * scale; pos += 2;
                    gtr[2][1] = ttf_i16( pos ) * scale; pos += 2;
                } else {
                    gtr[2][0] = ( (int8_t) *pos ) * scale; pos++;
                    gtr[2][1] = ( (int8_t) *pos ) * scale; pos++;
                }
            } else {
                assert( false );
            }

            // Component rotation and scale
            if ( flags & ( 1 << 3 ) ) {
                // Uniform scale
                gtr[0][0] = gtr[1][1] = ttf_i16( pos ) / 16384.0f; pos += 2;
            } else if ( flags & ( 1 << 6 ) ) {
                // XY-scale
                gtr[0][0] = ttf_i16( pos ) / 16384.0f; pos += 2;
                gtr[1][1] = ttf_i16( pos ) / 16384.0f; pos += 2;
            } else if ( flags & ( 1 << 7 ) ) {
                // Rotion matrix
                gtr[0][0] = ttf_i16( pos ) / 16384.0f; pos += 2;
                gtr[0][1] = ttf_i16( pos ) / 16384.0f; pos += 2;
                gtr[1][0] = ttf_i16( pos ) / 16384.0f; pos += 2;
                gtr[1][1] = ttf_i16( pos ) / 16384.0f; pos += 2;
            }

            GlyphComponent gc;
            gc.glyph_idx = comp_glyph_idx;
            gc.transform = gtr;
            font.glyph_components.push_back( gc );            

            // More components?
            next_comp = flags & ( 1 << 5 );
        }
        glyph.components_count = font.glyph_components.size() - glyph.components_start;
    }
}



// Reading kerning table

static bool fill_kern( Font& font, const uint8_t *ttf, float scale ) {
    const uint8_t *kern = find_table( ttf, "kern" );
    if ( !kern ) return false;

    uint16_t  num_tables = ttf_u16( kern + 2 );
    const uint8_t  *table = nullptr;
    const uint8_t  *pos = kern + 4;

    for ( size_t itbl = 0; itbl < num_tables; ++itbl ) {
        uint16_t length = ttf_u16( pos + 2 );
        uint16_t coverage = ttf_u16( pos + 4 );
        
        if ( coverage == 1 ) {
            table = pos;
            break;
        }
        pos += length;
    }

    if ( !table ) return false;

    uint32_t num_pairs = ttf_u16( table + 6 );
    pos = table + 14;
    
    for ( uint32_t ipair = 0; ipair < num_pairs; ++ipair ) {
        uint32_t left  = ttf_u16( pos );
        uint32_t right = ttf_u16( pos + 2 );
        int32_t  kern  = ttf_i16( pos + 4 );
        uint32_t pair = ( left << 16 ) | right;
        font.kern_map.insert( { pair, kern * scale } );
        pos += 6;
    }

    return true;
}

bool Font::load_ttf_file( const char *filename ) {
    FILE *f = fopen( filename, "rb" );
    if ( !f ) return false;
    
    fseek( f, 0, SEEK_END );
    size_t fsize = ftell( f );
    fseek( f, 0, SEEK_SET );

    uint8_t *ttf = (unsigned char*) malloc( fsize );
    fread( ttf, 1, fsize, f );
    fclose( f );

    bool res = load_ttf_mem( ttf );
    free( ttf );
    return res;
}


bool Font::load_ttf_mem( const uint8_t *ttf ) {
    if ( ttf == nullptr ) return false;
    if ( !is_font( ttf ) ) return false;

    uint32_t num_glyphs = 0xffff;

    const uint8_t *head = find_table( ttf, "head" );
    if ( !head ) return false;

    uint16_t loc_format = ttf_u16( head + 50 );
    // 0 - 16 bit offset
    // 1 - 32 bit offset
    // >1 - unsupported
    if ( loc_format > 1 ) return false;
    bool is_loc32 = loc_format;

    const uint8_t *loca = find_table( ttf, "loca" );
    if ( !loca ) return false;

    const uint8_t *hmtx = find_table( ttf, "hmtx" );
    if ( !hmtx ) return false;

    const uint8_t *glyf = find_table( ttf, "glyf" );
    if ( !glyf ) return false;

    const uint8_t *maxp = find_table( ttf, "maxp" );
    if ( maxp ) num_glyphs = ttf_u16( maxp + 4 );

    const uint8_t *hhea = find_table( ttf, "hhea" );
    if ( !hhea ) return false;
    em_ascent  = ttf_i16( hhea + 4 );
    em_descent = ttf_i16( hhea + 6 );
    em_line_gap = ttf_i16( hhea + 8 );

    uint32_t num_hmtx = ttf_u16( hhea + 34 );
    
    float scale = 1.0f / em_ascent;
    ascent   = 1.0;
    descent  = em_descent * scale;
    line_gap = em_line_gap * scale;

    // Filling glyph idx mappings
    if ( !fill_cmap( *this, ttf ) ) return false;

    glyphs = std::vector<Glyph>( num_glyphs, Glyph{} );

    // These glyphs have both advance with and left side bearing in "hmtx" table
    for ( size_t iglyph = 0; iglyph < num_hmtx; ++iglyph ) {
        glyphs[ iglyph ].advance_width     = ttf_u16( hmtx + iglyph * 4 ) * scale;
        glyphs[ iglyph ].left_side_bearing = ttf_i16( hmtx + iglyph * 4 + 2 ) * scale;
    }
    // Rest of glyphs have left side bearing only
    for ( size_t iglyph = 0; iglyph < ( num_glyphs - num_hmtx ); ++iglyph ) {
        const uint8_t *pos = hmtx + num_hmtx * 4 + iglyph * 2;
        glyphs[ iglyph + num_hmtx ].advance_width = 0.0f;
        glyphs[ iglyph + num_hmtx ].left_side_bearing = ttf_i16( pos );
    }

    // Reading glyph display lists while calculating glyph max bounding box

    glyph_min = F2 { 2e38f };
    glyph_max = F2 { -2e38f };

    // Reading simple glyph display listd and components for composite glyphs
    for ( size_t iglyph = 0; iglyph < num_glyphs; ++iglyph ) {
        glyph_shape( *this, iglyph, is_loc32, loca, glyf, scale );
        glyph_min = min( glyph_min, glyphs[iglyph].min );
        glyph_max = max( glyph_max, glyphs[iglyph].max );        
    }

    // Calculating composite glyph commands
    for ( size_t iglyph = 0; iglyph < num_glyphs; ++iglyph ) {
        glyph_commands_composite( *this, iglyph );
    }

    // Reading glyph types
    for ( const std::pair<uint32_t, int>& cgpair : glyph_map ) {
        uint32_t codepoint = cgpair.first;
        int iglyph = cgpair.second;
        if ( iglyph < 0 ) continue;
        Glyph& g = glyphs[ iglyph ];
        if ( iswlower( codepoint ) ) g.char_type = Glyph::Lower;
        if ( iswupper( codepoint ) | iswdigit( codepoint ) ) g.char_type = Glyph::Upper;
        if ( iswpunct( codepoint ) ) g.char_type = Glyph::Punct;
        if ( iswspace( codepoint ) ) g.char_type = Glyph::Space;
    }

    // Some fonts store kerning information in "kern" table, reading it
    fill_kern( *this, ttf, scale );

    // TODO Other fonts store kerning information in "gpos" table
        
    return true;    
}
