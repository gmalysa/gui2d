/**
 * @file 2dgui/iQuadRenderable.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iQuadRenderable.h"

/**
 * Allocate blocks of memory, which will never be reallocated locally, to store
 * all of our vertex color and coordinate locations.
 * @param quads The number of quads to save space for
 */
gui2d::iQuadRenderable::iQuadRenderable(uint16_t quads) : _count(quads), _prevOffset(USHRT_MAX),
		_modified(true), _vCoords(0) {
	_vCoords = static_cast<glm::i16vec3 *>(calloc(1, _count*4*sizeof(glm::i16vec3)));
}

/**
 * Deallocate blocks of memory, make sure to let the QuadRenderer know that we
 * do not exist anymore.
 */
gui2d::iQuadRenderable::~iQuadRenderable(void) {
	free(_vCoords);
}

/**
 * Sets the Z position for all of the quads used by this Renderable
 * @param z The Z position to use for all quads drawn here
 */
void gui2d::iQuadRenderable::setQuadsZ(GLshort z) {
	uint16_t i;
	for (i = 0; i < _count*4; ++i) {
		_vCoords[i].z = z;
	}
}

/**
 * Sets the size information for a specific quad as a series of floating point numbers,
 * doing the conversion  to fixed point internally.
 * @param quad The quad number to set information for
 * @param x The leftmost x position, in normalized coordinates
 * @param y The bottommost y position, in normalized coordinates
 * @param w The width of the quad, in normalized values
 * @param h The height of the quad, in normalized values
 */
void gui2d::iQuadRenderable::setQuadXY(uint16_t quad, float x, float y, float w, float h) {
	GLshort sx, sy, sw, sh;
	sx = static_cast<GLshort>(glm::clamp(x, -1.0f, 1.0f)*SHRT_MAX);
	sy = static_cast<GLshort>(glm::clamp(y, -1.0f, 1.0f)*SHRT_MAX);
	sw = static_cast<GLshort>(glm::clamp(w, -1.0f, 1.0f)*SHRT_MAX);
	sh = static_cast<GLshort>(glm::clamp(h, -1.0f, 1.0f)*SHRT_MAX);
	setQuadXY(quad, sx, sy, sw, sh);
}

/**
 * Sets the size information for a specific quad
 * @param quad The quad number to set information for
 * @param x The leftmost x position, in normalized coordinates
 * @param y The bottommost y position, in normalized coordinates
 * @param w The width of the quad, in normalized values
 * @param h The height of the quad, in normalized values
 */
void gui2d::iQuadRenderable::setQuadXY(uint16_t quad, GLshort x, GLshort y, GLshort w, GLshort h) {
	int i = 4*quad;
	
	_vCoords[i].x = x;
	_vCoords[i].y = y;
	_vCoords[i+1].x = x+w;
	_vCoords[i+1].y = y;
	_vCoords[i+2].x = x+w;
	_vCoords[i+2].y = y+h;
	_vCoords[i+3].x = x;
	_vCoords[i+3].y = y+h;

	_modified = true;
}

/**
 * Set the Z coordinate for a specific quad.
 * @param quad The quad whose Z coordinate is being modified
 * @param z The coordinate to set.
 */
void gui2d::iQuadRenderable::setQuadZ(uint16_t quad, GLshort z) {
	int i = 4*quad;
	_vCoords[i].z = z;
	_vCoords[i+1].z = z;
	_vCoords[i+2].z = z;
	_vCoords[i+3].z = z;
	_modified = true;
}

/**
 * Set the position of a quad including Z offset
 * @param quad The quad whose position should be updated
 * @param coords The coordinates for the bottom left; The Z is carried through for all coordinates
 * @param w The width, normalized
 * @param h The height, normalized
 */
void gui2d::iQuadRenderable::setQuadPosition(uint16_t quad, const glm::i16vec3& coords, GLshort w, GLshort h) {
	setQuadZ(quad, coords.z);
	setQuadXY(quad, coords.x, coords.y, w, h);
}

/**
 * Rendering method that copies over quad data to a given array
 * @param vCoords The vertex coordinate array to copy our vertex data to
 * @param offset Our offset within the master array
 * @param force Should we force an update (master array was corrupted for some reason)
 * @return Did we change the input arrays?
 */
bool gui2d::iQuadRenderable::render(glm::i16vec3 *vCoords, uint16_t offset, bool force) {
	if (force || _modified || (offset != _prevOffset)) {
		std::memcpy(vCoords, _vCoords, _count*4*sizeof(glm::i16vec3));

		_modified = false;
		_prevOffset = offset;
		return true;
	}

	return false;
}
