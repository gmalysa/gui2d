#ifndef _2DGUI_FONT_H_
#define _2DGUI_FONT_H_
/**
 * @class gui2d::Font
 * Font container for the 2D GUI class.
 * @todo Support loading fonts from memory, not just from files
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <string>

// Project definitions
#include "2dgui/Manager.h"

namespace gui2d {

class Font {
public:
	/**
	 * char_info is used to store the necessary information to draw a quad representing
	 * a specific character using this Font
	 */
	struct char_info {
		// Screen normalized
		GLushort ax;	//!< advance.x

		// Screen normalized coordinates for positioning
		GLushort sbw;	//!< bitmap.width;
		GLushort bl;	//!< bitmap_left;

		// Texture normalized which get copied verbatim
		GLushort tx;	//!< x offset of glyph within the texture
		GLushort txEnd;	//!< end x offset of glyph within texture
	};

private:
	int _id;
	bool _init;
	GLuint _texture;
	float _height;
	FT_Bool _kerning;

	GLshort _texHeight;			// Normalized to screen coordinates
	GLshort _maxDescender;		// Absolute value of the max descent value
	GLushort _maxWidth;			// Maximum character width using advance.x

	char_info _info[128];

	gui2d::Manager *_manager;
	FT_Face _face;

public:
	/**
	 * The constructor is totally empty, because this font does nothing until
	 * it is explicitly loaded, since constructors can't produce error messages.
	 */
	Font(void) {}
	~Font(void);

	GLuint getMaxStringWidth(int count);
	GLuint getStringWidth(const std::string& text);
	float getStringWidthf(const std::string& text);
	GLshort getKerning(FT_UInt left, FT_UInt right);

	/**
	 * Return the font ID used by the manager to refer to this font
	 * @return Manager's ID for this font instance
	 */
	int getId(void) const { return _id; }

	/**
	 * Return the texture id that serves as a handle to the opengl texture
	 * @return OpenGL Texture ID for this font atlas
	 */
	GLuint getTextureId(void) const { return _texture; }
	
	/**
	 * Get the char_info struct for a specific character
	 * @param c The character whose info we need to look up
	 * @return pointer to a character info struct for the character
	 */
	const char_info *getCharInfo(unsigned char c) const { return &_info[c]; }

	/**
	 * Get the height of this font when drawn on screen, in normalized coordinates
	 * @return Height of this font as a float, in normalized coordinates
	 */
	float getHeight(void) const { return _height; }

	/**
	 * Get the height of this texture in normalized coordinates. I'm not sure
	 * that this is different from getHeight(), aside from already being a short
	 * @return The texture height in normalized coordinates, already as a short
	 */
	GLshort getTexHeight(void) const { return _texHeight; }

	/**
	 * Get the absolute value of the maximum descent below the baseline. Alternatively,
	 * this could be interpreted as the offset of the baseline above the bottom of
	 * the texture.
	 * @return The absolute value of the maximum descender below the font baseline, normalized
	 */
	GLshort getMaxDescender(void) const { return _maxDescender; }

public:
	static Font *loadFont(int id, gui2d::Manager *manager, const std::string& path, int size, std::ostream& err);
};

};

#endif
