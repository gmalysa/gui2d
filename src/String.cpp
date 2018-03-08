/**
 * @file 2dgui/String.cpp
 * @todo License/copyright statement
 */

// Standard headers
// None

// Project definitions
#include "2dgui/String.h"
#include "2dgui/Manager.h"

/**
 * This is the only constructor that should be used, to configure the necessary
 * rendering information immediately.
 * @param m Manager to use to track this string
 * @param font Font to use to render the string (this can be changed later)
 */
gui2d::String::String(gui2d::Font* font) : _font(font) {
	init();
}

/**
 * Initialization routine sets all of our default member variable data to avoid repeating it
 */
void gui2d::String::init(void) {
	_init = _gInit = _modified = false;
	_x = _y = 0.0f;
	_vertexCount = _indexCount = _strLen = _maxCount = 0;
	_color = glm::vec4(1.0f);
	_bMinX = _bMinY = SHRT_MIN;
	_bMaxX = _bMaxY = SHRT_MAX;
	_vertcoords = NULL;
	_texcoords = NULL;
	_index = NULL;
}

/**
 * Destructor, cleans up resources based on initialization state at call
 */
gui2d::String::~String(void) {
	gui2d::Manager::getSingleton().removeString(this);
	
	if (_init) {
		delete[] _vertcoords;
		delete[] _texcoords;
		delete[] _index;
	}

	if (_gInit) {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &_vao);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(2, _vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &_ibo);
	}
}

/**
 * Check for capacity in the currently allocated arrays
 * @param Capacity requested
 * @return bool True if the capacity is high enough
 */
bool gui2d::String::hasCapacity(int count) {
	return count <= _maxCount;
}

/**
 * Double the allocated space and copy the count elements to the new buffer
 * @param minCapacity The minimum capacity needed
 * @param copyCount The number of values to copy from the previous buffers
 */
void gui2d::String::increaseCapacity(int minCapacity, int copyCount) {
	int newCap = _maxCount*2;
	glm::i16vec2 *newVert;
	glm::u16vec2 *newTex;
	GLushort* newIndex;

	if (_maxCount*2 < minCapacity)
		newCap = minCapacity+1;
	else
		newCap = _maxCount*2;

	// Create new arrays
	newVert = new glm::i16vec2[newCap*4];
	newTex = new glm::u16vec2[newCap*4];
	newIndex = new GLushort[newCap*6];

	// Copy over values
	if (_init) {
		memcpy(newVert, _vertcoords, copyCount*4*sizeof(glm::i16vec2));
		memcpy(newTex, _texcoords, copyCount*4*sizeof(glm::u16vec2));
		memcpy(newIndex, _index, copyCount*6*sizeof(GLushort));

		// Clean up memory that we are ditching
		delete[] _vertcoords;
		delete[] _texcoords;
		delete[] _index;
	}

	// Save pointers
	_vertcoords = newVert;
	_texcoords = newTex;
	_index = newIndex;

	// Update our maximum containable count
	_maxCount = newCap;
	_init = true;
}

/**
 * Translate this string by a fixed amount in normalized coordinates; redraw
 * @param normX The amount to translate x by, in normalized coordinates
 * @param normY The amount to translate y by, in normalized coordinates
 * @return Reference to the string to allow chaining
 */
gui2d::String& gui2d::String::translate(float normX, float normY) {
	return setPosition(_x+normX, _y+normY);
}

/**
 * Update the opacity methods; we don't use the parent transparent approach because
 * opacity is stored as part of our color vec4.
 * @param alpha
 */
void gui2d::String::setOpacity(float alpha) {
	_color.w = alpha;
}

/**
 * Update the opacity methods; we don't use a glubyte to store alpha but it is part of
 * the interface
 */
void gui2d::String::setOpacity(GLubyte alpha) {
	setOpacity(alpha/255.0f);
}

