#ifndef _QUAD_RENDERER_H_
#define _QUAD_RENDERER_H_
/**
 * @class gui2d::QuadRenderer
 * This class tracks and renders all of the quads that use the same
 * shader program. These quads are untextured.
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <stdint.h>

// Project definitions
#include "sks.h"
#include "Singleton.h"
#include "2dgui/gui2d.h"
#include "2dgui/QuadRendererBase.h"
#include "2dgui/iUntexturedQuadRenderable.h"

namespace gui2d {

class QuadRenderer : public QuadRendererBase<QuadRenderer, iUntexturedQuadRenderable>,
						public Singleton<QuadRenderer> {
private:
	glm::u8vec4 *_vColors;		// Coloring is possible on a per-vertex basis
	GLuint _colorVBO;			// OpenGL Vertex Buffer Objects used to store vertices, colors, and indices

public:
	QuadRenderer(Shader *s);
	~QuadRenderer(void);

	void resizeBuffers(uint16_t quads);
	bool renderItem(RenderableIter& iter, uint16_t offset);
	void updateBuffers(void);
	void drawElements(void);
};

};

#endif
