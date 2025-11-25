attribute vec3 vertexTangent;

// Outputs to fragment shader (will be interpolated across fragments)
varying vec3 cameraPosition;  // Position in camera space
varying vec3 cameraNormal;     // Normal in camera space
varying vec3 cameraTangent;   // Tangent in camera space
varying vec3 cameraBitangent; // Bitangent in camera space
varying vec2 texCoordVarying; // Texture coordinates passed to fragment shader

void main() {
    // Transform vertex position to camera/camera space using built-in matrix
    vec4 position = gl_ModelViewMatrix * gl_Vertex;
    cameraPosition = position.xyz;  // Pass camera position to fragment shader
    
    // Transform normal to camera/camera space using built-in normal matrix
    cameraNormal = normalize(gl_NormalMatrix * gl_Normal);
    cameraTangent = normalize(gl_NormalMatrix * vertexTangent);
    cameraBitangent = cross(cameraTangent, cameraNormal);
    
    // Get texture coordinates from gl_MultiTexCoord0 (set by glTexCoord2f)
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    // Transform to clip space using built-in combined matrix (or separate matrices)
    gl_Position = gl_ProjectionMatrix * position;
}