/**
 * Adjust this string's position, redrawing it in order to do so
 * @param normX The new x position, in normalized coordinates
 * @param normY The new y position, in normalized coordinates
 * @return Reference to the string to allow chaining
 */
gui2d::String& gui2d::String::setPosition(float normX, float normY) {
	// Initialize our member variables for this string
	_vertexCount = 0;
	_indexCount = 0;

	// Calculate initial location based on normalized coordinates
	_curX = static_cast<GLshort>(normX * (1 << 15));
	_curY = static_cast<GLshort>(normY * (1 << 15));
	_x = normX;
	_y = normY;
	_startX = _curX;
	_startY = _curY;

	// Actually draw the string now
	findPenDraw(_source);

	return *this;
}

/**
 * Provide a normalized float maximum x coordinate for clipping
 * @param normMaxX The maximum x value of a displayed string
 */
void gui2d::String::setMaxX(float normMaxX) {
	if (normMaxX > 1.0f)
		_bMaxX = SHRT_MAX;
	else if (normMaxX < -1.0f)
		_bMaxX = SHRT_MIN;
	else
		_bMaxX = static_cast<GLint>(normMaxX * (1 << 15));
}

/**
 * Provide a normalized float maximum y coordinate for clipping
 * @param normMaxY The maximum y value of a displayed string
 */
void gui2d::String::setMaxY(float normMaxY) {
	if (normMaxY > 1.0f)
		_bMaxY = SHRT_MAX;
	else if (normMaxY < -1.0f)
		_bMaxY = SHRT_MIN;
	else
		_bMaxY = static_cast<GLint>(normMaxY * (1 << 15));
}

/**
 * Draw the string from scratch, set init flag, save important rendering parameters, using
 * the saved x and y position data
 * @param source The source text to draw
 */
void gui2d::String::drawText(const std::string& source) {
	drawText(source, _x, _y);
}

/**
 * Draws the string from scratch, saves rendering parameters, etc. using a new position
 * @param source The source text to draw
 * @param normX The normalized x coordinate of the new string
 * @param normY The normalized y coordinate of the new string
 */
void gui2d::String::drawText(const std::string& source, float normX, float normY) {
	// Make sure our buffers are big enough to hold the desired string
	if (!hasCapacity(source.length())) {
		increaseCapacity(source.length()+1, 0);
	}

	// Allocate some space, we need four unique vertices (+textures) per character (at most)
	// in order to account for different texture mappings at "shared" vertices, and
	// we need six indices to render the two triangles per character
	_strLen = source.length();
	_maxCount = _strLen;

	// Initialize our member variables for this string
	_vertexCount = 0;
	_indexCount = 0;
	_source = std::string(source);

	// Calculate initial location based on normalized coordinates
	_curX = static_cast<GLshort>(normX * (1 << 15));
	_curY = static_cast<GLshort>(normY * (1 << 15));
	_x = normX;
	_y = normY;
	_startX = _curX;
	_startY = _curY;

	// Actually draw the string now
	findPenDraw(source);
}

/**
 * Helper method for rendering a single character into the appropriate arrays.
 * Note that this does not check for memory!
 * @param ci Reference to character info struct for the character being drawn
 * @param curX The current X coordinate to draw the character at
 * @param curY The current Y coordinate to draw the character at
 * @param indexOffset The offset of the index buffer to write to for this character's quad
 * @param vertexOffset The offset of the vertex buffer to write to for this character's quad
 */
