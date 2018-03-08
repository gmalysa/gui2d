/**
 * @file 2dgui/iUntexturedQuadRenderable.cpp
 * @todo license/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <cstring>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iQuadRenderable.h"
#include "2dgui/iUntexturedQuadRenderable.h"

/**
 * Constructor initializes data structures necessary to store untextured quad information
 * @param quads Number of quads to create space for, that this will draw
 */
gui2d::iUntexturedQuadRenderable::iUntexturedQuadRenderable(uint16_t quads) : iQuadRenderable(quads), _vColors(0) {
	_vColors = static_cast<glm::u8vec4 *>(calloc(1, _count*4*sizeof(glm::u8vec4)));
}

/**
 * Destructor frees the color storage space
 */
gui2d::iUntexturedQuadRenderable::~iUntexturedQuadRenderable(void) {
	free(_vColors);
}

/**
 * Set the color of a quad using 0-255 normalized values
 * @param quad The quad whose color should be changed
 * @param color The color to set, including alpha component.
 */
void gui2d::iUntexturedQuadRenderable::setQuadColor(uint16_t quad, const glm::u8vec4& color) {
	int i = 4*quad;
	_vColors[i] = color;
	_vColors[i+1] = color;
	_vColors[i+2] = color;
	_vColors[i+3] = color;
	_modified = true;
}

/**
 * Set the color of a quad using 0-1 normalized float values, which are converted to bytes
 * @param quad The Quad whose color should be changed
 * @param color The color to set for this quad
 */
void gui2d::iUntexturedQuadRenderable::setQuadColor(uint16_t quad, const glm::vec4& color) {
	uint8_t sr, sb, sg, sa;
	sr = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f)*255);
	sg = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f)*255);
	sb = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f)*255);
	sa = static_cast<uint8_t>(glm::clamp(color.a, 0.0f, 1.0f)*255);
	setQuadColor(quad, glm::u8vec4(sr, sg, sb, sa));
}

/**
 * Set only the RGB components of a quad's color, without changing the alpha value
 * @param quad Which quad to update
 * @param color 0-255 RGB values to update
 */
void gui2d::iUntexturedQuadRenderable::setQuadColorRGB(uint16_t quad, const glm::u8vec3& color) {
	int i = 4*quad;
	_vColors[i] = glm::u8vec4(color, _vColors[i].a);
	_vColors[i+1] = glm::u8vec4(color, _vColors[i+1].a);
	_vColors[i+2] = glm::u8vec4(color, _vColors[i+2].a);
	_vColors[i+3] = glm::u8vec4(color, _vColors[i+3].a);
	_modified = true;
}

/**
 * Set only the RGB component using normalized float values for color
 * @param quad Which quad's RGB color to update
 * @param color 0-1 float values for RGB
 */
void gui2d::iUntexturedQuadRenderable::setQuadColorRGB(uint16_t quad, const glm::vec3& color) {
	uint8_t sr, sg, sb;
	sr = static_cast<uint8_t>(glm::clamp(color.r, 0.0f, 1.0f)*255);
	sg = static_cast<uint8_t>(glm::clamp(color.g, 0.0f, 1.0f)*255);
	sb = static_cast<uint8_t>(glm::clamp(color.b, 0.0f, 1.0f)*255);
	setQuadColorRGB(quad, glm::u8vec3(sr, sg, sb));
}

/**
 * Set the alpha color component for a quad without changing the others
 * @param quad Which quad's alpha to change
 * @param alpha Alpha component to set
 */
void gui2d::iUntexturedQuadRenderable::setQuadAlpha(uint16_t quad, uint8_t alpha) {
	int i = 4*quad;
	_vColors[i].a = alpha;
	_vColors[i+1].a = alpha;
	_vColors[i+2].a = alpha;
	_vColors[i+3].a = alpha;
	_modified = true;
}

/**
 * Set the alpha color component for a quad using a floating point input
 * @param quad Which quad's alpha to change
 * @param alpha 0-1 float alpha value to use
 */
void gui2d::iUntexturedQuadRenderable::setQuadAlpha(uint16_t quad, float alpha) {
	setQuadAlpha(quad, static_cast<uint8_t>(glm::clamp(alpha, 0.0f, 1.0f)*255));
}

/**
 * Called by the Renderer to get this Renderable to draw its data into
 * the master arrays at a specific offset. Only draws if the offset is
 * different from before.
 * @param vCoords The vertex coordinate array to copy our vertex data to
 * @param vColors The vertex color array to copy our color data to
 * @param offset Our offset within the master array
 * @param force Should we force an update (master array was corrupted for some reason)
 * @return Did we change the input arrays?
 */
bool gui2d::iUntexturedQuadRenderable::render(glm::i16vec3 * __restrict vCoords, glm::u8vec4 * __restrict vColors, uint16_t offset, bool force) {
	if (static_cast<iQuadRenderable*>(this)->render(vCoords, offset, force)) {
		std::memcpy(vColors, _vColors, _count*4*sizeof(glm::u8vec4));
		return true;
	}
	return false;
}
