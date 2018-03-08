/**
 * @file 2dgui/Manager.cpp
 * @todo License/copyright information
 */

// Standard headers
#include <IL/il.h>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <string>
#include <list>
#include <map>

// Project definitions
#include "Shader.h"
#include "GraphicsEngine.h"
#include "2dgui/Manager.h"
#include "2dgui/Font.h"
#include "2dgui/String.h"
#include "2dgui/InputBox.h"
#include "2dgui/Button.h"
#include "2dgui/QuadTree.h"
#include "2dgui/QuadRenderer.h"
#include "2dgui/TexturedQuadRenderer.h"

/**
 * GUI Manager constructor initializes all of the tracking mechanisms
 * @param ge Pointer to the graphics engine that we care about for this manager
 */
gui2d::Manager::Manager(GraphicsEngine *ge) : _init(false), _qr(0), _tqr(0), _ge(ge), _destructor(NONE) {
	
	glm::vec4 bounds = glm::vec4(0.0f);
	bounds[iMBR::MIN_X] = -1.0f;
	bounds[iMBR::MAX_X] = 1.0f;
	bounds[iMBR::MIN_Y] = -1.0f;
	bounds[iMBR::MAX_Y] = 1.0f;

	_mouseHandlers = new QuadTree<iMouseHandler>(bounds, 0, 1);
	_mouseMotionHandlers = new QuadTree<iMouseMotionHandler>(bounds, 0, 1);
}

/**
 * Destructor cleans up any allocated resources, however.
 */
gui2d::Manager::~Manager() {
	gui2d::FontMapIter fontIter;
	gui2d::FontStringListIter sListIter;
	gui2d::StringListIter stringIter;
	gui2d::StringList* sList;
	gui2d::ButtonListIter buttonIter;

	// Clean up the global renderer resources
	if (_init) {
		FT_Done_FreeType(_ft);
	}

	// Delete input mappings
	delete _mouseHandlers;
	delete _mouseMotionHandlers;

	// Delete all of the inputs
	// TODO: Delete inputs

	// Free the sub-renderers
	free(_qr);
	free(_tqr);

	// Delete all the displayed strings and their container lists
	_destructor = STRINGS;
	for (sListIter = _strings.begin(); sListIter != _strings.end(); sListIter++) {
		sList = (*sListIter).second;
		for (stringIter = sList->begin(); stringIter != sList->end(); stringIter++) {
			delete *stringIter;
		}
		delete sList;
	}

	// Delete all of the allocated fonts last, since they do not use other resources
	for (fontIter = _fonts.begin(); fontIter != _fonts.end(); fontIter++) {
		delete (*fontIter).second;
	}
}

/**
 * Initialize the 2D system by loading shaders, saving location information, etc.
 * @param screenWidth The width of the screen we're drawing to, in pixels.
 * @param screenHeight The height of the screen we're drawing to, in pixels.
 * @param err An output stream that can be used to write out errors that we detect during runtime.
 * @return True if we initialized successfully, false otherwise
 */
