#ifndef _2DGUI_STRING_H_
#define _2DGUI_STRING_H_
/**
 * A visible string that is displayed on screen
 * TODO: Implement the minimum bounding box for a centralized window around the string
 * TODO: Implement optional word wrapping
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Project definitions
#include "2dgui/Font.h"
#include "2dgui/iTransparent.h"
#include "2dgui/iZOrderable.h"
#include "2dgui/iVisible.h"

namespace gui2d {

class String : public iZOrderable, public iTransparent, public iVisible {
private:
	//
	std::string _source;
	bool _init, _gInit, _modified;
	Font *_font;

	// Coordinates
	float _x, _y;					// Start of the string
	GLshort _startX, _startY;		// Start of string, normalized
	GLshort _curX, _curY;			// End of the string, normalized

	// Bounds for where we are allowed to draw our string (should not emit vertices greater than these)
	GLint _bMinX;
	GLint _bMaxX;
	GLint _bMinY;
	GLint _bMaxY;

	// Buffers to copy to GPU
	glm::i16vec2* _vertcoords;
	glm::u16vec2* _texcoords;
	glm::vec4 _color;
	GLushort* _index;

	// Array size counters
	int _strLen;
	int _maxCount;
	int _vertexCount;
	int _indexCount;
	
	// opengl buffer identifiers
	GLuint _vao;
	GLuint _vbo[2];
	GLuint _ibo;

	// Drawing and management helper methods
	void drawChar(const Font::char_info& ci, GLshort curX, GLshort curY, int indexOffset, int vertexOffset);
	bool hasCapacity(int count);
	void increaseCapacity(int minCapacity, int copyCount);
	void findPen(const std::string& source);
	void findPenDraw(const std::string& source);
	void init(void);

public:
	String(Font* font);
	virtual ~String(void);

	// Accessors
	int length(void) const { return _source.length(); }
	Font* getFont(void) const { return _font; }
	int getVertexCount(void) const { return _vertexCount; }
	int getIndexCount(void) const { return _indexCount; }
	const std::string& getText(void) const { return _source; }

	// Text adjustment
	void drawText(const std::string& source, float normX, float normY);
	void drawText(const std::string& source);
	String& translate(float normX, float normY);
	String& setPosition(float normX, float normY);
	String& setColor(const glm::vec4& color) { _color = color; return *this; }

	// Update the opacity
	void setOpacity(float alpha);
	void setOpacity(GLubyte alpha);

	// Bounding box adjustment and reading
	void setMinX(float minX);
	void setMinY(float minY);
	void setMaxX(float maxX);
	void setMaxY(float maxY);
	float getWidthf(void) const { return _font->getStringWidthf(_source); }
	float getHeightf(void) const { return _font->getHeight(); }
	float getX(void) const { return _x; }
	float getY(void) const { return _y; }

	// Text modification
	void append(const std::string& source);
	void remove(int start);
	void remove(int start, int length);
	void insert(const std::string& source, int start);

	// Rendering
	void graphicsInit(void);
	void render(void);
};

};

#endif
