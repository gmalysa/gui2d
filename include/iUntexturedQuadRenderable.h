#ifndef _IUNTEXTURED_QUAD_RENDERABLE_H_
#define _IUNTEXTURED_QUAD_RENDERABLE_H_
/**
 * @class gui2d::iUntexturedQuadRenderable
 * This handles quads that do not use textures; as such they use colors instead.
 * They have a dedicated shader and untextured quad renderer.
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <stdint.h>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iQuadRenderable.h"

namespace gui2d {

class iUntexturedQuadRenderable : public iQuadRenderable {
private:
	glm::u8vec4 *_vColors;		//! Local copy of vertex colors

public:
	iUntexturedQuadRenderable(uint16_t quads);
	~iUntexturedQuadRenderable(void);

	// Subclasses can set colors
	void setQuadColor(uint16_t quad, const glm::u8vec4& color);
	void setQuadColor(uint16_t quad, const glm::vec4& color);
	void setQuadColorRGB(uint16_t quad, const glm::u8vec3& color);
	void setQuadColorRGB(uint16_t quad, const glm::vec3& color);
	void setQuadAlpha(uint16_t quad, uint8_t alpha);
	void setQuadAlpha(uint16_t quad, float alpha);

	bool render(glm::i16vec3 *vCoords, glm::u8vec4 *vColors, uint16_t offset, bool force);
};

};

#endif
