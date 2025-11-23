// Outputs to fragment shader (will be interpolated across fragments)
varying vec3 worldPosition;  // Position in world/camera space
varying vec3 worldNormal;     // Normal in world/camera space

void main() {
    // Transform vertex position to camera/world space using built-in matrix
    vec4 position = gl_ModelViewMatrix * gl_Vertex;
    worldPosition = position.xyz;  // Pass world position to fragment shader
    
    // Transform normal to world/camera space using built-in normal matrix
    worldNormal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Transform to clip space using built-in combined matrix (or separate matrices)
    gl_Position = gl_ProjectionMatrix * position;
}

