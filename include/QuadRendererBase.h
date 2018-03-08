#ifndef _QUAD_RENDERER_BASE_H_
#define _QUAD_RENDERER_BASE_H_
/**
 * @class gui2d::QuadRendererBase
 * This is a templated base class for rendering subclasses of iQuadRenderable that
 * attempts to unify as much as possible and use CRTP-style programming to expose
 * hooks to allow specific quad renderer types to make small adjustments to the
 * overall rendering algorithm
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <stdint.h>
#include <set>

// Project definitions
#include "sks.h"
#include "Shader.h"

namespace gui2d {

/**
 * @tparam T The derived class that extends this base class and contains implementations
 * @tparam U The iQuadRenderable-descendent to be used for storage
 */
template <typename T, typename U>
class QuadRendererBase {
public:
	typedef std::set<U*> RenderableSet;							//! Set of quad renderable instances to draw
	typedef typename RenderableSet::iterator RenderableIter;	//! Iterator for the set of renderables

protected:
	uint16_t _bufferSize;		//! Size of arrays as allocated in memory
	uint16_t _count;			//! Number of quads that we are going to draw
	bool _updateIndex;			//! Flag indicating whether or not to re-push the index buffer
	
	glm::i16vec3 *_vCoords;		//! Coordinates are stored as x, y, z, but z is constant for a quad
	GLushort *_index;			//! Index buffers are used to reduce the memory requirements
	
	GLuint _vao;				//! OpenGL Vertex Array Object that is used for all quads
	GLuint _vbo[2];				//! OpenGL Vertex Buffer Objects used to store index and vertex data
	Shader *_shader;			//! Shader that is used to draw

	RenderableSet _drawItems;	// List of quad renderables that we should draw each frame

public:
	/**
	 * Initializes as much of the shared state as possible. Derived classes will need
	 * to initialize additional VBOs and bind all VBOs to shader locations
	 * @param s The shader to use to draw these quads
	 */
	QuadRendererBase(Shader *s) : _vCoords(0), _index(0), _shader(s), _bufferSize(0), _count(0),
			_updateIndex(false), _vao(0) {
		// Create buffers
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		glGenBuffers(2, _vbo);

		// Bind the element buffer to vbo
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[1]);
		glBindVertexArray(0);
	}

	/**
	 * Frees top level opengl resources and buffers that we control
	 */
	~QuadRendererBase(void) {
		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(2, _vbo);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &_vao);

		if (_bufferSize > 0) {
			free(_vCoords);
			free(_index);
		}
	}

	/**
	 * Adds the quad renderable to the drawing set
	 * @param r The Renderable to start drawing
	 */
	void show(U *r) {
		std::pair<RenderableIter, bool> result = _drawItems.insert(r);
		if (result.second) {
			_count += r->getQuadCount();
			ensureCapacity(_count);
		}
	}

	/**
	 * Removes the quad renderable from the drawing set.
	 * @param r The Renderable to remove
	 */
	void hide(U *r) {
		size_t count = _drawItems.erase(r);
		if (count == 1)
			_count -= r->getQuadCount();
	}

	/**
	 * Ensure that the coordinate arrays are large enough to handle a given number of
	 * entries all at once, to limit the number of memory operations that must be
	 * performed.
	 * @param quads The number of quads we need to be able to store
	 */
	void ensureCapacity(uint16_t quads) {
		if (quads > _bufferSize) {
			// Reallocate memory
			_vCoords = static_cast<glm::i16vec3 *>(realloc(_vCoords, quads*4*sizeof(glm::i16vec3)));
			_index = static_cast<GLushort *>(realloc(_index, quads*6*sizeof(GLushort)));
			static_cast<T*>(this)->resizeBuffers(quads);

			// Update indices for new values
			for (uint16_t i = _bufferSize; i < quads; ++i) {
				_index[6*i] = 4*i;
				_index[6*i+1] = 4*i + 2;
				_index[6*i+2] = 4*i + 3;
				_index[6*i+3] = 4*i;
				_index[6*i+4] = 4*i + 1;
				_index[6*i+5] = 4*i + 2;
			}

			_bufferSize = quads;
			_updateIndex = true;
		}
	}

	/**
	 * To render, we iterate over the visible quads, update their information in our
	 * buffer if necessary, and then draw them.
	 */
	void render(void) {
		bool updateVBO = false;
		uint16_t offset = 0;
		RenderableIter iter;
	
		// Activate our shader program before doing anything else
		_shader->use();
		glBindVertexArray(_vao);

		// Iterate over our tracked renderables and ask them for state updates
		for (iter = _drawItems.begin(); iter != _drawItems.end(); ++iter) {
			updateVBO = static_cast<T*>(this)->renderItem(iter, offset) || updateVBO;
			offset += (*iter)->getQuadCount();
		}

		// Update GPU memory as appropriate
		if (updateVBO) {
			glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, _count*sizeof(glm::i16vec3)*4, _vCoords, GL_DYNAMIC_DRAW);
			static_cast<T*>(this)->updateBuffers();
		}

		// Index buffers are reloaded separately
		if (_updateIndex) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[2]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _count*sizeof(GLushort)*6, _index, GL_DYNAMIC_DRAW);
		}

		// Draw the elements indexed if we have any visible
		if (_count > 0) {
			static_cast<T*>(this)->drawElements();
		}

		glBindVertexArray(0);
		glUseProgram(0);
	}

};

};

#endif
