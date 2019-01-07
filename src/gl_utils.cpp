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

#include "gl_utils.h"
#include <cstdlib>
#include <cstdio>
#include <cassert>

GLuint createVertexBuffer( GLenum usage, size_t size, const void *data ) {
    GLuint id;
    glGenBuffers( 1, &id );
    glBindBuffer( GL_ARRAY_BUFFER, id );
    glBufferData( GL_ARRAY_BUFFER, size, data, usage );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    return id;
}


GLuint compileShader(const char *name, const char *source, ShaderType type) {
	GLuint sid = glCreateShader(type);
	if(sid == 0) return 0;

	glShaderSource(sid, 1, &source, NULL);
	glCompileShader(sid);

	GLint scompiled;
	glGetShaderiv(sid, GL_COMPILE_STATUS, &scompiled);

	if(!scompiled) {
		GLint infoLen = 0;
		glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1) {
			char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			glGetShaderInfoLog(sid, infoLen, NULL, infoLog);

            switch (type) {
                case VertexShader :
                    fprintf(stderr, "Error compiling vertex shader '%s':\n%s\n", name, infoLog);
                    assert( false );
                    break;
                case FragmentShader :
                    fprintf(stderr, "Error compiling fragment shader '%s':\n%s\n", name, infoLog);
                    assert( false );
                    break;
                default:
                    fprintf(stderr, "Error compiling shader '%s':\n%s\n", name, infoLog);
                    assert( false );
                    break;
            }

			free(infoLog);
		}

		glDeleteShader(sid);
		return false;
    }

	return sid;
}


bool linkProgram(GLuint program_id)  {
	GLint linked;
	glLinkProgram(program_id);
	glGetProgramiv(program_id, GL_LINK_STATUS, &linked);

	if(!linked)	{
		GLint infoLen = 0;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1) {
			char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			glGetProgramInfoLog(program_id, infoLen, NULL, infoLog);
            fprintf(stderr, "Error linking program:\n%s\n",  infoLog);
			free(infoLog);
		}
		glDeleteProgram(program_id);
		return false;
	}
	return true;
}



GLuint createProgram(const char* name, const char* vertex_shader, const char* fragment_shader, VertexAttrib *attribs, size_t attrib_count, ProgramAction before_link) {
    GLuint vs_id = compileShader(name, vertex_shader, VertexShader);
    if (!vs_id) return 0;

    GLuint fs_id = compileShader(name, fragment_shader, FragmentShader);
    if (!fs_id) return 0;

    GLuint id =  glCreateProgram();
    if (!id) return 0;

    glAttachShader(id, vs_id);
    glAttachShader(id, fs_id);

    for (size_t i = 0; i < attrib_count; i++ ) {
        glBindAttribLocation(id, attribs[i].location, attribs[i].name);    
    }

    if ( before_link ) before_link( id );

    bool linked = linkProgram(id);
    if (!linked) return 0;
    
    return id;
}


void deleteProgram( GLuint program ) {
    GLuint shaders[16];
    GLsizei count = 0;
    glGetAttachedShaders( program, 16, &count, shaders );
    glDeleteProgram( program );
    
    for ( GLsizei i = 0; i < count; ++i ) {
        glDeleteShader( shaders[i] );
    }
}

void initUniforms(GLuint program_id, Uniform *uniforms, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        Uniform* u = uniforms + i;
        GLint location = glGetUniformLocation(program_id, u->name);
        u->program_id = program_id;
        u->location = location;
    }
}

GLuint vertexAttribsStride(VertexAttrib *attribs, size_t attrib_count) {
    GLuint size = 0;
    for (size_t i = 0; i < attrib_count; ++i) {
        size += attribs[i].type.size * attribs[i].size;
    }

    return size;
}

    
void initVertexAttribs(VertexAttrib *attribs, size_t attrib_count, GLvoid *offset, GLuint stride) {
    GLuint new_stride = stride ? stride : vertexAttribsStride(attribs, attrib_count);
    char *voffset = (char*)offset;

    for (size_t i = 0; i < attrib_count; ++i) {
        VertexAttrib *va = attribs + i;
        va->stride = new_stride;
        va->offset = voffset;
        voffset += va->size * va->type.size;
    }
}


void bindAttribs( VertexAttrib *attribs, size_t attrib_count, size_t offset ) {
    for (size_t i = 0; i < attrib_count; ++i) {
        VertexAttrib *va = attribs + i;
        glVertexAttribPointer(va->location, va->size, va->type.gl_type, va->normalize, va->stride, (void*)((size_t)va->offset + offset));
        glEnableVertexAttribArray(va->location);
    }
}
