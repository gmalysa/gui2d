/**
 * @file 2dgui/Font.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <string>

// Project definitions
#include "2dgui/Font.h"

/**
 * Destructor will simply deallocate our texture memory, if it has been allocated
 */
gui2d::Font::~Font(void) {
	if (_init) {
		glDeleteTextures(1, &_texture);
	}
}

/**
 * Factory constructor for Font objects. This should only be called by the Manager in order to
 * ensure consistent id assignments.
 * @todo Is it necessary to close the font face on failure?
 * @param id The id number assigned by the Manager
 * @param manager A pointer to the owning manager
 * @param path The path to the font in question
 * @param size The pixel size to use for this font
 * @param err A stream to write error messages to
 * @return A pointer to the newly loaded Font instance
 */
gui2d::Font *gui2d::Font::loadFont(int id, gui2d::Manager *manager, const std::string& path, int size, std::ostream& err) {
	FT_Library *ft = manager->getFreeTypeLibrary();
	FT_GlyphSlot g;
	gui2d::Font *font;
	unsigned char curChar;
	int w, h, x;
	GLubyte *blankImage;
	FT_Pos maxAscent = 0, minDescent = 0;
	int sWidth = manager->getScreenWidth();
	int sHeight = manager->getScreenHeight();

	// Create new Font instance and configure it
	font = new Font();
	font->_id = id;
	font->_init = false;
	font->_manager = manager;

	// Open font face for reading
	if (FT_New_Face(*ft, path.c_str(), 0, &font->_face)) {
		err << "(gui2d::Font::loadFont()) Could not open font: " << path.c_str() << std::endl;
		delete font;
		return NULL;
	}

	// Configure our font face
	FT_Set_Pixel_Sizes(font->_face, 0, size);
	font->_kerning = FT_HAS_KERNING(font->_face);
	g = font->_face->glyph;

	// Iterate over the characters we plan on displaying and measure their size
	w = h = 0;
	for (curChar = 32; curChar < 128; ++curChar) {
		if (FT_Load_Char(font->_face, curChar, FT_LOAD_RENDER)) {
			err << "(gui2d::Font::loadFont()) Could not load character: " << curChar << std::endl;
			delete font;
			return NULL;
		}

		w += g->bitmap.width;
		maxAscent = std::max(maxAscent, g->metrics.horiBearingY/64);
		minDescent = std::min(minDescent, g->metrics.horiBearingY/64 - g->bitmap.rows);
	}

	// Compute the height of the image from the ascent and descent
	h = maxAscent - minDescent + 1;

	// Now create the texture that we need for our atlas
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &font->_texture);
	glBindTexture(GL_TEXTURE_2D, font->_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Configure texture wrapping and filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// We store in a single channel. This gets interpreted as an alpha value in the shader,
	// but the only single channel option is GL_RED
	blankImage = new GLubyte[w*h];
	memset(blankImage, 0, w*h*sizeof(GLubyte));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, blankImage);
	delete[] blankImage;

	// Iterate over characters again and copy bitmaps to the texture
	x = 0;
	font->_maxWidth = 0;
	for (curChar = 32; curChar < 128; ++curChar) {
		if (FT_Load_Char(font->_face, curChar, FT_LOAD_RENDER))
			continue;
		
		// Store the image so that all the glyphs share the same baseline position
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, static_cast<GLint>(maxAscent - g->bitmap_top), g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		// Save the character information for later use, note that all of these are normalized integers
		// so 0 -> 0.0f, and 65535 -> 1.0f, the other end of the texture in that direction,
		// To accomplish this in an N-bit integer type we multiply the desired value by (2^N/norm), where
		// norm is the maximum value for the normalized data type.
		// Advance is in 1/64 of pixel, so we don't normalize it to the width of the texture;
		// instead we normalize it to the screen dimensions
		font->_info[curChar].ax = NORMALIZE(GLushort, g->advance.x, 10, sWidth);
		font->_info[curChar].sbw = NORMALIZE(GLushort, g->bitmap.width, 16, sWidth);
		font->_info[curChar].bl = NORMALIZE(GLushort, g->bitmap_left, 16, sWidth);
		font->_info[curChar].tx = NORMALIZE(GLushort, x, 16, w);
		font->_info[curChar].txEnd = NORMALIZE(GLushort, x+g->bitmap.width, 16, w);

		// Increment the x offset within the texture by the width of the bitmap
		x += g->bitmap.width;

		// Update the maximum glyph width
		font->_maxWidth = std::max(font->_maxWidth, font->_info[curChar].ax);
	}

	font->_texHeight = NORMALIZE(GLshort, h, 16, sHeight);
	font->_maxDescender = NORMALIZE(GLshort, abs(minDescent), 16, sHeight);
	font->_height = static_cast<float>(2*h)/sHeight;
	font->_init = true;
	return font;
}

/**
 * Get the aboslute maximum width of a string that is N characters long, excluding
 * control characters (tab, newline, etc.)
 * @param count The number of characters in the string
 * @return Normalized length of the string
 */
GLuint gui2d::Font::getMaxStringWidth(int count) {
	return count * _maxWidth;
}

/**
 * Get the normalized screen width of a given string, when rendered with this font. Note
 * that a GLuint is used, rather than GLushort, to allow us to measure string width accurately
 * when a string doesn't fit on screen.
 * @param text The string whose expected display width we should calculate
 * @return Width of the string as displayed by this font
 */
GLuint gui2d::Font::getStringWidth(const std::string& text) {
	const char *c;
	const char *p = 0;
	const gui2d::Font::char_info *ci;
	GLuint tempX = 0;

	for (c = text.c_str(); *c != 0; c++) {
		ci = getCharInfo(*c);
		tempX += ci->ax;

		// Account for kerning
		if (p)
			tempX += getKerning(*p, *c);

		// Save previous character
		p = c;
		
		// Disable kerning for next iteration on zero width glyph
		if (ci->sbw == 0)
			p = 0;
	}

	return tempX;
}

/**
 * Alternatively, get the normalized string width as a float, to simplify the calling code.
 * It is still normalized to the width of the window, but no longer represented in fixed point.
 * @param text The string whose expected display width we should calculate
 * @return Width of the string as displayed by this font, as a float
 */
float gui2d::Font::getStringWidthf(const std::string& text) {
	return getStringWidth(text) / static_cast<float>(1 << 15);
}

/**
 * Retrieve kerning information for a pair of characters, and since we only use horizontal layouts,
 * return the x distance after normalizing to screen units.
 * @param left The left character in the kerning pair
 * @param right The right character in the kerning pair
 * @return The adjustment due to kerning for this pair of characters; it may be positive or negative
 */
GLshort gui2d::Font::getKerning(FT_UInt left, FT_UInt right) {
	FT_Vector kern;
	int sWidth = _manager->getScreenWidth();
	
	if (_kerning) {
		FT_Get_Kerning(_face, left, right, FT_KERNING_DEFAULT, &kern);

		return NORMALIZE(GLshort, kern.x, 10, sWidth);
	}
	
	return 0;
}
