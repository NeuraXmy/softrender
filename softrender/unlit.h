#ifndef UNLIT_H
#define UNLIT_H

#include "shader.h"
#include "maths.h"
#include "texture.h"

namespace Unlit
{
	enum
	{
		ATTR_position,
		ATTR_texcoord,
		ATTR_NUM
	};
	enum
	{
		VARY_position,
		VARY_texcoord,
		VARY_NUM
	};

	class VS : public VertexShader
	{
	public:

		void load_uniforms() override;
		
		void run(const VSIn& in, VSOut& out) override;

	};

	class FS : public FragmentShader
	{
	public:

		void load_uniforms() override;
		
		void run(const FSIn& in, FSOut& out) override;
	};

	inline ShaderProgram program = {
		std::make_shared<VS>(),
		std::make_shared<FS>(),
		VARY_NUM
	};
}


#endif
