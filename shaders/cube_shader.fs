#version 330 core

struct PointLight
{
    vec3 position;
    vec3 colour;
    float intensity;
};

uniform PointLight lights[16];
uniform int lightCount;
uniform vec3 viewPos;

uniform sampler2D tex;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 albedo = texture(tex, TexCoord).rgb;

    vec3 result = vec3(0.0);

    for(int i = 0; i < lightCount; i++)
    {
        vec3 lightDir = normalize(lights[i].position - FragPos);

        float diff = max(dot(norm, lightDir), 0.0);

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 /
            (1.0 + 0.1 * distance + 0.1 * distance * distance);

        vec3 ambient = 0.025 * albedo;
        vec3 diffuse = diff * albedo * lights[i].colour * lights[i].intensity;
        vec3 specular = spec * lights[i].colour;

        result += (diffuse + specular) * attenuation + ambient;
    }

    FragColor = vec4(result, 1.0);
}