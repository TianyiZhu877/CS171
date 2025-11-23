// Uniforms for lighting (passed from CPU)
uniform int numLights;
uniform vec3 lightPositions[16];      // Array of light positions
uniform vec3 lightColors[16];         // Array of light colors
uniform float lightAttenuations[16]; // Array of light attenuation factors (k)
uniform vec3 eyePosition;

// Uniforms for material properties (passed from CPU per object)
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

// passed from vertex shader
varying vec3 worldPosition;  // Position in world/camera space
varying vec3 worldNormal;     // Normal in world/camera space

void main() {
    vec3 normal = normalize(worldNormal);
    
    // Eye position in camera/view space is at origin
    // vec3 eyePos = vec3(0.0, 0.0, 0.0);
    // vec3 viewDir = normalize(eyePos - worldPosition);
    vec3 viewDir = normalize(eyePosition - worldPosition);
    
    // Initialize diffuse and specular sums
    vec3 diffuseSum = vec3(0.0, 0.0, 0.0);
    vec3 specularSum = vec3(0.0, 0.0, 0.0);
    
    // Loop through all lights
    for (int i = 0; i < numLights && i < 8; i++) {
        // Light vector from fragment to light
        vec3 lightVec = lightPositions[i] - worldPosition;
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
    
    // Final color: ambient + diffuse * materialDiffuse + specular * materialSpecular
    vec3 ambient = materialAmbient;
    vec3 diffuse = diffuseSum * materialDiffuse;
    vec3 specular = specularSum * materialSpecular;
    
    vec3 result = ambient + diffuse + specular;
    
    result = clamp(result, 0.0, 1.0);
    
    gl_FragColor = vec4(result, 1.0);
}
