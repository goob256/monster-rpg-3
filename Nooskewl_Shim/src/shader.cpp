#include "Nooskewl_Shim/error.h"
#include "Nooskewl_Shim/gfx.h"
#include "Nooskewl_Shim/image_base.h"
#include "Nooskewl_Shim/shader.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/util.h"

#include "Nooskewl_Shim/internal/gfx.h"

using namespace noo;

#ifdef _WIN32
static float *tmp_vec;
static int tmp_vec_size;
#endif

namespace noo {

namespace gfx {

std::vector<Shader::Internal *> Shader::loaded_shaders;
std::vector< std::pair<std::string, Image_Base *> > Shader::bound_images;
Shader::Internal *Shader::last_shader;
GLuint Shader::opengl_texture0;
int Shader::d3d_vertex_shader_count;
int Shader::d3d_fragment_shader_count;

void Shader::static_start()
{
	loaded_shaders.clear();
	bound_images.clear();
	last_shader = 0;
	opengl_texture0 = 0;
	d3d_vertex_shader_count = 0;
	d3d_fragment_shader_count = 0;
#ifdef _WIN32
	tmp_vec = 0;
	tmp_vec_size = 0;
#endif
}

void Shader::release_all(bool force)
{
	for (size_t i = 0; i < loaded_shaders.size(); i++) {
		loaded_shaders[i]->release(force);
	}

	shim::current_shader = 0;

#ifdef _WIN32
	delete[] tmp_vec;
	tmp_vec = 0;
	tmp_vec_size = 0;
#endif
}

void Shader::reload_all(bool force)
{
	for (size_t i = 0; i < loaded_shaders.size(); i++) {
		loaded_shaders[i]->reload(force);
	}
}

std::vector< std::pair<std::string, Image_Base *> > Shader::get_bound_images()
{
	return bound_images;
}

void Shader::rebind_opengl_texture0()
{
	if (opengl_texture0 != 0) {
		glActiveTexture_ptr(GL_TEXTURE0);
		PRINT_GL_ERROR("glActiveTexture\n");

		glBindTexture_ptr(GL_TEXTURE_2D, opengl_texture0);
		PRINT_GL_ERROR("glBindTexture\n");
	}
}

void Shader::audit()
{
	util::debugmsg("d3d_vertex_shader_count=%d\n", d3d_vertex_shader_count);
	util::debugmsg("d3d_fragment_shader_count=%d\n", d3d_fragment_shader_count);
}

#ifdef USE_D3DX
Shader::Shader(std::string vertex_source, std::string fragment_source)
{
	internal = new Internal();

	internal->opengl = false;

#ifdef _WIN32
#ifdef USE_D3DX
	internal->_is_d3dx = true;
#endif
#endif

	internal->is_master_vertex = true;
	internal->is_master_fragment = true;

	internal-.vertex_source = vertex_source;
	internal-.fragment_source = fragment_source;

	loaded_shaders.push_back(internal);

	internal->reload(true);
}
#endif

Shader::Shader(OpenGL_Shader *vertex_shader, OpenGL_Shader *fragment_shader, bool is_master_vertex, bool is_master_fragment)
{
	internal = new Internal();

	internal->opengl = true;

	internal->is_master_vertex = is_master_vertex;
	internal->is_master_fragment = is_master_fragment;

	internal->opengl_vertex = vertex_shader;
	internal->opengl_fragment = fragment_shader;

	loaded_shaders.push_back(internal);

	internal->reload(true);
}

#ifdef USE_D3DX
Shader::Shader(std::string vertex_filename, std::string fragment_filename)
{
	internal = new Internal();

	internal->opengl = false;

	internal->_is_d3dx = false;

	internal->is_master_vertex = true;
	internal->is_master_fragment = true;

	internal->d3d_vertex = load_d3d_vertex_shader(vertex_filename);
	internal->d3d_fragment = load_d3d_fragment_shader(fragment_filename);

	loaded_shaders.push_back(internal);

	internal->reload(true);
}
#endif

#ifdef _WIN32
Shader::Shader(D3D_Vertex_Shader *vertex_shader, D3D_Fragment_Shader *fragment_shader, bool is_master_vertex, bool is_master_fragment)
{
	internal = new Internal();

	internal->opengl = false;

#ifdef USE_D3DX
	internal->_is_d3dx = false;
#endif

	internal->is_master_vertex = is_master_vertex;
	internal->is_master_fragment = is_master_fragment;

	internal->d3d_vertex = vertex_shader;
	internal->d3d_fragment = fragment_shader;

	loaded_shaders.push_back(internal);

	internal->reload(true);
}
#endif

Shader::~Shader()
{
	for (size_t i = 0; i < loaded_shaders.size(); i++) {
		if (loaded_shaders[i] == internal) {
			loaded_shaders.erase(loaded_shaders.begin()+i);
			break;
		}
	}
	delete internal;
}

void Shader::use()
{
	internal->use();
}

void Shader::Internal::use()
{
	if (last_shader != 0 && last_shader != this) {
		unbind(last_shader);
	}
	else if (last_shader == this) {
		return;
	}

	if (opengl) {
		glUseProgram_ptr(opengl_shader);
		PRINT_GL_ERROR("glUseProgram\n");
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		// FIXME: d3dx version!
		if (_is_d3dx == false)
#endif
		{
			if (last_shader == 0 || last_shader->d3d_vertex->shader != d3d_vertex->shader) {
				shim::d3d_device->SetVertexShader(d3d_vertex->shader);
			}
			if (last_shader == 0 || last_shader->d3d_fragment->shader != d3d_fragment->shader) {
				shim::d3d_device->SetPixelShader(d3d_fragment->shader);
			}
		}
	}
#endif

	last_shader = this;
}

#ifdef USE_D3DX
bool Shader::is_d3dx()
{
	return internal->_is_d3dx;
}
#endif

void Shader::set_texture(std::string name, Image_Base *image, int unit)
{
	internal->set_texture(name, image, unit);
}

void Shader::set_matrix(std::string name, glm::mat4 &matrix)
{
	if (internal->opengl) {
		GLint loc = internal->get_uniform_location(name);
		if (loc != -1) {
			glUniformMatrix4fv_ptr(loc, 1, GL_FALSE, glm::value_ptr(matrix));
			PRINT_GL_ERROR("glUniformMatrix4fv\n");
		}
	}
#ifdef _WIN32
	else {
#if USE_D3DX
		if (internal->_is_d3dx) {
			internal->d3d_effect->SetMatrix(name.c_str(), (D3DXMATRIX *)glm::value_ptr(matrix));
		}
		else
#endif
		{
			glm::mat4 transposed = matrix;
			transposed = glm::transpose(transposed);
			float *floats = glm::value_ptr(transposed);

			std::map<std::string, int>::iterator it;
			if ((it = internal->d3d_vertex->floats.find(name)) != internal->d3d_vertex->floats.end()) {
				int size_in_shader = internal->d3d_vertex->sizes[name];
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetVertexShaderConstantF(p.second, floats, MIN(size_in_shader, 4));
			}
			else {
				//util::debugmsg("Trying to set floats %s in vertex shader but it doesn't exist!\n", name.c_str());
			}
			if ((it = internal->d3d_fragment->floats.find(name)) != internal->d3d_fragment->floats.end()) {
				int size_in_shader = internal->d3d_fragment->sizes[name];
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetPixelShaderConstantF(p.second, floats, MIN(size_in_shader, 4));
			}
			else {
				//util::debugmsg("Trying to set floats %s in fragment shader but it doesn't exist!\n", name.c_str());
			}
		}
	}
#endif
}

void Shader::set_float(std::string name, float value)
{
	if (internal->opengl) {
		GLint loc = internal->get_uniform_location(name);
		if (loc != -1) {
			glUniform1f_ptr(loc, value);
			PRINT_GL_ERROR("glUniform1f\n");
		}
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (internal->_is_d3dx) {
			internal->d3d_effect->SetFloat(name.c_str(), value);
		}
		else
#endif
		{
			float floats[4] = { 0 };
			floats[0] = value;
			std::map<std::string, int>::iterator it;
			if ((it = internal->d3d_vertex->floats.find(name)) != internal->d3d_vertex->floats.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetVertexShaderConstantF(p.second, floats, 1);
			}
			else {
				//util::debugmsg("Trying to set matrix %s in vertex shader but it doesn't exist!\n", name.c_str());
			}
			if ((it = internal->d3d_fragment->floats.find(name)) != internal->d3d_fragment->floats.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetPixelShaderConstantF(p.second, floats, 1);
			}
			else {
				//util::debugmsg("Trying to set matrix %s in fragment shader but it doesn't exist!\n", name.c_str());
			}
		}
	}
#endif
}

// Taken from Allegro
bool Shader::set_float_vector(std::string name, int num_components, const float *vector, int num_elements)
{
	if (internal->opengl) {
		GLint loc = internal->get_uniform_location(name);

		if (loc < 0) {
			return false;
		}

		switch (num_components) {
			case 1:
				glUniform1fv_ptr(loc, num_elements, vector);
				break;
			case 2:
				glUniform2fv_ptr(loc, num_elements, vector);
				break;
			case 3:
				glUniform3fv_ptr(loc, num_elements, vector);
				break;
			case 4:
				glUniform4fv_ptr(loc, num_elements, vector);
				break;
			default:
				return false;
		}
		PRINT_GL_ERROR("glUniform?fv\n");
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (use_d3dx) {
			return internal->d3d_effect->SetFloatArray(name.c_str(), vector, num_components * num_elements) == D3D_OK;
		}
		else
#endif
		{
			// D3D expects float vectors in chunks of 4 floats, so we might have to juggle a few things
			float *v;
			if (num_components != 4) {
				if (num_elements * 4 > tmp_vec_size) {
					delete[] tmp_vec;
					tmp_vec_size = num_elements * 4;
					tmp_vec = new float[tmp_vec_size];
				}
				v = tmp_vec;
				for (int i = 0; i < num_elements; i++) {
					for (int j = 0; j < num_components; j++) {
						v[i*4+j] = vector[i*num_components+j];
					}
				}
			}
			else {
				v = (float *)vector;
			}
			std::map<std::string, int>::iterator it;
			if ((it = internal->d3d_vertex->floats.find(name)) != internal->d3d_vertex->floats.end()) {
				int size_in_shader = internal->d3d_vertex->sizes[name];
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetVertexShaderConstantF(p.second, v, MIN(size_in_shader, num_elements));
			}
			else {
				//util::debugmsg("Trying to set vector %s in vertex shader but it doesn't exist!\n", name.c_str());
			}
			if ((it = internal->d3d_fragment->floats.find(name)) != internal->d3d_fragment->floats.end()) {
				int size_in_shader = internal->d3d_fragment->sizes[name];
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetPixelShaderConstantF(p.second, v, MIN(size_in_shader, num_elements));
			}
			else {
				//util::debugmsg("Trying to set vector %s in fragment shader but it doesn't exist!\n", name.c_str());
			}
		}
	}
#endif

	return true;
}

void Shader::set_bool(std::string name, bool value)
{
	if (internal->opengl) {
		GLint loc = internal->get_uniform_location(name);
		if (loc != -1) {
			glUniform1i_ptr(loc, value);
			PRINT_GL_ERROR("glUniform1i\n");
		}
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (internal->_is_d3dx) {
			internal->d3d_effect->SetBool(name.c_str(), value);
		}
		else
#endif
		{
			BOOL bools[4] = { 0 };
			bools[0] = value ? 1 : 0;
			std::map<std::string, int>::iterator it;
			if ((it = internal->d3d_vertex->bools.find(name)) != internal->d3d_vertex->bools.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetVertexShaderConstantB(p.second, bools, 1);
			}
			else {
				//util::debugmsg("Trying to set matrix %s in vertex shader but it doesn't exist!\n", name.c_str());
			}
			if ((it = internal->d3d_fragment->bools.find(name)) != internal->d3d_fragment->bools.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetPixelShaderConstantB(p.second, bools, 1);
			}
			else {
				//util::debugmsg("Trying to set matrix %s in fragment shader but it doesn't exist!\n", name.c_str());
			}
		}
	}
#endif
}

void Shader::set_int(std::string name, int value)
{
	if (internal->opengl) {
		GLint loc = internal->get_uniform_location(name);
		if (loc != -1) {
			glUniform1i_ptr(loc, value);
			PRINT_GL_ERROR("glUniform1i\n");
		}
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (internal->_is_d3dx) {
			internal->d3d_effect->SetInt(name.c_str(), value);
		}
		else
#endif
		{
			int ints[4] = { 0 };
			ints[0] = value;
			// Sometimes ints are set in float locations
			float floats[4] = { 0 };
			floats[0] = value;
			std::map<std::string, int>::iterator it;
			if ((it = internal->d3d_vertex->ints.find(name)) != internal->d3d_vertex->ints.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetVertexShaderConstantI(p.second, ints, 1);
			}
			else {
				if ((it = internal->d3d_vertex->floats.find(name)) != internal->d3d_vertex->floats.end()) {
					std::pair<std::string, int> p = *it;
					shim::d3d_device->SetVertexShaderConstantF(p.second, floats, 1);
				}
				else {
					//util::debugmsg("Trying to set matrix %s in vertex shader but it doesn't exist!\n", name.c_str());
				}
			}
			if ((it = internal->d3d_fragment->ints.find(name)) != internal->d3d_fragment->ints.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetPixelShaderConstantI(p.second, ints, 1);
			}
			else {
				if ((it = internal->d3d_fragment->floats.find(name)) != internal->d3d_fragment->floats.end()) {
					std::pair<std::string, int> p = *it;
					shim::d3d_device->SetPixelShaderConstantF(p.second, floats, 1);
				}
				else {
					//util::debugmsg("Trying to set matrix %s in fragment shader but it doesn't exist!\n", name.c_str());
				}
			}
		}
	}
#endif
}

bool Shader::set_colour(std::string name, SDL_Colour colour)
{
	float vector[4];
	vector[0] = colour.r / 255.0f;
	vector[1] = colour.g / 255.0f;
	vector[2] = colour.b / 255.0f;
	vector[3] = colour.a / 255.0f;

	if (internal->opengl) {
		GLint loc = internal->get_uniform_location(name);

		if (loc < 0) {
			return false;
		}

		glUniform4fv_ptr(loc, 1, vector);
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (internal->_is_d3dx) {
			return internal->d3d_effect->SetFloatArray(name.c_str(), vector, 4) == D3D_OK;
		}
		else
#endif
		{
			std::map<std::string, int>::iterator it;
			if ((it = internal->d3d_vertex->floats.find(name)) != internal->d3d_vertex->floats.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetVertexShaderConstantF(p.second, vector, 1);
			}
			else {
				//util::debugmsg("Trying to set vector %s in vertex shader but it doesn't exist!\n", name.c_str());
			}
			if ((it = internal->d3d_fragment->floats.find(name)) != internal->d3d_fragment->floats.end()) {
				std::pair<std::string, int> p = *it;
				shim::d3d_device->SetPixelShaderConstantF(p.second, vector, 1);
			}
			else {
				//util::debugmsg("Trying to set vector %s in fragment shader but it doesn't exist!\n", name.c_str());
			}
		}
	}
#endif

	return true;
}

GLuint Shader::get_opengl_shader()
{
	return internal->opengl_shader;
}

#ifdef _WIN32
#ifdef USE_D3DX
LPD3DXEFFECT Shader::get_d3d_effect()
{
	return internal->d3d_effect;
}
#endif
#endif

Shader::Internal::Internal() :
	opengl_shader(0),
	pos_attrib(-1),
	normal_attrib(-1),
	texcoord_attrib(-1),
	colour_attrib(-1),
	pos_ptr(0),
	normal_ptr(0),
	texcoord_ptr(0),
	colour_ptr(0)
{
}

Shader::Internal::~Internal()
{
	release(true);
	if (opengl) {
		if (is_master_vertex) {
			delete opengl_vertex;
		}
		if (is_master_fragment) {
			delete opengl_fragment;
		}
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (_is_d3dx == false)
#endif
		{
			if (is_master_vertex) {
				delete[] d3d_vertex->data;
				delete d3d_vertex;
			}
			if (is_master_fragment) {
				delete[] d3d_fragment->data;
				delete d3d_fragment;
			}
		}
	}
#endif
}

void Shader::Internal::set_texture(std::string name, Image_Base *image, int unit)
{
	Image_Base *root = image == 0 ? 0 : image->get_root();

	std::pair<std::string, Image_Base *> p;

	for (size_t i = bound_images.size(); (int)i < unit+1; i++) {
		p.first = "";
		p.second = 0;
		bound_images.push_back(p);
	}
	p.first = name;
	p.second = root;
	bound_images[unit] = p;

	if (root == 0) {
		if (unit == 0) {
			opengl_texture0 = 0;
		}
		return;
	}

	if (opengl) {
		GLuint texture = root->internal->texture;

		glActiveTexture_ptr(GL_TEXTURE0 + unit);
		PRINT_GL_ERROR("glActiveTexture\n");

		glBindTexture_ptr(GL_TEXTURE_2D, texture);
		PRINT_GL_ERROR("glBindTexture\n");

		if (unit == 0) {
			opengl_texture0 = texture;
		}

		GLint loc = get_uniform_location(name);
		if (loc != -1) {
			glUniform1i_ptr(loc, unit);
		}
	}
#ifdef _WIN32
	else if (internal::gfx_context.d3d_lost == false) {
		if (root != 0) {
#ifdef USE_D3DX
			if (_is_d3dx) {
				d3d_effect->SetTexture(name.c_str(), root->video_texture);
			}
			else
#endif
			{
				std::map<std::string, int>::iterator it;
				if ((it = d3d_fragment->samplers.find(name)) != d3d_fragment->samplers.end()) {
					std::pair<std::string, int> p = *it;
					shim::d3d_device->SetTexture(p.second, root->internal->video_texture);
				}
				else {
					//util::debugmsg("Trying to set sampler %s in fragment shader but it doesn't exist!\n", name.c_str());
				}
			}
		}
		else {
			shim::d3d_device->SetTexture(unit, 0);
		}
	}
#endif
}

void Shader::Internal::release(bool force)
{
	if (this == last_shader) {
		unbind(this);
	}

	if (opengl) {
		if (is_master_vertex && opengl_vertex->shader != 0) {
			glDeleteShader_ptr(opengl_vertex->shader);
			PRINT_GL_ERROR("glDeleteShader\n");
			opengl_vertex->shader = 0;
		}
		if (is_master_fragment && opengl_fragment->shader != 0) {
			glDeleteShader_ptr(opengl_fragment->shader);
			PRINT_GL_ERROR("glDeleteShader\n");
			opengl_fragment->shader = 0;
		}

		if (opengl_shader != 0) {
			glDeleteProgram_ptr(opengl_shader);
			PRINT_GL_ERROR("glDeleteProgram\n");
			opengl_shader = 0;
		}

		uniform_locations.clear();
		
		pos_attrib = normal_attrib = texcoord_attrib = colour_attrib = -1;
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (_is_d3dx && d3d_effect != 0) {
			util::verbosemsg("Shader::Internal::release (%p), d3d_effect->Release=%d\n", this, d3d_effect->Release());
			d3d_effect = 0;
		}
		else
#endif
		if (force)
		{
			if (is_master_vertex && d3d_vertex->shader) {
				util::verbosemsg("d3d_vertex->shader->Release (%p) = %d\n", this, d3d_vertex->shader->Release());
				d3d_vertex->shader = 0;
				d3d_vertex_shader_count--;
			}
			if (is_master_fragment && d3d_fragment->shader) {
				util::verbosemsg("d3d_fragment->shader->Release (%p) = %d\n", this, d3d_fragment->shader->Release());
				d3d_fragment->shader = 0;
				d3d_fragment_shader_count--;
			}
		}
	}
#endif
}

void Shader::Internal::reload(bool force)
{
	if (opengl) {
		if (is_master_vertex && opengl_vertex->shader == 0) {
			opengl_vertex->shader = compile_opengl_vertex_shader(opengl_vertex->source);
		}
		if (is_master_fragment && opengl_fragment->shader == 0) {
			opengl_fragment->shader = compile_opengl_fragment_shader(opengl_fragment->source);
		}

		if (opengl_shader == 0) {
			opengl_shader = glCreateProgram_ptr();
			glAttachShader_ptr(opengl_shader, opengl_vertex->shader);
			PRINT_GL_ERROR("glAttachShader\n");
			glAttachShader_ptr(opengl_shader, opengl_fragment->shader);
			PRINT_GL_ERROR("glAttachShader\n");
			glLinkProgram_ptr(opengl_shader);
			PRINT_GL_ERROR("glLinkProgram\n");
		}
	}
#ifdef _WIN32
	else {
#ifdef USE_D3DX
		if (_is_d3dx) {
			LPD3DXBUFFER errors;

			std::string shader_source = vertex_source + fragment_source;

			shader_source +=
				"technique TECH"
				"{"
				"		pass p1"
				"		{"
				"				VertexShader = compile vs_2_0 vs_main();"
				"				PixelShader = compile ps_2_0 ps_main();"
				"		}"
				"}";

			DWORD result = D3DXCreateEffect(shim::d3d_device, shader_source.c_str(), shader_source.length(), 0, 0, D3DXSHADER_PACKMATRIX_ROWMAJOR, 0, &d3d_effect, &errors);

			if (result != D3D_OK) {
				char *msg = (char *)errors->GetBufferPointer();
				throw util::Error("Shader error: " + std::string(msg));
			}

			d3d_technique = d3d_effect->GetTechniqueByName("TECH");
			d3d_effect->ValidateTechnique(d3d_technique);
			d3d_effect->SetTechnique(d3d_technique);
		}
		else
#endif
		if (force)
		{
			if (is_master_vertex && d3d_vertex->shader == 0) {
				if (shim::d3d_device->CreateVertexShader((DWORD *)d3d_vertex->data, &d3d_vertex->shader) != D3D_OK) {
					throw util::LoadError("Error creating vertex shader (precompiled)!");
				}
				d3d_vertex_shader_count++;
			}
			if (is_master_fragment && d3d_fragment->shader == 0) {
				if (shim::d3d_device->CreatePixelShader((DWORD *)d3d_fragment->data, &d3d_fragment->shader) != D3D_OK) {
					throw util::LoadError("Error creating fragment shader (precompiled)!");
				}
				d3d_fragment_shader_count++;
			}
		}
	}
#endif
}

void Shader::Internal::unbind(Shader::Internal *s)
{
	for (size_t i = 0; i < bound_images.size(); i++) {
		std::pair<std::string, Image_Base *> p = bound_images[i];
		s->set_texture(p.first, 0, (int)i);
	}

	bound_images.clear();
	
	if (s == last_shader) {
		last_shader = 0;
	}

	if (opengl) {
		pos_ptr = normal_ptr = texcoord_ptr = colour_ptr = 0;
	}
}

#ifdef _WIN32
void Shader::load_d3d_shader(std::string filename, D3D_Shader *shader)
{
	int full_size;
	SDL_RWops *f = util::open_file("gfx/shaders/hlsl/" + filename + ".shader", &full_size);
	if (f == 0) {
		throw util::FileNotFoundError("HLSL shader '" + filename + "' not found.");
	}

	int num_constants = util::SDL_fgetc(f);

	for (int i = 0; i < num_constants; i++) {
		int str_len = util::SDL_fgetc(f);
		std::string name;
		for (int j = 0; j < str_len; j++) {
			int c = util::SDL_fgetc(f);
			char s[2];
			s[0] = c;
			s[1] = 0;
			name += std::string(s);
		}
		int type = util::SDL_fgetc(f);
		int reg_num = util::SDL_fgetc(f);
		int sz = SDL_ReadLE32(f);

		if (type == 'c') {
			shader->floats[name] = reg_num;
		}
		else if (type == 'i') {
			shader->ints[name] = reg_num;
		}
		else if (type == 'b') {
			shader->bools[name] = reg_num;
		}
		else if (type == 's') {
			shader->samplers[name] = reg_num;
		}

		shader->sizes[name] = sz;
	}

	int offset = SDL_RWtell(f);
	int data_size = full_size - offset;

	shader->data = new Uint8[data_size];

	SDL_RWread(f, shader->data, data_size, 1);

	util::close_file(f);
}
		
Shader::D3D_Vertex_Shader *Shader::load_d3d_vertex_shader(std::string filename)
{
	D3D_Vertex_Shader *shader = new D3D_Vertex_Shader;
	shader->shader = 0;
	load_d3d_shader(filename, shader);
	return shader;
}
		
Shader::D3D_Fragment_Shader *Shader::load_d3d_fragment_shader(std::string filename)
{
	D3D_Fragment_Shader *shader = new D3D_Fragment_Shader;
	shader->shader = 0;
	load_d3d_shader(filename, shader);
	return shader;
}
#endif

void Shader::set_opengl_attributes(float *pos, float *normal, float *texcoord, float *colour)
{
	if (internal->pos_attrib == -1) {
		internal->pos_attrib = glGetAttribLocation_ptr(internal->opengl_shader, "in_position");
		if (internal->pos_attrib != -1) {
			glEnableVertexAttribArray_ptr(internal->pos_attrib);
			PRINT_GL_ERROR("glEnableVertexAttribArray _ptr(in_position)\n");
		}
	}

	if (internal->normal_attrib == -1) {
		internal->normal_attrib = glGetAttribLocation_ptr(internal->opengl_shader, "in_normal");
		if (internal->normal_attrib != -1) {
			glEnableVertexAttribArray_ptr(internal->normal_attrib);
			PRINT_GL_ERROR("glEnableVertexAttribArray _ptr(in_normal)\n");
		}
	}

	if (internal->texcoord_attrib == -1) {
		internal->texcoord_attrib = glGetAttribLocation_ptr(internal->opengl_shader, "in_texcoord");
		if (internal->texcoord_attrib != -1) {
			glEnableVertexAttribArray_ptr(internal->texcoord_attrib);
			PRINT_GL_ERROR("glEnableVertexAttribArray _ptr(in_texcoord)\n");
		}
	}

	if (internal->colour_attrib == -1) {
		internal->colour_attrib = glGetAttribLocation_ptr(internal->opengl_shader, "in_colour");
		if (internal->colour_attrib != -1) {
			glEnableVertexAttribArray_ptr(internal->colour_attrib);
			PRINT_GL_ERROR("glEnableVertexAttribArray _ptr(in_colour)\n");
		}
	}

	if (internal->pos_attrib != -1 && internal->pos_ptr != pos) {
		internal->pos_ptr = pos;
		glVertexAttribPointer_ptr(internal->pos_attrib, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), pos);
		PRINT_GL_ERROR("glVertexAttribPointer _ptr(in_position)\n");
	}

