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

#include "sdf_gl.h"

#include "shaders/shape_vsh.cpp"
#include "shaders/shape_fsh.cpp"

#include "shaders/line_vsh.cpp"
#include "shaders/line_fsh.cpp"


VertexAttrib vattribs[] = {
    VertexAttrib( 0, "pos", 2 ),
    VertexAttrib( 1, "par", 2 ),
    VertexAttrib( 2, "limits", 2 ),
    VertexAttrib( 3, "scale", 1 ),
    VertexAttrib( 4, "line_width", 1 )
};

constexpr size_t vattribs_count = sizeof( vattribs ) / sizeof( vattribs[0] );

void SdfGl::init() {
    initVertexAttribs( vattribs, vattribs_count );
    fill_prog = createProgram( "fill", shape_vsh, shape_fsh, vattribs, vattribs_count );
    initUniformStruct( fill_prog, ufill );

    line_prog = createProgram( "line", line_vsh, line_fsh, vattribs, vattribs_count );
    initUniformStruct( line_prog, uline );
}

void SdfGl::render_sdf( F2 tex_size, const std::vector<SdfVertex> &fill_vertices, const std::vector<SdfVertex> &line_vertices ) {

    // full screen quad vertices    
    SdfVertex fs_quad[6] = {
        { F2( -1.0, -1.0 ), F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f },
        { F2(  1.0, -1.0 ), F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f },
        { F2(  1.0,  1.0 ), F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f },
        
        { F2( -1.0, -1.0 ), F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f },
        { F2(  1.0,  1.0 ), F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f },
        { F2( -1.0,  1.0 ), F2( 0.0f, 1.0f ), F2( 0.0f ), 0.0f, 0.0f }
    };

    size_t lcount = line_vertices.size();
    size_t fcount = fill_vertices.size();

    // screen matrix
    float mscreen3[] = {
          2.0f / tex_size.x, 0, 0,
          0, 2.0f / tex_size.y, 0,
          -1, -1, 1 };

    // identity matrix
    float mid[] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    glViewport( 0, 0, tex_size.x, tex_size.y );    

    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // Drawing lines with depth test

    if ( line_vertices.size() ) {
    
        bindAttribs( vattribs, vattribs_count, (size_t) line_vertices.data() );

        glUseProgram( line_prog );
        uline.transform_matrix.setv( mscreen3 );
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );
        glDrawArrays( GL_TRIANGLES, 0, lcount );    
        glDisable( GL_DEPTH_TEST );

    }

    // Drawing fills

    if ( fill_vertices.size() ) {
    
        bindAttribs( vattribs, vattribs_count, (size_t) fill_vertices.data() );
    
        glUseProgram( fill_prog );
        ufill.transform_matrix.setv( mscreen3 );
    
        glEnable( GL_STENCIL_TEST );
        glEnable( GL_CULL_FACE );
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

        glStencilFunc( GL_ALWAYS, 0, 0xff );

        // Front face (CCW) increases stencil values
        glCullFace( GL_FRONT );
        glStencilOp( GL_KEEP, GL_INCR, GL_INCR );
        glDrawArrays( GL_TRIANGLES, 0, fcount );

        // Back face (CW) decreaces
        glCullFace( GL_BACK );
        glStencilOp( GL_KEEP, GL_DECR, GL_DECR );
        glDrawArrays( GL_TRIANGLES, 0, fcount );

        glDisable( GL_CULL_FACE );    

        // Drawing full screen quad, inverting colors where stencil == 1

        bindAttribs( vattribs, vattribs_count, (size_t) fs_quad );

        glEnable( GL_BLEND );
        glBlendEquation( GL_FUNC_ADD );
        glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
        glStencilFunc( GL_EQUAL, 1, 0xff );
        glStencilOp( GL_ZERO, GL_ZERO, GL_ZERO );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        ufill.transform_matrix.setv( mid );
        glDrawArrays( GL_TRIANGLES, 0, 6 );

    }

    glDisable( GL_BLEND );
    glDisable( GL_STENCIL_TEST );
    
    glUseProgram( 0 );
}
