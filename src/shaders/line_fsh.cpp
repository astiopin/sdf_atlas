const char *line_fsh =  R"(  // "
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
uniform float line_width;
    
varying vec2 vpar;
varying vec2 vlimits;
varying float dist_scale;


void main() {
    float p = 0.5 - vpar.y;
    float q = -0.5 * vpar.x;
    
    // Solving  x^3 + p*x + q = 0

    float sigx = vpar.x > 0.0 ? 1.0 : -1.0;
    float sq = 27.0*q*q;
    float cp = 4.0*p*p*p;
    float tp = -p * 0.33333333;
    float dist;

    if ( sq >= -cp ) {
        // Point below evolute - single root        
        float rcb = 0.096225; // 1 / ( 2*3^(3/2) )
        float mc = sigx * pow( sqrt( abs( sq + cp ) ) * rcb + 0.5 * abs( q ), 0.33333333 ); 
        float x0 = tp / mc + mc;
        x0 = clamp( x0, vlimits.x, vlimits.y );
        dist = length( vec2( x0, x0*x0 ) - vpar );
    } else {
        // Point above evolute - three roots

        float a2   = abs( sq / cp );
        float a    = sqrt( a2 ); 

        // Exact solution
        //float dacs = 2.0 * cos( acos( a ) / 3.0 );
        // Approximation with cubic
        float dacs = a2 * ( 0.01875324 * a - 0.08179158 ) + ( 0.33098754 * a + 1.7320508 );

        float rsp = sqrt( abs( tp ) );
        float x0 = sigx * rsp * dacs;
        
        float dx = sigx * sqrt( -0.75 * x0*x0 - p );
        float x1 = -0.5 * x0 - dx;

        //Third root is never the closest
        //float x2 = -0.5 * x0 + dx;        

        x0 = clamp( x0, vlimits.x, vlimits.y );        
        x1 = clamp( x1, vlimits.x, vlimits.y );

        float d0 = length( vec2( x0, x0*x0 ) - vpar );
        float d1 = length( vec2( x1, x1*x1 ) - vpar );

        dist = min( d0, d1 );
    }

    float pdist = min( dist * dist_scale, 1.0 );
    
    float color = 0.5 - 0.5 * pdist;

    if ( color == 0.0 ) discard;

    gl_FragColor = vec4( color );
    gl_FragDepth = pdist;        
}
    

)"; // "