	if (internal->normal_attrib != -1 && internal->normal_ptr != normal) {
		internal->normal_ptr = normal;
		glVertexAttribPointer_ptr(internal->normal_attrib, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), normal);
		PRINT_GL_ERROR("glVertexAttribPointer _ptr(in_normal)\n");
	}

	if (internal->texcoord_attrib != -1 && internal->texcoord_ptr != texcoord) {
		internal->texcoord_ptr = texcoord;
		glVertexAttribPointer_ptr(internal->texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), texcoord);
		PRINT_GL_ERROR("glVertexAttribPointer _ptr(in_texcoord)\n");
	}

	if (internal->colour_attrib != -1 && internal->colour_ptr != colour) {
		internal->colour_ptr = colour;
		glVertexAttribPointer_ptr(internal->colour_attrib, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), colour);
		PRINT_GL_ERROR("glVertexAttribPointer _ptr(in_colour)\n");
	}
}

GLint Shader::Internal::get_uniform_location(std::string name)
{
	std::map<std::string, GLint>::iterator it = uniform_locations.find(name);
	if (it == uniform_locations.end()) {
		GLint loc = glGetUniformLocation_ptr(opengl_shader, name.c_str());
		PRINT_GL_ERROR("glGetUniformLocation\n");
		uniform_locations[name] = loc;
		return loc;
	}
	else {
		return (*it).second;
	}
}