void gui2d::String::drawChar(const gui2d::Font::char_info& ci, GLshort curX, GLshort curY, int indexOffset, int vertexOffset) {
	GLshort mx, my;

	mx = curX + ci.bl;
	my = curY;

	// Skip empty characters (this means we don't render spaces, for instance)
	if (ci.sbw == 0)
		return;

	// Set up vertex and texture coordinates, going counter clockwise
	_index[indexOffset] = vertexOffset;
	_index[indexOffset+3] = vertexOffset;
	_vertcoords[vertexOffset] = glm::i16vec2(mx, my);
	_texcoords[vertexOffset] = glm::u16vec2(ci.tx, USHRT_MAX);

	_index[indexOffset+1] = vertexOffset+1;
	_vertcoords[vertexOffset+1] = glm::i16vec2(mx + ci.sbw, my);
	_texcoords[vertexOffset+1] = glm::u16vec2(ci.txEnd, USHRT_MAX);

	_index[indexOffset+2] = vertexOffset+2;
	_index[indexOffset+4] = vertexOffset+2;
	_vertcoords[vertexOffset+2] = glm::i16vec2(mx + ci.sbw, my + _font->getTexHeight());
	_texcoords[vertexOffset+2] = glm::u16vec2(ci.txEnd, 0);

	_index[indexOffset+5] = vertexOffset+3;
	_vertcoords[vertexOffset+3] = glm::i16vec2(mx, my + _font->getTexHeight());
	_texcoords[vertexOffset+3] = glm::u16vec2(ci.tx, 0);
}

/**
 * Starting with the current values for all counts and positions, update our counts and pen location
 * for the current string (used to recalculate pen position when saving part of the string)
 * @param source The string to find the pen state for.
 */
void gui2d::String::findPen(const std::string& source) {
	const char *c;
	const char *p = 0;
	const gui2d::Font::char_info *ci;
	GLint tempX = 0;

	for (c = source.c_str(); *c != 0; c++) {
		ci = _font->getCharInfo(*c);

		// Account for kerning
		if (p) {
			tempX = static_cast<GLint>(_curX) + _font->getKerning(*p, *c) + ci->ax;
		}
		else {
			tempX = static_cast<GLint>(_curX) + ci->ax;
		}

		// Check if we're going to go out of bounds by advancing our pointer, if so stop drawing
		if (tempX > _bMaxX) {
			return;
		}

		// Update our "pen" for where to start the next character
		_curX = static_cast<GLshort>(tempX);

		if (ci->sbw == 0) {
			// Disable kerning for next iteration
			p = 0;
			continue;
		}

		_vertexCount += 4;
		_indexCount += 6;
		p = c;
	}
}

/**
 * Same thing as the pen finding method, except this draws the string as well.
 * @param source The string to find the pen+draw for
 */
void gui2d::String::findPenDraw(const std::string& source) {
	const char *c;
	const char *p = 0;
	const gui2d::Font::char_info *ci;
	GLint tempX = 0;

	_modified = true;

	for (c = source.c_str(); *c != 0; ++c) {
		ci = _font->getCharInfo(*c);

		// Account for kerning
		if (p) {
			tempX = static_cast<GLint>(_curX) + _font->getKerning(*p, *c) + ci->ax;
		}
		else {
			tempX = static_cast<GLint>(_curX) + ci->ax;
		}

		// Check if we're going to go out of bounds by advancing our pointer, if so stop drawing
		if (tempX > _bMaxX) {
			return;
		}

		drawChar(*ci, _curX, _curY, _indexCount, _vertexCount);

		// Update our "pen" for where to start the next character
		_curX = static_cast<GLshort>(tempX);

		// Skip empty characters (in either dimension, this means we don't render spaces, for instance)
		if (ci->sbw == 0) {
			p = 0;
			continue;
		}

		_vertexCount += 4;
		_indexCount += 6;
		p = c;
	}
}

/**
 * Append a string to this one
 * @param source The string to append
 */
void gui2d::String::append(const std::string& source) {
	// First ensure we have capacity
	if (!hasCapacity(_source.length() + source.length())) {
		increaseCapacity(_source.length() + source.length(), _strLen);
	}

	// Now fill in the characters for our new string
	findPenDraw(source);

	// Save the string to our local copy
	_source.append(source);
	_strLen = _source.length();
}

/**
 * Remove all of a string after the offset given
 * @param start The starting character index to remove from (to end)
 */
