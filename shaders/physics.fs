#version 330 core
out vec4 FragColor;

uniform vec2 bounds;
uniform sampler2D physicsInput;

void FAST32_hash_2D( vec2 gridcell, out vec4 hash_0, out vec4 hash_1 )    //    generates 2 random numbers for each of the 4 cell corners
{
    //    gridcell is assumed to be an integer coordinate
    const vec2 OFFSET = vec2( 26.0, 161.0 );
    const float DOMAIN = 71.0;
    const vec2 SOMELARGEFLOATS = vec2( 951.135664, 642.949883 );
    vec4 P = vec4( gridcell.xy, gridcell.xy + 1.0 );
    P = P - floor(P * ( 1.0 / DOMAIN )) * DOMAIN;
    P += OFFSET.xyxy;
    P *= P;
    P = P.xzxz * P.yyww;
    hash_0 = fract( P * ( 1.0 / SOMELARGEFLOATS.x ) );
    hash_1 = fract( P * ( 1.0 / SOMELARGEFLOATS.y ) );
}

float SimplexPerlin2D( vec2 P )
{
    //    simplex math constants
    const float SKEWFACTOR = 0.36602540378443864676372317075294;            // 0.5*(sqrt(3.0)-1.0)
    const float UNSKEWFACTOR = 0.21132486540518711774542560974902;            // (3.0-sqrt(3.0))/6.0
    const float SIMPLEX_TRI_HEIGHT = 0.70710678118654752440084436210485;    // sqrt( 0.5 )    height of simplex triangle
    const vec3 SIMPLEX_POINTS = vec3( 1.0-UNSKEWFACTOR, -UNSKEWFACTOR, 1.0-2.0*UNSKEWFACTOR );        //    vertex info for simplex triangle
    
    //    establish our grid cell.
    P *= SIMPLEX_TRI_HEIGHT;        // scale space so we can have an approx feature size of 1.0  ( optional )
    vec2 Pi = floor( P + dot( P, vec2( SKEWFACTOR ) ) );
    
    //    calculate the hash.
    //    ( various hashing methods listed in order of speed )
    vec4 hash_x, hash_y;
    FAST32_hash_2D( Pi, hash_x, hash_y );
    //SGPP_hash_2D( Pi, hash_x, hash_y );
    
    //    establish vectors to the 3 corners of our simplex triangle
    vec2 v0 = Pi - dot( Pi, vec2( UNSKEWFACTOR ) ) - P;
    vec4 v1pos_v1hash = (v0.x < v0.y) ? vec4(SIMPLEX_POINTS.xy, hash_x.y, hash_y.y) : vec4(SIMPLEX_POINTS.yx, hash_x.z, hash_y.z);
    vec4 v12 = vec4( v1pos_v1hash.xy, SIMPLEX_POINTS.zz ) + v0.xyxy;
    
    //    calculate the dotproduct of our 3 corner vectors with 3 random normalized vectors
    vec3 grad_x = vec3( hash_x.x, v1pos_v1hash.z, hash_x.w ) - 0.49999;
    vec3 grad_y = vec3( hash_y.x, v1pos_v1hash.w, hash_y.w ) - 0.49999;
    vec3 grad_results = inversesqrt( grad_x * grad_x + grad_y * grad_y ) * ( grad_x * vec3( v0.x, v12.xz ) + grad_y * vec3( v0.y, v12.yw ) );
    
    //    Normalization factor to scale the final result to a strict 1.0->-1.0 range
    //    x = ( sqrt( 0.5 )/sqrt( 0.75 ) ) * 0.5
    //    NF = 1.0 / ( x * ( ( 0.5 ñ x*x ) ^ 4 ) * 2.0 )
    //    http://briansharpe.wordpress.com/2012/01/13/simplex-noise/#comment-36
    const float FINAL_NORMALIZATION = 99.204334582718712976990005025589;
    
    //    evaluate the surflet, sum and return
    vec3 m = vec3( v0.x, v12.xz ) * vec3( v0.x, v12.xz ) + vec3( v0.y, v12.yw ) * vec3( v0.y, v12.yw );
    m = max(0.5 - m, 0.0);        //    The 0.5 here is SIMPLEX_TRI_HEIGHT^2
    m = m*m;
    return dot(m*m, grad_results) * FINAL_NORMALIZATION;
}


vec4 texel(vec2 offset) {
    vec2 coord = (gl_FragCoord.xy + offset) / bounds;
    return texture(physicsInput, coord);
}

void main()
{
    
    if (gl_FragCoord.x < 0.5 * bounds.x)
        FragColor = vec4(0.0,gl_FragCoord.x/bounds.x,0.0,1.0);
    else {
//        FragColor = vec4(gl_FragCoord.x/bounds.x,0.0,0.0,1.0);
//        FragColor = texture(physicsInput, vec2(0.0,0.0));
        FragColor = texel(vec2(0.0,0.0));  
    }
    
    vec2 pos = gl_FragCoord.xy + texel(vec2(0.0, 1.0)).xy ;
    
//    FragColor = vec4(0.5, 0.5, 0.5, 1.0);
    FragColor = vec4(SimplexPerlin2D(pos + 0.1), SimplexPerlin2D(pos - 0.1), 1.0, 1.0);
    
    
//    FragColor = vec4(0.0,0.0,max(0,gl_FragCoord.x-0.5),1.0);
}
