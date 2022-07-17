#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include <memory>
#include "renderdevice.h"
#include "framebuffer.h"

class Model;

class RenderTarget
{
public:

	virtual ~RenderTarget() = default;
	
	void clear(const Color4& color, float depth = std::numeric_limits<float>::max());

	void draw(std::shared_ptr<RenderDevice> device, const VertexArray& vertices);

	void draw(std::shared_ptr<RenderDevice> device, Model& model);

protected:

	std::unique_ptr<FrameBuffer> _framebuffer = nullptr;
	
};

#endif