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

#include "glyph_painter.h"

#include "parabola.h"

#include <cstdlib>
#include <cstdio>
#include <cassert>

#include <iostream>

static void fill_triangle( F2 p0, F2 p1, F2 p2, std::vector<SdfVertex>* vertices ) {
    SdfVertex v0, v1, v2;
    v0 = { p0, F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f };
    v1 = { p1, F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f };
    v2 = { p2, F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f };

    vertices->push_back( v0 );
    vertices->push_back( v1 );
    vertices->push_back( v2 );
}

void FillPainter::move_to( F2 p0 ) {
    fan_pos = p0;
    prev_pos = p0;
}

void FillPainter::line_to( F2 p1 ) {
    fill_triangle( fan_pos, prev_pos, p1, &vertices );
    
    prev_pos = p1;
}

void FillPainter::qbez_to( F2 p1, F2 p2 ) {
    SdfVertex v0, v1, v2, v3, v4, v5;

    fill_triangle( fan_pos, prev_pos, p2, &vertices );
    
    v0 = { prev_pos,  F2( -1.0f,  1.0f ), F2( 0.0f ), 0.0f, 0.0f };
    v1 = { p1,        F2(  0.0f, -1.0f ), F2( 0.0f ), 0.0f, 0.0f };
    v2 = { p2,        F2(  1.0f,  1.0f ), F2( 0.0f ), 0.0f, 0.0f };

    vertices.push_back( v0 );
    vertices.push_back( v1 );
    vertices.push_back( v2 );
    
    prev_pos = p2;
}

void FillPainter::close() {
    if ( sqr_length( fan_pos - prev_pos ) < 1e-7 ) return;
    line_to( fan_pos );
}

void LinePainter::move_to( F2 p0 ) {
    prev_pos = p0;
    start_pos = p0;
}

static void set_par_vertex( SdfVertex *v, const Parabola &par ) {
    F2 par_pos = par.world_to_par( v->pos );
    v->par = par_pos;
    v->limits = F2( par.xstart, par.xend );
    v->scale = par.scale;
}

static void line_rect( const Parabola &par, F2 vmin, F2 vmax, float line_width, std::vector<SdfVertex> *vertices ) {
    SdfVertex v0, v1, v2, v3;
    v0.pos = F2( vmin.x, vmin.y );
    v1.pos = F2( vmax.x, vmin.y );
    v2.pos = F2( vmax.x, vmax.y );
    v3.pos = F2( vmin.x, vmax.y );

    v0.line_width = line_width;
    v1.line_width = line_width;
    v2.line_width = line_width;
    v3.line_width = line_width;

    set_par_vertex( &v0, par );
    set_par_vertex( &v1, par );
    set_par_vertex( &v2, par );
    set_par_vertex( &v3, par );

    vertices->push_back( v0 );
    vertices->push_back( v1 );
    vertices->push_back( v2 );

    vertices->push_back( v0 );
    vertices->push_back( v2 );
    vertices->push_back( v3 );
}

void LinePainter::line_to( F2 p1, float line_width ) {
    F2 vmin = min( prev_pos, p1 );
    F2 vmax = max( prev_pos, p1 );

    vmin -= F2( line_width );
    vmax += F2( line_width );

    Parabola par = Parabola::from_line( prev_pos, p1 );
    line_rect( par, vmin, vmax, line_width, &vertices );
    
    prev_pos = p1;
}

void LinePainter::qbez_to( F2 p1, F2 p2, float line_width ) {
    F2 p0 = prev_pos;
    
    F2 mid01 = F2( 0.5 ) * ( p0 + p1 );
    F2 mid12 = F2( 0.5 ) * ( p1 + p2 );

    F2 vmin = min( p0, mid01 );
    vmin = min( vmin, mid12 );
    vmin = min( vmin, p2 );

    F2 vmax = max( p0, mid01 );
    vmax = max( vmax, mid12 );
    vmax = max( vmax, p2 );

    vmin -= F2( line_width );
    vmax += F2( line_width );

    F2 v10 = p0 - p1;
    F2 v12 = p2 - p1;
    F2 np10 = normalize( v10 );
    F2 np12 = normalize( v12 );
    
    QbezType qtype = qbez_type( np10, np12 );
    Parabola par;
    
    switch ( qtype ) {
    case QbezType::Parabola:
        par = Parabola::from_qbez( p0, p1, p2 );
        line_rect( par, vmin, vmax, line_width, &vertices );
        break;
    case QbezType::Line:
        par = Parabola::from_line( p0, p2 );
        line_rect( par, vmin, vmax, line_width, &vertices );
        break;
    case QbezType::TwoLines: {
        float l10 = length( v10 );
        float l12 = length( v12 );
        float qt = l10 / ( l10 + l12 );
        float nqt = 1.0f - qt;
        F2 qtop = p0 * ( nqt * nqt ) + p1 * ( 2.0f * nqt * qt ) + p2 * ( qt * qt );
        Parabola par0 = Parabola::from_line( p0, qtop );
        line_rect( par0, vmin, vmax, line_width, &vertices );
        Parabola par1 = Parabola::from_line( qtop, p1 );
        line_rect( par1, vmin, vmax, line_width, &vertices ); 
        break;
    }
    }

    prev_pos = p2;
}


void LinePainter::close( float line_width ) {
    if ( sqr_length( start_pos - prev_pos ) < 1e-7 ) return;
    line_to( start_pos, line_width );
}



void GlyphPainter::draw_glyph( const Font *font, int glyph_index, F2 pos, float scale, float sdf_size ) {
    const Glyph& g = font->glyphs[ glyph_index ];
    if ( g.command_count == 0 ) return;

    for ( int ic = g.command_start; ic < g.command_start + g.command_count; ++ic ) {
        const GlyphCommand& gc = font->glyph_commands[ ic ];
        F2 p0, p1;

        switch ( gc.type ) {
        case GlyphCommand::MoveTo:
            p0 = gc.p0 * scale + pos;
            fp.move_to( p0 );
            lp.move_to( p0 );
            break;
        case GlyphCommand::LineTo:
            p0 = gc.p0 * scale + pos;
            fp.line_to( p0 );
            lp.line_to( p0, sdf_size );
            break;
        case GlyphCommand::BezTo:
            p0 = gc.p0 * scale + pos;
            p1 = gc.p1 * scale + pos;            
            fp.qbez_to( p0, p1 );
            lp.qbez_to( p0, p1, sdf_size );
            break;
        case GlyphCommand::ClosePath:
            fp.close();
            lp.close( sdf_size );
            break;
        }
    }
}
