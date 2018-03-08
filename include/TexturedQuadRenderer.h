#ifndef _TEXTURED_QUAD_RENDERER_H_
#define _TEXTURED_QUAD_RENDERER_H_
/**
 * @class gui2d::TexturedQuadRenderer
 * This handles rendering textured quads, which use a different shader program
 * from untextured quads.
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <stdint.h>

// Project definitions
#include "sks.h"
#include "Singleton.h"
#include "2dgui/gui2d.h"
#include "2dgui/QuadRendererBase.h"
#include "2dgui/iTexturedQuadRenderable.h"

namespace gui2d {

class TexturedQuadRenderer : public QuadRendererBase<TexturedQuadRenderer, iTexturedQuadRenderable>,
								public Singleton<TexturedQuadRenderer> {
private:
	glm::u16vec3 *_tCoords;
	GLuint _textureVBO;
	GLint _gs_tex;

public:
	TexturedQuadRenderer(Shader *s);
	~TexturedQuadRenderer(void);

	void resizeBuffers(uint16_t quads);
	bool renderItem(RenderableIter& iter, uint16_t offset);
	void updateBuffers(void);
	void drawElements(void);
};

};

#endif
