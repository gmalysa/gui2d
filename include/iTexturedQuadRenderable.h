#ifndef _ITEXTURED_QUAD_RENDERABLE_H_
#define _ITEXTURED_QUAD_RENDERABLE_H_
/**
 * @class gui2d::iTexturedQuadRenderable
 * This completes the quad renderable interface except requires quads to use textures to
 * do rendering. It is handled somewhat separately from the untextured quads but shares
 * core positioning code
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

class iTexturedQuadRenderable : public iQuadRenderable {
private:
	GLuint *_tId;
	glm::u16vec3 *_tCoords;

public:
	iTexturedQuadRenderable(uint16_t quads);
	~iTexturedQuadRenderable(void);

	void setQuadUV(uint16_t quad, float minU, float maxU, float minV, float maxV);
	void setQuadUV(uint16_t quad, GLushort minU, GLushort maxU, GLushort minV, GLushort maxV);
	void setQuadAlpha(uint16_t quad, float alpha);
	void setQuadAlpha(uint16_t quad, GLushort alpha);

	bool render(glm::i16vec3 *vCoords, glm::u16vec3 *tCoords, int offset, bool force);

	/**
	 * Set the texture ID that should be used during rendering
	 * @param quad Which quad to set the texture for
	 * @param tId the new opengl texture id to use
	 */
	void setTextureId(uint16_t quad, GLuint tId) { _tId[quad] = tId; }

	/**
	 * Retrieves the texture ID for a specific quad, useful during rendering
	 * @return The opengl id for a specific quad's texture
	 */
	GLuint getTextureId(uint16_t quad) const { return _tId[quad]; }

};

};

#endif
