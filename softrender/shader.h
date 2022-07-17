#ifndef SHADER_H
#define SHADER_H

#include <memory>
#include <string>
#include <any>
#include "maths.h"
#include "renderdevice.h"


class Shader
{
public:

	void set_device(std::shared_ptr<RenderDevice> device);

	std::shared_ptr<RenderDevice> device() const;

	virtual void load_uniforms() { }

	virtual ~Shader() = default;

protected:

	template<class T>
	T get_uniform(std::string_view name, const T& default_val = T()) const
	{
		return _device->get_shader_uniform<T>(name, default_val);
	}

	bool has_shader_uniform(std::string_view name) const
	{
		return _device->has_shader_uniform(name);
	}

private:

	std::shared_ptr<RenderDevice> _device = nullptr;
	
};

class VertexShader : public Shader
{

public:

	virtual void run(const VSIn& in, VSOut& out) = 0;
	
	virtual ~VertexShader() = default;

};

class FragmentShader : public Shader
{
public:

	virtual void run(const FSIn& in, FSOut& out) = 0;
	
	virtual ~FragmentShader() = default;
	
};

struct ShaderProgram
{
	std::shared_ptr<VertexShader> vertex_shader	 = nullptr;
	std::shared_ptr<FragmentShader> fragment_shader = nullptr;
	int varying_num = 0;

	ShaderProgram() = default;
	ShaderProgram(std::shared_ptr<VertexShader> vs, std::shared_ptr<FragmentShader> fs, int varying_num);
};

#endif

