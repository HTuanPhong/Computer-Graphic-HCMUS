#version 300 es
precision highp float;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 VertexColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform sampler2D textTexture;

void main()
{
    vec4 sampled = texture(textTexture, TexCoord);
    if (sampled.a == 0.0) {
        discard;
    }
    
    // Ambient - reduced strength for more realistic lighting
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine colors: apply ambient+diffuse to texture, then add specular and vertex color
    vec3 texColor = sampled.rgb * VertexColor.rgb;
    vec3 result = (ambient + diffuse) * texColor + specular;
    
    FragColor = vec4(result, VertexColor.a * sampled.a);
}