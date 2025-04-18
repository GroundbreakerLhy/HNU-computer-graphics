#version 330 core

in vec2 UV;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 color;

uniform sampler2D textureSampler;
uniform sampler2D shadowMap;
uniform vec3 viewPos;
uniform int showShadows;

uniform vec3 dirLight_direction;
uniform vec3 dirLight_ambient;
uniform vec3 dirLight_diffuse;
uniform vec3 dirLight_specular;
uniform float dirLight_intensity;

uniform vec3 pointLight_position;
uniform vec3 pointLight_ambient;
uniform vec3 pointLight_diffuse;
uniform vec3 pointLight_specular;
uniform float pointLight_constant;
uniform float pointLight_linear;
uniform float pointLight_quadratic;

uniform vec3 spotLight_position;
uniform vec3 spotLight_direction;
uniform vec3 spotLight_ambient;
uniform vec3 spotLight_diffuse;
uniform vec3 spotLight_specular;
uniform float spotLight_cutOff;
uniform float spotLight_outerCutOff;
uniform float spotLight_constant;
uniform float spotLight_linear;
uniform float spotLight_quadratic;
uniform float spotLight_intensity;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    if(showShadows == 0) return 0.0;
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

void main()
{
    vec3 norm = normalize(Normal);
    
    // 方向光计算
    vec3 lightDir = normalize(-dirLight_direction);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    vec3 ambient = dirLight_ambient * dirLight_intensity;
    vec3 diffuse = dirLight_diffuse * diff * dirLight_intensity;
    vec3 specular = dirLight_specular * spec * dirLight_intensity;
    
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 dirResult = ambient + (1.0 - shadow) * (diffuse + specular);
    
    // 点光源计算
    lightDir = normalize(pointLight_position - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    float distance = length(pointLight_position - FragPos);
    float attenuation = 1.0 / (pointLight_constant + pointLight_linear * distance + 
                     pointLight_quadratic * (distance * distance));
    
    vec3 pointAmbient = pointLight_ambient * attenuation;
    vec3 pointDiffuse = pointLight_diffuse * diff * attenuation;
    vec3 pointSpecular = pointLight_specular * spec * attenuation;
    vec3 pointResult = pointAmbient + pointDiffuse + pointSpecular;
    
    // 聚光灯计算
    lightDir = normalize(spotLight_position - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    distance = length(spotLight_position - FragPos);
    attenuation = 1.0 / (spotLight_constant + spotLight_linear * distance + spotLight_quadratic * (distance * distance));
    
    float theta = dot(lightDir, normalize(-spotLight_direction));
    float epsilon = spotLight_cutOff - spotLight_outerCutOff;
    float intensity = clamp((theta - spotLight_outerCutOff) / epsilon, 0.0, 1.0) * spotLight_intensity;
    
    vec3 spotAmbient = spotLight_ambient * attenuation * intensity;
    vec3 spotDiffuse = spotLight_diffuse * diff * attenuation * intensity;
    vec3 spotSpecular = spotLight_specular * spec * attenuation * intensity;
    vec3 spotResult = spotAmbient + spotDiffuse + spotSpecular;
    
    vec3 result = texture(textureSampler, UV).rgb * (dirResult + pointResult + spotResult);
    color = vec4(result, 1.0);
}