void gui2d::String::remove(int start) {
	remove(start, _strLen);
}

/**
 * Remove part of a string, moving the characters after the substring down into
 * the newly vacated area
 * @param start The starting character index to remove from
 * @param length The number of characters to remove
 */
void gui2d::String::remove(int start, int length) {
	std::string tail, front;

	// Reset string info
	_curX = _startX;
	_curY = _startY;
	_vertexCount = 0;
	_indexCount = 0;

	// If we're removing from the end, this is a lot faster
	if (start+length >= _strLen) {
		// Update the metrics
		_source = _source.substr(0, start);
		_strLen = start;
		findPen(_source);
	}
	else {
		// Get the two string bits that we're keeping
		tail = _source.substr(start+length, std::string::npos);
		front = _source.substr(0, start);

		// Calculate the pen location based on the retained portion
		findPen(front);

		// Now, draw in the tail in the right place
		findPenDraw(tail);
		
		// Update our source string and count information
		_source = front.append(tail);
		_strLen = _source.length();
	}
}

/**
 * Insert a string at a given offset
 * @param source The string to insert
 * @param offset The character offset to insert after
 */
void gui2d::String::insert(const std::string& source, int offset) {
	std::string front, tail;

	// Ensure we have capacity for both strings
	if (!hasCapacity(_source.length() + source.length())) {
		increaseCapacity(_source.length() + source.length(), offset);
	}

	// Set up all of our strings
	front = _source.substr(0, offset);
	tail = _source.substr(offset, std::string::npos);

	// Zero counters for new draw
	_curX = _startX;
	_curY = _startY;
	_vertexCount = 0;
	_indexCount = 0;

	// Update/draw each string component
	findPen(front);
	findPenDraw(source);
	findPenDraw(tail);

	// Save new metrics
	_source = front.append(source).append(tail);
	_strLen = _source.length();
}

/**
 * Initialize the graphics the first time that this is rendered
 */
void gui2d::String::graphicsInit(void) {
	gui2d::Manager& m = gui2d::Manager::getSingleton();

	// If already initialized, delete these buffers
	if (_gInit) {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &_vao);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(2, _vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &_ibo);
	}

	// Generate a vertex array object for our buffers
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	
	// Generate the vertex buffer that will store our vertices and texture coordinates
	glGenBuffers(2, _vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	glVertexAttribPointer(m.getTSInVertLocation(), 2, GL_SHORT, GL_TRUE, sizeof(glm::i16vec2), 0);
	glEnableVertexAttribArray(m.getTSInVertLocation());
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
	glVertexAttribPointer(m.getTSInTexLocation(), 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(glm::u16vec2), 0);
	glEnableVertexAttribArray(m.getTSInTexLocation());

	// Generate the index buffer object
	glGenBuffers(1, &_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

	glBindVertexArray(0);

	_gInit = true;
}

/**
 * Render function. For now this is kind of dumb and will regenerate+rebind VBOs on each render call.
 */
void gui2d::String::render(void) {
	// Generate buffers one time if they haven't been
	if (!_gInit)
		graphicsInit();

	// Don't draw anything if this string isn't visible
	if (!_visible)
		return;

	// Update the buffer data if we've been modified
	glBindVertexArray(_vao);
	if (_modified) {
		glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, _vertexCount*sizeof(glm::i16vec2), _vertcoords, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, _vertexCount*sizeof(glm::u16vec2), _texcoords, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexCount*sizeof(GLushort), _index, GL_DYNAMIC_DRAW);
	}

	// Apply the user's chosen color and Z-ordering
	gui2d::Manager& m = gui2d::Manager::getSingleton();
	glUniform4fv(m.getTSUniformColorLocation(), 1, glm::value_ptr(_color));
	glUniform1f(m.getTSUniformZLocation(), _z);
	
	// Render using the index buffer
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	_modified = false;
}
