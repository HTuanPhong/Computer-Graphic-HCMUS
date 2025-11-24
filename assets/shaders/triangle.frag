#version 330 core
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
    if (sampled.a == 0) {
        discard;
    }
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 1;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 ambientAndDiffuse = (ambient + diffuse) * VertexColor.rgb * sampled.rgb;
    vec3 result = ambientAndDiffuse + specular;
    FragColor = vec4(result, VertexColor.a * sampled.a);
}