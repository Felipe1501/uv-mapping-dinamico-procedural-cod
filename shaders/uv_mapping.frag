#version 450

layout(set = 0, binding = 0) uniform Parameters
{
    mat4 model;
    mat4 view;
    mat4 projection;
    float time;
} params;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main()
{
    // UV DINÂMICA

    vec2 uv = fragUV;

    uv.x += params.time * 0.40;
    uv.y += sin(params.time * 1.2) * 0.10;


    // DISTORÇÃO PROCEDURAL

    float distortionX =
        sin(uv.y * 14.0 + params.time * 2.0) * 0.08;

    float distortionY =
        cos(uv.x * 12.0 - params.time * 1.5) * 0.08;

    uv.x += distortionX;
    uv.y += distortionY;


    // ONDA PRINCIPAL

    float wave1 =
        sin(
            uv.x * 20.0
            + uv.y * 8.0
            + params.time * 4.0
        );

    float wave2 =
        sin(
            uv.y * 24.0
            - uv.x * 6.0
            - params.time * 3.0
        );

    float wave3 =
        sin(
            (uv.x + uv.y) * 18.0
            + params.time * 2.0
        );


    // Combinação das ondas

    float energy =
        (wave1 + wave2 + wave3) / 3.0;

    energy =
        energy * 0.5 + 0.5;


    // PADRÃO DE GRADE

    vec2 gridUV =
        fract(uv * 8.0);

    float gridX =
        smoothstep(
            0.46,
            0.50,
            abs(gridUV.x - 0.5)
        );

    float gridY =
        smoothstep(
            0.46,
            0.50,
            abs(gridUV.y - 0.5)
        );

    float grid =
        max(gridX, gridY);


    // CORES

    vec3 deepBlue =
        vec3(
            0.005,
            0.015,
            0.12
        );

    vec3 blue =
        vec3(
            0.0,
            0.18,
            0.85
        );

    vec3 cyan =
        vec3(
            0.0,
            0.85,
            1.0
        );

    vec3 white =
        vec3(
            0.65,
            1.0,
            1.0
        );


    // GRADIENTE DE ENERGIA

    vec3 color =
        mix(
            deepBlue,
            blue,
            energy
        );

    color =
        mix(
            color,
            cyan,
            smoothstep(0.55, 0.90, energy)
        );

    color =
        mix(
            color,
            white,
            smoothstep(0.90, 1.0, energy)
        );


    // GRADE SUTIL

    color =
        mix(
            color,
            color * 0.35,
            grid * 0.25
        );


    // BRILHO

    float glow =
        pow(energy, 3.0);

    color +=
        vec3(0.0, 0.25, 0.35) * glow;


    // SAIDA

    outColor =
        vec4(color, 1.0);
}