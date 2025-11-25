// Uniforms for lighting (passed from CPU)
uniform int numLights;
uniform vec3 lightPositions[16];      // Array of light positions
uniform vec3 lightColors[16];         // Array of light colors
uniform float lightAttenuations[16]; // Array of light attenuation factors (k)
uniform vec3 eyePosition;

// Uniforms for material properties (passed from CPU per object)
uniform vec3 materialAmbient;
// uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform sampler2D normalMap;
uniform sampler2D textureMap;

// passed from vertex shader
varying vec3 cameraPosition;  // Position in camera space
varying vec3 cameraNormal;     // Normal in camera space
varying vec3 cameraTangent;   // Tangent in camera space
varying vec3 cameraBitangent; // Bitangent in camera space
// varying vec2 texCoordVarying; // Texture coordinates

void main() {    
    vec3 texture_normal = normalize(texture2D(normalMap, gl_TexCoord[0].st).rgb * 2.0 - 1.0);
    mat3 TBN = mat3(cameraTangent, cameraBitangent, cameraNormal);
    // Manually transpose: construct matrix with rows instead of columns
    mat3 TBN_transposed = mat3(
        cameraTangent.x, cameraBitangent.x, cameraNormal.x,
        cameraTangent.y, cameraBitangent.y, cameraNormal.y,
        cameraTangent.z, cameraBitangent.z, cameraNormal.z
    );
    // vec3 normal = normalize(TBN_transposed * texture_normal);  // With transpose
    vec3 normal = normalize(TBN * texture_normal);         // Without transpose (alternative)
    
    // Eye position in camera/view space is at origin
    // vec3 eyePos = vec3(0.0, 0.0, 0.0);
    // vec3 viewDir = normalize(eyePos - cameraPosition);
    vec3 viewDir = normalize(eyePosition - cameraPosition);
    // gl_TexCoord[0] = gl_MultiTexCoord0;
    // Initialize diffuse and specular sums
    vec3 diffuseSum = vec3(0.0, 0.0, 0.0);
    vec3 specularSum = vec3(0.0, 0.0, 0.0);
    
    // Loop through all lights
    for (int i = 0; i < numLights && i < 16; i++) {
        // Light vector from fragment to light
        vec3 lightVec = lightPositions[i] - cameraPosition;
        float distance = length(lightVec);
        vec3 lightDir = normalize(lightVec);
        
        float attenuation = 1.0 / (1.0 + lightAttenuations[i] * distance * distance);
        
        float diffuseFactor = max(0.0, dot(normal, lightDir));
        vec3 lightDiffuse = lightColors[i] * diffuseFactor * attenuation;
        diffuseSum += lightDiffuse;
        
        vec3 halfVector = normalize(viewDir + lightDir);
        float specularFactor = pow(max(0.0, dot(normal, halfVector)), materialShininess);
        vec3 lightSpecular = lightColors[i] * specularFactor * attenuation;
        specularSum += lightSpecular;
    }
    
    // Final color: ambient + diffuse * texture + specular * materialSpecular
    vec3 ambient = materialAmbient;
    vec3 diffuse = diffuseSum * texture2D(textureMap, gl_TexCoord[0].st).rgb;
    // vec3 diffuse = diffuseSum * texture2D(textureMap, texCoordVarying).rgb;
    vec3 specular = specularSum * materialSpecular;
    
    vec3 result = ambient + diffuse + specular;
    
    result = clamp(result, 0.0, 1.0);
    // result = clamp(diffuse, 0.0, 1.0);

    // vec4 a = vec4(texture2D(normalMap, texCoordVarying).rgb, 1.0);
    // gl_FragColor = vec4(texture2D(textureMap, texCoordVarying).rgb, 1.0);
    gl_FragColor = vec4(result, 1.0);
}
