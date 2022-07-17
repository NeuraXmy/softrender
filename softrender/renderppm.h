#ifndef RENDER_PPM_H
#define RENDER_PPM_H

#include "rendertarget.h"

class RenderPpm : public RenderTarget
{
public:
	
	RenderPpm(int w, int h);
	
	void save(std::string_view filename);

private:

	int _w;
	int _h;
	
};

#endif 
