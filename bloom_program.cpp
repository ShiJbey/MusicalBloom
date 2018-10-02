#include "bloom_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

// Implementation of a Bloom Shader with help from
// https://learnopengl.com/Advanced-Lighting/Bloom
BloomProgram::BloomProgram() {
	program = compile_program(
		"#version 330\n"
		"layout (location = 0) in vec3 Position;\n" //note: layout keyword used to make sure that the location-0 attribute is always bound to something
		"layout (location = 1) in vec3 Normal;\n"
		"layout (location = 2) in vec4 Color;\n"
        "struct VS_OUT {\n"
        "   vec4 objectColor\n"
        "   vec3 fragPos;\n"
        "   vec3 Normal;\n"
        "   vec2 TexCoords;\n"
        "} vs_out;\n"
        "uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"
		"void main() {\n"
        "   vs_out.FragPos = vec3(model * vec4(Position, 1.0);\n"
        "   mat3 normalMatrix = transpose(inverse(mat3(model)));\n"
        "   vs_out.Normal = normalize(normalMatrix * Normal)\n"
		"	gl_Position = object_to_clip * Position;\n"
		"}\n"
		,
		"#version 330\n"
        "layout (location = 0) out vecc4 FragColor\n"
        "layout (location = 1) out vec4 BrightColor\n"
        "in VS_OUT {\n"
        "   vec4 objectColor\n"
        "   vec3 fragPos;\n"
        "   vec3 Normal;\n"
        "   vec2 TexCoords;\n"
        "} fs_in;\n"
        "struct Light {\n"
        "   vec3 Position;\n"
        "   vec3 Color;\n"
        "};\n"
		"uniform Light light;\n"
        "uniform sampler2D diffuseTexture;\n"
        "uniform vec3 viewPos\n"
		"void main() {\n"
		"	vec3 total_light = vec3(0.0, 0.0, 0.0);\n"
		"	vec3 n = normalize(normal);\n"
		"	{ //sky (hemisphere) light:\n"
		"		vec3 l = sky_direction;\n"
		"		float nl = 0.5 + 0.5 * dot(n,l);\n"
		"		total_light += nl * sky_color;\n"
		"	}\n"
		"	{ //sun (directional) light:\n"
		"		vec3 l = sun_direction;\n"
		"		float nl = max(0.0, dot(n,l));\n"
		"		total_light += nl * sun_color;\n"
		"	}\n"
		"	{ //spot (point with fov + shadow map) light:\n"
		"		vec3 l = normalize(spot_position - position);\n"
		"		float nl = max(0.0, dot(n,l));\n"
		"		//TODO: look up shadow map\n"
		"		float d = dot(l,-spot_direction);\n"
		"		float amt = smoothstep(spot_outer_inner.x, spot_outer_inner.y, d);\n"
		"		float shadow = textureProj(spot_depth_tex, spotPosition);\n"
		"		total_light += shadow * nl * amt * spot_color;\n"
		//"		fragColor = vec4(s,s,s, 1.0);\n" //DEBUG: just show shadow
		"	}\n"

		"	fragColor = texture(tex, texCoord) * vec4(color.rgb * total_light, color.a);\n"
		"}\n"
	);

	object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	//object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	//normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	sun_direction_vec3 = glGetUniformLocation(program, "sun_direction");
	sun_color_vec3 = glGetUniformLocation(program, "sun_color");
	sky_direction_vec3 = glGetUniformLocation(program, "sky_direction");
	sky_color_vec3 = glGetUniformLocation(program, "sky_color");
}

Load< BloomProgram > texture_program(LoadTagInit, [](){
	return new BloomProgram();
});
