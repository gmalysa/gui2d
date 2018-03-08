/**
 * @file 2dgui/iTexturedQuadRenderable.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <cstring>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iQuadRenderable.h"
#include "2dgui/iTexturedQuadRenderable.h"

/**
 * Construct a textured quad object with a given number of quads
 * @param quads The number of quads to draw
 */
gui2d::iTexturedQuadRenderable::iTexturedQuadRenderable(uint16_t quads) : iQuadRenderable(quads), _tId(0), _tCoords(0) {
	_tId = static_cast<GLuint *>(calloc(1, _count*sizeof(GLuint)));
	_tCoords = static_cast<glm::u16vec3 *>(calloc(1, _count*4*sizeof(glm::u16vec3)));
}

/**
 * Deallocates the memory saved. Does not unbind the textures, because they were bound somewhere else
 */
gui2d::iTexturedQuadRenderable::~iTexturedQuadRenderable(void) {
	free(_tId);
	free(_tCoords);
}

/**
 * Assign texture coordinates to a given quad
 * @param quad The quad number to assign texture coordinates
 * @param minU The minimum U coordinate
 * @param maxU The maximum U coordinate
 * @param minV The minimum V coordinate
 * @param maxV The maximum V coordinate
 */
void gui2d::iTexturedQuadRenderable::setQuadUV(uint16_t quad, float minU, float maxU, float minV, float maxV) {
	GLushort sMinU = static_cast<GLushort>(glm::clamp(minU, 0.0f, 1.0f)*USHRT_MAX);
	GLushort sMaxU = static_cast<GLushort>(glm::clamp(maxU, 0.0f, 1.0f)*USHRT_MAX);
	GLushort sMinV = static_cast<GLushort>(glm::clamp(minV, 0.0f, 1.0f)*USHRT_MAX);
	GLushort sMaxV = static_cast<GLushort>(glm::clamp(maxV, 0.0f, 1.0f)*USHRT_MAX);
	setQuadUV(quad, sMinU, sMaxU, sMinV, sMaxV);
}

/**
 * Assign texture coordinates to a given quad
 * @param quad The quad number to assign texture coordinates
 * @param minU The minimum U coordinate
 * @param maxU The maximum U coordinate
 * @param minV The minimum V coordinate
 * @param maxV The maximum V coordinate
 */
void gui2d::iTexturedQuadRenderable::setQuadUV(uint16_t quad, GLushort minU, GLushort maxU, GLushort minV, GLushort maxV) {
	int i = 4*quad;
	_tCoords[i].x = minU;
	_tCoords[i].y = minV;
	_tCoords[i+1].x = maxU;
	_tCoords[i+1].y = minV;
	_tCoords[i+2].x = maxU;
	_tCoords[i+2].y = maxV;
	_tCoords[i+3].x = minU;
	_tCoords[i+3].y = maxV;
	_modified = true;
}

/**
 * Sets the alpha value for all parts of a given quad
 * @param quad The quad to update
 * @param alpha The alpha value to set
 */
void gui2d::iTexturedQuadRenderable::setQuadAlpha(uint16_t quad, float alpha) {
	setQuadAlpha(quad, static_cast<GLushort>(glm::clamp(alpha, 0.0f, 1.0f)*USHRT_MAX));
}

/**
 * Sets the alpha value for all parts of a given quad
 * @param quad The quad to update
 * @param alpha The alpha value to set
 */
void gui2d::iTexturedQuadRenderable::setQuadAlpha(uint16_t quad, GLushort alpha) {
	int i = 4*quad;
	_tCoords[i].z = alpha;
	_tCoords[i+1].z = alpha;
	_tCoords[i+2].z = alpha;
	_tCoords[i+3].z = alpha;
	_modified = true;
}

/**
 * Renders to the arrays given, but only when necessary
 * @param vCoords The vertex coordinate array to copy our vertex data to
 * @parm tCoords The texture coordinate array to copy our texture data to
 * @param offset Our offset within the master array
 * @param force Should we force an update (master array was corrupted for some reason)
 * @return Did we change the input arrays?
 */
bool gui2d::iTexturedQuadRenderable::render(glm::i16vec3 * __restrict vCoords, glm::u16vec3 * __restrict tCoords, int offset, bool force) {
	if (static_cast<iQuadRenderable*>(this)->render(vCoords, offset, force)) {
		std::memcpy(tCoords, _tCoords, _count*4*sizeof(glm::u16vec3));
		return true;
	}
	return false;
}
