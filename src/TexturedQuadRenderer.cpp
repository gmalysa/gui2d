/**
 * @file 2dgui/TexturedQuadRenderer.cpp
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
#include "2dgui/iTexturedQuadRenderable.h"
#include "2dgui/TexturedQuadRenderer.h"

/**
 * Assign vertex attribute pointers and create VBO for storing texture coordinates
 * @param s The shader program to use for textured quads
 */
gui2d::TexturedQuadRenderer::TexturedQuadRenderer(Shader *s) : QuadRendererBase<gui2d::TexturedQuadRenderer, gui2d::iTexturedQuadRenderable>(s), _tCoords(0), _textureVBO(0) {
	int s_vert = s->getAttribLocation("in_vert");
	int s_tex = s->getAttribLocation("in_tex");
	_gs_tex = s->getUniformLocation("tex");

	// Create buffers
	glBindVertexArray(_vao);
	glGenBuffers(1, &_textureVBO);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	glVertexAttribPointer(s_vert, 3, GL_SHORT, GL_TRUE, sizeof(glm::i16vec3), 0);
	glEnableVertexAttribArray(s_vert);

	glBindBuffer(GL_ARRAY_BUFFER, _textureVBO);
	glVertexAttribPointer(s_tex, 3, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(glm::u16vec3), 0);
	glEnableVertexAttribArray(s_tex);

	glBindVertexArray(0);
}

/**
 * Deallocate our buffers and release opengl resources
 */
gui2d::TexturedQuadRenderer::~TexturedQuadRenderer(void) {
	glDeleteBuffers(1, &_textureVBO);

	if (_bufferSize) {
		free(_tCoords);
	}
}

/**
 * Ensure that the coordinate arrays are large enough to handle a given number of
 * entries all at once, to limit the number of memory operations that must be
 * performed.
 * @param quads The number of quads we need to be able to store
 */
void gui2d::TexturedQuadRenderer::resizeBuffers(uint16_t quads) {
	_tCoords = static_cast<glm::u16vec3*>(realloc(_tCoords, quads*4*sizeof(glm::u16vec3)));
}

/**
 * Calls the appropriate render method for one specific item to update our arrays
 * @param iter The iterator pointing to the item to pass render() to
 * @param offset The array offset to use
 * @return Passes back the iterator's render response
 */
bool gui2d::TexturedQuadRenderer::renderItem(RenderableIter& iter, uint16_t offset) {
	return (*iter)->render(&_vCoords[4*offset], &_tCoords[4*offset], offset, false);
}

/**
 * Pushes the texture coordinate buffer to the gpu
 */
void gui2d::TexturedQuadRenderer::updateBuffers(void) {
	glBindBuffer(GL_ARRAY_BUFFER, _textureVBO);
	glBufferData(GL_ARRAY_BUFFER, _count*sizeof(glm::u16vec3)*4, _tCoords, GL_DYNAMIC_DRAW);
}

/**
 * Rebinds textures a bunch and passes out several draw elements calls
 */
void gui2d::TexturedQuadRenderer::drawElements(void) {
	RenderableIter iter;
	GLuint lastTextureId = 0;
	uint16_t currentQuad = 0;
	uint16_t offset = 0;

	// Make sure our texture is enabled and our uniform is set properly
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(_gs_tex, 0);

	// Iterate over renderables and draw them
	for (iter = _drawItems.begin(); iter != _drawItems.end(); ++iter) {
		for (currentQuad = 0; currentQuad < (*iter)->getQuadCount(); ++currentQuad) {
			// Only update texture ID if they have changed
			if (lastTextureId != (*iter)->getTextureId(currentQuad)) {
				lastTextureId = (*iter)->getTextureId(currentQuad);
				glBindTexture(GL_TEXTURE_2D, lastTextureId);
			}
		
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(offset*6*sizeof(GLushort)));
			offset += 1;
		}
	}
}
