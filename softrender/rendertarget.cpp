#include "rendertarget.h"
#include "model.h"

void RenderTarget::clear(const Color4& color, float depth)
{
	_framebuffer->clear_color(color);
	_framebuffer->clear_depth(std::numeric_limits<float>::max());
}

void RenderTarget::draw(std::shared_ptr<RenderDevice> device, const VertexArray& vertices)
{
	device->draw(*_framebuffer, vertices);
}

void RenderTarget::draw(std::shared_ptr<RenderDevice> device, Model& model)
{
	model.draw(device, *_framebuffer);
}
