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

#include <GL/glew.h>

enum ShaderType {
	VertexShader = GL_VERTEX_SHADER, FragmentShader = GL_FRAGMENT_SHADER
};

struct VertexAttribType {
    GLuint gl_type;
    GLuint size;
};

namespace vatypes {
    const VertexAttribType gl_byte   = { GL_BYTE, 1 };
    const VertexAttribType gl_ubyte  = { GL_UNSIGNED_BYTE, 1 };
    const VertexAttribType gl_short  = { GL_SHORT, 2 };
    const VertexAttribType gl_ushort = { GL_UNSIGNED_SHORT, 2 };
    const VertexAttribType gl_int    = { GL_INT, 4 };
    const VertexAttribType gl_uint   = { GL_UNSIGNED_INT, 4};
    const VertexAttribType gl_float  = { GL_FLOAT, 4 };
    const VertexAttribType gl_fixed  = { GL_FIXED, 4 };
}

struct VertexAttrib {
    GLuint location;
    const char* name;
    GLuint size;
    VertexAttribType type;
    bool normalize;
    GLuint stride;
    GLvoid *offset;

    VertexAttrib(GLuint location, 
                 const char *name,
                 GLuint size = 4,
                 VertexAttribType type = vatypes::gl_float, 
                 bool normalize = false,
                 GLvoid *offset = nullptr) :
        location(location), name(name), size(size), type(type),
        normalize(normalize), offset(offset) {} 
};

struct Uniform {
    char* name;
    GLuint program_id;
    GLint location;

    Uniform(const char *name) : name((char*)name), program_id(0), location(-1) {}
    Uniform( GLuint program_id, GLint location ) :
        name(nullptr), program_id( program_id ), location( location )
    {}
};

#define UNIFORM( type, name ) Uniform##type name { #name };

#define UNIFORM_MATRIX( size, name ) UniformMatrix##size name { #name };


using ProgramAction = void(*)( GLuint );

GLuint createVertexBuffer( GLenum usage, size_t size, const void *data = nullptr );

GLuint compileShader( const char *name, const char *source, ShaderType type = VertexShader );

bool linkProgram( GLuint program_id );

GLuint createProgram( const char* name, const char* vertex_shader, const char* fragment_shader, VertexAttrib *attribs = nullptr, size_t attrib_count = 0, ProgramAction before_link = 0 );

void deleteProgram( GLuint program );

void initUniforms( GLuint program_id, Uniform *uniform, size_t count = 1 );

template <class T>
void initUniformStruct( GLuint program_id, T& unf_struct ) {
    initUniforms( program_id, (Uniform*) &unf_struct, sizeof(T) / sizeof( Uniform ) );
}

GLuint vertexAttribsStride( VertexAttrib *attribs, size_t attrib_count );

void initVertexAttribs( VertexAttrib *attribs, size_t attrib_count, GLvoid *offset = nullptr, GLuint stride = 0 );

void bindAttribs( VertexAttrib *attribs, size_t attrib_count, size_t offset = 0 );

struct Uniform1i : Uniform {
    Uniform1i(const char* name) : Uniform(name) {};

    void set(int v0) const {
        glUniform1i(location, v0);
    }
};

struct Uniform1f : Uniform {
    using Uniform::Uniform;

    void set(float v0) const {
        glUniform1f(location, v0);
    }
    
    void setv(const float *v, GLsizei count = 1) const {
        glUniform1fv(location, count, v);
    }
};

struct Uniform2f : Uniform {
    using Uniform::Uniform;
    
    void set(float v0, float v1) const {
        glUniform2f(location, v0, v1);
    }
    
    void setv(const float *v, GLsizei count = 1) const {
        glUniform2fv(location, count, v);
    }
};

struct Uniform3f : Uniform {
    using Uniform::Uniform;
    
    void set(float v0, float v1, float v2) const {
        glUniform3f(location, v0, v1, v2);
    }

    void setv(const float *v, GLsizei count = 1) const {
        glUniform3fv(location, count, v);
    }
};

struct Uniform4f : Uniform {
    using Uniform::Uniform;
    
    void set(float v0, float v1, float v2, float v3) const {
        glUniform4f(location, v0, v1, v2, v3);
    }

    void setv(float *v, GLsizei count = 1) const {
        glUniform4fv(location, count, v);
    }
};

struct UniformMatrix2 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix2fv(location, count, transpose, v);
    }
};

struct UniformMatrix3 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix3fv(location, count, transpose, v);
    }
};

struct UniformMatrix4 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix4fv(location, count, transpose, v);
    }
};

struct UniformMatrix2x3 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix2x3fv(location, count, transpose, v);
    }
};

struct UniformMatrix3x2 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix3x2fv(location, count, transpose, v);
    }
};

struct UniformMatrix2x4 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix2x4fv(location, count, transpose, v);
    }
};

struct UniformMatrix4x2 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix4x2fv(location, count, transpose, v);
    }
};

struct UniformMatrix3x4 : Uniform {
    using Uniform::Uniform;
    
    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix3x4fv(location, count, transpose, v);
    }
};

struct UniformMatrix4x3 : Uniform {
    using Uniform::Uniform;

    void setv(const float *v, GLsizei count = 1, GLboolean transpose = false) const {
        glUniformMatrix4x3fv(location, count, transpose, v);
    }
};
