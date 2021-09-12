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
#include <cmath>

struct Float2 {

    float x, y;

    Float2() {}

    Float2( float a ) :
        x(a), y(a) {}

    explicit Float2( float* a ) :
        x(a[0]), y(a[1]) {}

    Float2( float x, float y ) :
        x(x), y(y) {}

    Float2( const Float2& other ) :
        x( other.x ), y( other.y ) {}

    float* ptr() {
        return &x;
    }

    const float* ptr() const {
        return &x;
    }

    float& operator[] ( size_t index ) {
        return (&x)[index];        
    }

    const float& operator[] ( size_t index ) const {
        return (&x)[index];
    }

    Float2& operator- () {
        x = -x;
        y = -y;
        return *this;
    }

    Float2& operator= ( const Float2& other ) {
        x = other.x;
        y = other.y;
        return *this;
    }

    Float2& operator+= ( const Float2& other ) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Float2& operator-= ( const Float2& other ) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Float2& operator*= ( const Float2& other ) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Float2& operator/= ( const Float2& other ) {
        x /= other.x;
        y /= other.y;
        return *this;
    }
};


using F2 = Float2;


inline Float2 operator* ( const Float2& lv, const Float2& rv ) {
    return Float2( lv.x * rv.x, lv.y * rv.y );
}

inline Float2 operator* ( const Float2& lv, float rv ) {
    return lv * Float2( rv );
}

inline Float2 operator* ( float lv, const Float2& rv ) {
    return Float2( lv ) * rv;
}



inline Float2 operator/ ( const Float2& lv, const Float2& rv ) {
    return Float2( lv.x / rv.x, lv.y / rv.y );
}

inline Float2 operator/ ( const Float2& lv, float rv ) {
    return lv / Float2( rv );
}

inline Float2 operator/ (  float lv, const Float2& rv ) {
    return Float2( lv ) / rv;
}



inline Float2 operator+ ( const Float2& lv, const Float2& rv ) {
    return Float2( lv.x + rv.x, lv.y + rv.y );
}

inline Float2 operator+ ( const Float2& lv, float rv ) {
    return lv + Float2( rv );
}

inline Float2 operator+ (  float lv, const Float2& rv ) {
    return Float2( lv ) + rv;
}


inline Float2 operator- ( const Float2& lv, const Float2& rv ) {
    return Float2( lv.x - rv.x, lv.y - rv.y );
}

inline Float2 operator- ( const Float2& lv, float rv ) {
    return lv - Float2( rv );
}

inline Float2 operator- ( float lv, const Float2& rv ) {
    return Float2( lv ) - rv;
}


inline Float2 min( const Float2& v1, const Float2& v2 ) {
    Float2 res;
    res.x = v1.x < v2.x ? v1.x : v2.x;
    res.y = v1.y < v2.y ? v1.y : v2.y;
    return res;
}

inline Float2 max( const Float2& v1, const Float2& v2 ) {
    Float2 res;
    res.x = v1.x > v2.x ? v1.x : v2.x;
    res.y = v1.y > v2.y ? v1.y : v2.y;
    return res;
}


inline float sqr_length( const Float2& v ) {
    return v.x * v.x + v.y * v.y;
}

inline float length( const Float2& v ) {
    return sqrtf( sqr_length( v ) );
}


inline float dot( const Float2& v1, const Float2& v2 ) {
    return v1.x * v2.x + v1.y * v2.y;
}


inline float cross( const Float2& v1, const Float2& v2 ) {
    return v1.x * v2.y - v1.y * v2.x;
}

inline Float2 normalize( const Float2& v ) {
    return v / length( v );
}

inline Float2 mix( const Float2& p0, const Float2& p1, float t ) {
    return p0 * ( 1.0f - t ) + p1 * t;
}

inline Float2 perp_right( const Float2& v ) {
    return Float2( v.y, -v.x );
}

inline Float2 perp_left( const Float2& v ) {
    return Float2( -v.y, v.x );
}

inline Float2 clamp( const Float2& v, const Float2& vmin, const Float2& vmax ) {
    return max( min( v, vmax ), vmin );
}

inline Float2 vpow( const Float2& v, float p ) {
    return F2( powf( v.x, p ), powf( v.y, p ) );
}