bool gui2d::Manager::init(int screenWidth, int screenHeight, std::ostream& err) {
	// Successfully initializing the FreeType library is required
	if (FT_Init_FreeType(&_ft)) {
		err << "(gui2d::Manager::init()) Unable to initialize FreeType library!" << std::endl;
		return false;
	}

	// Load and check our text shader
	if (!(_textShader = Shader::load(gui2d::SHADER_TEXT_SLOT, "text.vert", "text.frag"))) {
		err << "(gui2d::Manager::init()) Failed to load text shader program!" << std::endl;
		return false;
	}
	if (!_textShader->link()) {
		err << "(gui2d::Manager::init()) Failed to link text shader program!" << std::endl;
		return false;
	}

	// As well as our generic 2D shader
	if (!(_guiShader = Shader::load(gui2d::SHADER_2DGUI_SLOT, "2dgui.vert", "2dgui.frag"))) {
		err << "(gui2d::Manager::init()) Failed to load 2dgui shader program!" << std::endl;
		return false;
	}
	if (!_guiShader->link()) {
		err << "(gui2d::Manager::init()) Failed to link 2dgui shader program!" << std::endl;
		return false;
	}

	// And the untextured quad shader
	if (!(_untexShader = Shader::load(gui2d::SHADER_UNTEX_QUAD_SLOT, "2dgui_quads.vert", "2dgui_quads.frag"))) {
		err << "(gui2d::Manager::init()) Failed to load 2dgui untextured quad shader program!" << std::endl;
		return false;
	}
	if (!_untexShader->link()) {
		err << "(gui2d::Manager::init()) Failed to link 2dgui untextured quad shader program!" << std::endl;
		return false;
	}

	// Save attribute locations for the text shader
	_ts_in_vert = _textShader->getAttribLocation("in_vert");
	_ts_in_tex = _textShader->getAttribLocation("in_tex");
	_ts_tex = _textShader->getUniformLocation("tex");
	_ts_color = _textShader->getUniformLocation("color");
	_ts_un_z = _textShader->getUniformLocation("un_z");

	// Create the quad renderers
	_qr = new gui2d::QuadRenderer(_untexShader);
	_tqr = new gui2d::TexturedQuadRenderer(_guiShader);

	// Save our screen information
	_screenWidth = screenWidth;
	_screenHeight = screenHeight;

	// Initialize the pen values
	_curZ = 128.0f;
	_curColor = glm::vec4(1.0f);
	_curFont = -1;

	_err = &err;
	_init = true;
	_nextFontId = 1;
	
	return true;
}

/**
 * Prepares the GL Rendering System for Rendering
 */
void gui2d::Manager::prepare(void) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * Load in a font and store it for later use.
 * @param path An asset loader-compatible path for the font to load
 * @param size Size of the font, in pixels, to load
 * @return The identifier for this font
 */
int gui2d::Manager::loadFont(const std::string& path, int size) {
	gui2d::Font* font;
	gui2d::FontName fname = gui2d::FontName(path, size);

	// Library must be initialized first
	if (!_init) {
		(*_err) << "(gui2d::Manager::loadFont) Attempted to load a Font before initializing!" << std::endl;
		return -1;
	}

	// Check if we've already loaded this font
	if (_fontIds.count(fname) == 1) {
		_curFont = _fontIds[fname];
		return _curFont;
	}

	// Attempt to load the font
	font = gui2d::Font::loadFont(_nextFontId, this, path, size, *_err);
	if (font == NULL) {
		return -1;
	}

	// Save and return
	_fontIds[fname] = _nextFontId;
	_fonts[_nextFontId] = font;
	_strings[_nextFontId] = new gui2d::StringList();
	_curFont = _nextFontId;
	_nextFontId += 1;
	return font->getId();
}

/**
 * Retrieve the ID used for a specific font+size combination
 * @param path Path name of the font, same as in loadFont()
 * @param size Pixel size of the font, same as in loadFont()
 * @return The font ID
 */
int gui2d::Manager::getFontId(const std::string& path, int size) {
	gui2d::FontName fname = gui2d::FontName(path, size);

	if (_fontIds.count(fname) == 1) {
		return _fontIds[fname];
	}
	return -1;
}

/**
 * Retrieve the font stored for a specific ID
 * @param fontId The ID to retrieve
 * @return Pointer to the identified Font
 */
gui2d::Font* gui2d::Manager::getFont(int fontId) {
	if (_fonts.count(fontId) == 1) {
		return _fonts[fontId];
	}
	return NULL;
}

/**
 * Set the current font by ID
 * @param fontId The ID to use
 */
void gui2d::Manager::setFont(int fontId) {
	if (_fonts.count(fontId) == 1) {
		_curFont = fontId;
	}
}

/**
 * Loading a texture just means adding a reference to it by name
 * @param name Name of the texture to load, in a way that is meaningful for the asset loader
 * @return OpenGL identifier that refers to the newly loaded texture
 */
GLuint gui2d::Manager::loadTexture(const std::string& name) {
	return _textures.addRef(name);
}

/**
 * Increment the reference counter for a given texture ID
 * @param textureId The texture ID to increment reference counts for
 */
