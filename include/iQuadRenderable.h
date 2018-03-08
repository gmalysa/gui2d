#ifndef _IQUAD_RENDERABLE_H_
#define _IQUAD_RENDERABLE_H_
/**
 * @class gui2d::iQuadRenderable
 * The interface iQuadRenderable describes an object that allocates and handles
 * at least some of its rendering through quads. This handles the position tracking
 * for the quads but does not dictate any of their other properties.
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <stdint.h>

// Project definitions
#include "2dgui/gui2d.h"

namespace gui2d {

class iQuadRenderable {
protected:
	uint16_t _count;			//! Count of quads rendered

	uint16_t _prevOffset;		//! Offset that we previously rendered to
	bool _modified;				//! Have the vertex or color data been modified?
	
	glm::i16vec3 *_vCoords;		//! Local copy of vertex coordinates

public:
	iQuadRenderable(uint16_t quads);
	~iQuadRenderable(void);

	// Interface for subclasses to draw the quad position information
	void setQuadsZ(GLshort z);
	void setQuadXY(uint16_t quad, float x, float y, float w, float h);
	void setQuadXY(uint16_t quad, GLshort x, GLshort y, GLshort w, GLshort h);
	void setQuadZ(uint16_t quad, GLshort z);
	void setQuadPosition(uint16_t quad, const glm::i16vec3& coords, GLshort w, GLshort h);

	bool render(glm::i16vec3 *vCoords, uint16_t offset, bool force);

	/**
	 * @return Number of quads this Renderable expects
	 */
	uint16_t getQuadCount(void) const { return _count; }
};

};

#endif
