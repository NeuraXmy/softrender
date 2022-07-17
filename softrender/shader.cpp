#include "shader.h"
#include "renderdevice.h"

void Shader::set_device(std::shared_ptr<RenderDevice> device)
{
	_device = device;
}

std::shared_ptr<RenderDevice> Shader::device() const
{
	return _device;
}

ShaderProgram::ShaderProgram(std::shared_ptr<VertexShader> vs, std::shared_ptr<FragmentShader> fs, int varying_num)
	: vertex_shader(vs)
	, fragment_shader(fs)
	, varying_num(varying_num)
{
	
}