void gui2d::Manager::textureAddRef(GLuint textureId) {
	_textures.addRef(textureId);
}

/**
 * Decrement the reference counter for a texture
 * @param textureId The texture ID to decrement the reference counter for
 */
void gui2d::Manager::textureRemoveRef(GLuint textureId) {
	_textures.subRef(textureId);
}

/**
 * Texture initialization hook called by the texture reference counter
 * @param[in] name The "name" of the texture--this should be a path that can be passed to the asset loader
 * @param[out] tId The id assigned by OpenGL is saved here
 * @todo Use AssetLoader to source image files for DevIL
 */
void gui2d::Manager::createTexture(const std::string& name, GLuint& tId) {
	ILuint iId;
	ILint w, h;

	// Create a new texture
	glGenTextures(1, &tId);
	glBindTexture(GL_TEXTURE_2D, tId);
		
	// Configure filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Load the image with DevIL
	ilGenImages(1, &iId);
	ilBindImage(iId);
	ilLoadImage(name.c_str());
	w = ilGetInteger(IL_IMAGE_WIDTH);
	h = ilGetInteger(IL_IMAGE_HEIGHT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
	ilDeleteImages(1, &iId);
}

/**
 * Cleanup hook called by the texture reference counter
 * @param tId The OpenGL-given texture Id
 */
void gui2d::Manager::cleanupTexture(GLuint& tId) {
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tId);
}

/**
 * Accepts mouse click events and forwards them to active handlers
 * @param e The mouse event struct
 * @param id The pressed button ID
 * @return True if processing should continue, false if this click was handled
 */
bool gui2d::Manager::mousePressed(const OIS::MouseEvent& e, OIS::MouseButtonID id) {
	float x, y;
	std::vector<iMouseHandler *> handlers;
	std::vector<iMouseHandler *>::iterator iter;
	int found;
	input::Cursor& c = _ge->getCursor();
	Ogre::Vector2 mp = c.getPosition();

	// Convert the 0,1 normalized coordinates to -1,1
	x = 2.0f * (mp.x - 0.5f);
	y = 2.0f * (0.5f - mp.y);

	// Get mouse handlers
	found = _mouseHandlers->locate(x, y, handlers);

	if(!found) {
		return false;
	}

	iter = handlers.begin();
	while (iter != handlers.end()) {
		if (!(*iter)->mousePressed(x, y, id)) {
			return false;
		}
		
		++iter;
	}

	return true;
}

/**
 * Accepts mouse click events and forwards them to active handlers
 * @param e The mouse event struct
 * @param id The pressed button ID
 * @return True if processing should continue, false if this click was handled
 */
bool gui2d::Manager::mouseReleased(const OIS::MouseEvent& e, OIS::MouseButtonID id) {
	float x, y;
	std::vector<iMouseHandler *> handlers;
	std::vector<iMouseHandler *>::iterator iter;
	int found;
	input::Cursor& c = _ge->getCursor();
	Ogre::Vector2 mp = c.getPosition();

	// Convert the 0,1 normalized coordinates to -1,1
	x = 2.0f * (mp.x - 0.5f);
	y = 2.0f * (0.5f - mp.y);

	// Get mouse handlers
	found = _mouseHandlers->locate(x, y, handlers);

	if(!found) {
		return false;
	}

	iter = handlers.begin();
	while (iter != handlers.end()) {
		if (!(*iter)->mouseReleased(x, y, id)) {
			return false;
		}
		
		++iter;
	}

	return true;
}

/**
 * Accepts mouse motion events and forwards them to the active handlers
 * @param e The mouse event struct
 * @return True if processing should continue, false otherwise
 */
bool gui2d::Manager::mouseMoved(const OIS::MouseEvent& e) {
	return true;
}

/**
 * Add an appropriate class as a mouse event handler, so it will receive events next time we get them
 * @param handler Pointer to the handler to add
 */
void gui2d::Manager::addMouseHandler(iMouseHandler *handler) {
	_mouseHandlers->insert(handler);
}

/**
 * Remove an already added mouse handler. If it wasn't already added, this does nothing
 * @param handler Pointer to the handler to remove
 */
