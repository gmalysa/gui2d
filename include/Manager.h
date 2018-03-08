#ifndef _2DGUI_MANAGER_H_
#define _2DGUI_MANAGER_H_
/**
 * @class gui2d::Manager
 * This defines the 2D GUI manager, which is the overall GUI interface for the program. It creates and manages
 * graphical user interface elements, such as display strings, input boxes, etc.
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <OIS/OIS.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <string>
#include <list>
#include <map>

// Project definitions
#include "sks.h"
#include "Singleton.h"
#include "TRResource.h"
#include "2dgui/gui2d.h"
#include "input/Cursor.h"

//! @todo Move these to a util package
#define NORMALIZE(type, x, bits, divisor) static_cast<type>((x) * ((1 << (bits))) / (divisor))
#define NORM2FLOAT(x, bits) (static_cast<float>(x) /(1 << (bits)))

namespace gui2d {

class Manager : public Singleton<Manager> {
public:
	// Static callbacks used by the texture refcounter
	static void createTexture(const std::string& name, GLuint& tId);
	static void cleanupTexture(GLuint& tId);

	/**
	 * Our specific reference counter for named textures
	 */
	typedef TRResource<std::string, GLuint, Manager::createTexture, Manager::cleanupTexture> TextureCounter;

	/**
	 * Destructor states
	 */
	enum DestructorStates {NONE, BUTTONS, INPUTS, STRINGS};

private:
	// General data
	bool _init;
	DestructorStates _destructor;
	FT_Library _ft;
	int _nextFontId;
	std::ostream *_err;
	
	// Track stuff we're responsible for
	TextureCounter _textures;
	FontIdMap _fontIds;
	FontMap _fonts;
	FontStringList _strings;
	InputList _inputs;

	// Shader information and attribute locations for text
	Shader *_textShader;
	GLint _ts_in_vert;
	GLint _ts_in_tex;
	GLint _ts_tex;
	GLint _ts_color;
	GLint _ts_un_z;

	// Quad rendering systems
	Shader *_guiShader;
	Shader *_untexShader;
	QuadRenderer *_qr;
	TexturedQuadRenderer *_tqr;

	// Mouse event listeners
	QuadTree<iMouseHandler> *_mouseHandlers;
	QuadTree<iMouseMotionHandler> *_mouseMotionHandlers;

	// Screen configuration
	GraphicsEngine *_ge;
	int _screenWidth, _screenHeight;

	// Current "pen" information
	int _curFont;
	glm::vec4 _curColor;
	float _curZ;

	// Render helpers for specific types of 2D elements
	void renderText(void);

public:
	Manager(GraphicsEngine *ge);
	~Manager(void);

	// Initializes the 2D GUI system
	bool init(int screenWidth, int screenHeight, std::ostream& err);

	// Prepares the 2D GUI system for rendering
	void prepare();

	// Shader variable accessors
	GLint getTSInVertLocation() const { return _ts_in_vert; }
	GLint getTSInTexLocation() const { return _ts_in_tex; }
	GLint getTSUniformTexLocation() const { return _ts_tex; }
	GLint getTSUniformColorLocation() const { return _ts_color; }
	GLint getTSUniformZLocation() const { return _ts_un_z; }

	// Setup accessors
	FT_Library *getFreeTypeLibrary(void) { return &_ft; }
	int getScreenWidth(void) const { return _screenWidth; }
	int getScreenHeight(void) const { return _screenHeight; }

	// Font storage and retrieval interface
	int loadFont(const std::string& path, int size);
	int getFontId(const std::string& path, int size);
	int getCurrentFontId(void) const { return _curFont; }
	Font *getCurrentFont(void) { return _fonts[_curFont]; }
	Font *getFont(int fontId);
	void setFont(int fontId);
	void setColor(glm::vec4 color) { _curColor = color; }

	// String management interface
	String *createString(const std::string& source);
	String *createString(int fontId, const std::string& source);
	String *createString(const std::string& source, float normX, float normY);
	String *createString(int fontId, const std::string& source, float normX, float normY);
	void removeString(String *s);

	// Methods to inject events
	bool mousePressed(const OIS::MouseEvent& e, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent& e, OIS::MouseButtonID id);
	bool mouseMoved(const OIS::MouseEvent& e);

	// Event handler management
	void addMouseHandler(iMouseHandler *handler);
	void removeMouseHandler(iMouseHandler *handler);
	void addMouseMotionHandler(iMouseMotionHandler *handler);
	void removeMouseMotionHandler(iMouseMotionHandler *handler);

	// Input management interface
	InputBox *createInputBox(float normX, float normY);
	InputBox *createInputBox(int fontId, float normX, float normY);
	void removeInputBox(InputBox *box);

	// Renderable quads interface
	void showQuads(iUntexturedQuadRenderable *r);
	void hideQuads(iUntexturedQuadRenderable *r);

	// Texture management interface
	GLuint loadTexture(const std::string& name);
	void textureAddRef(GLuint textureId);
	void textureRemoveRef(GLuint textureId);

	// Single entry point for drawing all of the 2D subsystem
	void render(void);

	/**
	 * Helper method that returns a height of one pixel normalized to our window height
	 * Note that for a screen height of 800, valid pixel coordinates are -400,399, so 1/800
	 * is actually half a pixel, thus we must return twice that.
	 */
	float getPixelHeight(void) const { return 2.0f / _screenHeight; }

	/**
	 * Helper method that returns a width of one pixel normalized to our window width
	 * @see getPixelHeight() for information on why this is 2/width, instead of 1/width
	 */
	float getPixelWidth(void) const { return 2.0f / _screenWidth; }
};

};

#endif
