#version 450 //glsl version

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 normalForFP;
layout(location = 2) in vec3 lightPos;
layout(location = 3) in vec3 positionForFP;

layout(location = 0) out vec4 outColor; //final output color, must have location 0. we output to the first attachment

void main()
{
    vec3 lightDirection = normalize(lightPos - positionForFP );
    vec3 viewDirection = normalize(-positionForFP );
    vec3 halfVector = normalize( lightDirection + viewDirection);

    float diffuse = max(0.0f,dot( normalForFP.xyz, lightDirection));
    float specular = max(0.0f,dot( normalForFP.xyz, halfVector ) );
    float distanceFromLight = length(lightPos - positionForFP);

    if (diffuse == 0.0) {
        specular = 0.0;
    } else {
        specular = pow( specular, 32.0f );
    }

    vec3 scatteredLight = 1.0f/distanceFromLight * fragColor.xyz * diffuse;
    vec3 reflectedLight = vec3(1.0f,1.0f,1.0f) * specular;
    vec3 ambientLight = fragColor.xyz * 0.1f;

    outColor = vec4(min( ambientLight + scatteredLight + reflectedLight, vec3(1,1,1)), 1.0);
}