void gui2d::Manager::removeMouseHandler(iMouseHandler *handler) {
	_mouseHandlers->remove(handler);
}

/**
 * Add an appropriate class as a mouse motion event handler, so that it will receive events in the future
 * @param handler Pointer to the handler to add
 */
void gui2d::Manager::addMouseMotionHandler(iMouseMotionHandler *handler) {
	_mouseMotionHandlers->insert(handler);
}

/**
 * Remove a mouse motion listener. If it wasn't added, this does nothing
 * @param handler Pointer to the handler to remove
 */
void gui2d::Manager::removeMouseMotionHandler(iMouseMotionHandler *handler) {
	_mouseMotionHandlers->remove(handler);
}

/**
 * Construct a String object and store a reference to it so that it will be rendered
 */
gui2d::String *gui2d::Manager::createString(const std::string& source) { return createString(_curFont, source); }
gui2d::String *gui2d::Manager::createString(int fontId, const std::string& source) { return createString(fontId, source, 0.0f, 0.0f); }
gui2d::String *gui2d::Manager::createString(const std::string& source, float normX, float normY) { return createString(_curFont, source, normX, normY); }
gui2d::String *gui2d::Manager::createString(int fontId, const std::string& source, float normX, float normY) {
	gui2d::String *str;
	gui2d::Font *font;
	gui2d::StringList *sList;

	if (_fonts.count(fontId) == 1) {
		font = _fonts[fontId];
		sList = _strings[fontId];

		str = new gui2d::String(font);
		sList->push_back(str);
		str->drawText(source, normX, normY);
		str->setZ(_curZ);
		str->setColor(_curColor);
		return str;
	}

	return NULL;
}

/**
 * Construct an InputBox object and store a pointer so that it will be rendered
 */
gui2d::InputBox *gui2d::Manager::createInputBox(float normX, float normY) { return createInputBox(_curFont, normX, normY); }
gui2d::InputBox *gui2d::Manager::createInputBox(int fontId, float normX, float normY) {
	gui2d::Font *font;
	gui2d::InputBox *input;

	if (_fonts.count(fontId) == 1) {
		font = _fonts[fontId];

		input = new gui2d::InputBox(this, font);
		input->setActiveColor(_curColor);
		input->setPosition(normX, normY);
		_inputs.push_back(input);
		return input;
	}
	return NULL;
}

/**
 * Remove a reference to a string from the render list, unless we're in the destructor
 * @param s Pointer to the string to remove
 */
void gui2d::Manager::removeString(gui2d::String *s) {
	if (_destructor != STRINGS && _strings.count(s->getFont()->getId()) == 1) {
		_strings[s->getFont()->getId()]->remove(s);
	}
}

/**
 * Show the quads associated with an object by passing it to the underlying QuadRenderer
 * @param r The QuadRenderable to show
 */
void gui2d::Manager::showQuads(iUntexturedQuadRenderable *r) {
	_qr->show(r);
}

/**
 * Hide the quads associated with an object by passing it to the underlying QuadRenderer
 * @param r The QuadRenderable to hide
 */
void gui2d::Manager::hideQuads(iUntexturedQuadRenderable *r) {
	_qr->hide(r);
}

/**
 * Render front-end interface
 */
void gui2d::Manager::render(void) {
	prepare();

	_qr->render();
	_tqr->render();
	renderText();
}

/**
 * Render all of the strings, sorted by font type in order to minimize texture binds
 */
void gui2d::Manager::renderText(void) {
	StringListIter iter;
	FontMapIter fontIter;
	StringList* strings;

	// Activate our text shader
	_textShader->use();
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(_ts_tex, 0);

	// For each font we have loaded, render all strings at once to minimize texture rebinding
	for (fontIter = _fonts.begin(); fontIter != _fonts.end(); ++fontIter) {
		glBindTexture(GL_TEXTURE_2D, (*fontIter).second->getTextureId());

		strings = _strings[(*fontIter).first];
		for (iter = strings->begin(); iter != strings->end(); iter++) {
			(*iter)->render();
		}
	}

	// Deactive the text shader
	glUseProgram(0);
}