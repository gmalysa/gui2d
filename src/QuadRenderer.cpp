/**
 * @file 2dgui/QuadRenderer.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

// Project definitions
#include "Shader.h"
#include "2dgui/gui2d.h"
#include "2dgui/QuadRendererBase.h"
#include "2dgui/QuadRenderer.h"
#include "2dgui/iQuadRenderable.h"

/**
 * Constructor for the QuadRenderer, initializes the opengl resources necessary.
 * @param s Object representing the shader program that is to be used for rendering
 */
gui2d::QuadRenderer::QuadRenderer(Shader *s) : QuadRendererBase<gui2d::QuadRenderer, gui2d::iUntexturedQuadRenderable>(s), _vColors(0) {
	// Get shader attribute location information
	int s_vert = s->getAttribLocation("in_vert");
	int s_color = s->getAttribLocation("in_color");

	// Create buffers
	glBindVertexArray(_vao);
	glGenBuffers(1, &_colorVBO);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	glVertexAttribPointer(s_vert, 3, GL_SHORT, GL_TRUE, sizeof(glm::i16vec3), 0);
	glEnableVertexAttribArray(s_vert);

	glBindBuffer(GL_ARRAY_BUFFER, _colorVBO);
	glVertexAttribPointer(s_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(glm::u8vec4), 0);
	glEnableVertexAttribArray(s_color);

	glBindVertexArray(0);
}

/**
 * Destructor for the QuadRenderer deallocates the buffers used and frees OpenGL memory
 */
gui2d::QuadRenderer::~QuadRenderer(void) {
	// Deallocate the opengl buffers
	glDeleteBuffers(1, &_colorVBO);

	// Free up any memory that was allocated for coordinate arrays
	if (_bufferSize) {
		free(_vColors);
	}
}

/**
 * Ensure that the coordinate arrays are large enough to handle a given number of
 * entries all at once, to limit the number of memory operations that must be
 * performed.
 * @param quads The number of quads we need to be able to store
 */
void gui2d::QuadRenderer::resizeBuffers(uint16_t quads) {
	_vColors = static_cast<glm::u8vec4 *>(realloc(_vColors, quads*4*sizeof(glm::u8vec4)));
}

/**
 * Passes the render call forward to one item pointed to by an iterator
 * @param iter The iterator pointing to the item to pass render() to
 * @param offset The array offset to use
 * @return Passes back the iterator's render response
 */
bool gui2d::QuadRenderer::renderItem(RenderableIter& iter, uint16_t offset) {
	return (*iter)->render(&_vCoords[4*offset], &_vColors[4*offset], offset, false);
}

/**
 * Pushes color buffer data to the gpu, overwriting the already loaded vbo for colors
 */
void gui2d::QuadRenderer::updateBuffers(void) {
	glBindBuffer(GL_ARRAY_BUFFER, _colorVBO);
	glBufferData(GL_ARRAY_BUFFER, _count*sizeof(glm::u8vec4)*4, _vColors, GL_DYNAMIC_DRAW);
}

/**
 * Draws the data pointed to by our index buffer, which happens in one call for the untextured
 * stuff.
 */
void gui2d::QuadRenderer::drawElements(void) {
	glDrawElements(GL_TRIANGLES, 6*_count, GL_UNSIGNED_SHORT, 0);
}