GLuint Shader::compile_opengl_shader(GLenum type, std::string source)
{
	GLint status;

	const GLchar *p = (const GLchar *)source.c_str();

	GLuint shader;

	shader = glCreateShader_ptr(type);
	PRINT_GL_ERROR("glCreateShader\n");
	glShaderSource_ptr(shader, 1, &p, 0);
	PRINT_GL_ERROR("glShaderSource\n");
	glCompileShader_ptr(shader);
	PRINT_GL_ERROR("glCompileShader\n");
	glGetShaderiv_ptr(shader, GL_COMPILE_STATUS, &status);
	PRINT_GL_ERROR("glGetShaderiv\n");
	if (status != GL_TRUE) {
		char buffer[512];
		glGetShaderInfoLog_ptr(shader, 512, 0, buffer);
		throw util::Error(util::string_printf("%s shader error: %s", buffer, type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
	}

	return shader;
}

GLuint Shader::compile_opengl_vertex_shader(std::string source)
{
	return compile_opengl_shader(GL_VERTEX_SHADER, source);
}

GLuint Shader::compile_opengl_fragment_shader(std::string source)
{
	return compile_opengl_shader(GL_FRAGMENT_SHADER, source);
}


std::string Shader::add_opengl_header(bool is_vertex, Precision precision, std::string source)
{
#if defined ANDROID || defined IOS || defined RASPBERRYPI
	std::string p;
	if (is_vertex) {
		if (precision == LOW) {
			p = "lowp";
		}
		else if (precision == MEDIUM) {
			p = "mediump";
		}
		else {
			p = "highp"; // default
		}
	}
	else if (is_vertex == false) {
		if (precision == MEDIUM) {
			p = "mediump";
		}
		else if (precision == HIGH) {
			p = "highp";
		}
		else {
			p = "lowp"; // default
		}
	}
	source = "precision " + p + " float;\n" + source;
#else
	source = std::string("#version 120\n") + source;
#endif
	return source;
}

Shader::OpenGL_Shader *Shader::load_opengl_vertex_shader(std::string source, Precision precision)
{
	OpenGL_Shader *shader = new OpenGL_Shader;
	shader->type = GL_VERTEX_SHADER;
	shader->precision = precision;
	shader->source = source = add_opengl_header(true, precision, source);
	shader->shader = 0;
	return shader;
}

Shader::OpenGL_Shader *Shader::load_opengl_fragment_shader(std::string source, Precision precision)
{
	OpenGL_Shader *shader = new OpenGL_Shader;
	shader->type = GL_FRAGMENT_SHADER;
	shader->precision = precision;
	shader->source = source = add_opengl_header(true, precision, source);
	shader->shader = 0;
	return shader;
}

} // End namespace gfx

} // End namespace noo
