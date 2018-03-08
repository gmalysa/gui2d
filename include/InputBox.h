#ifndef _2DGUI_INPUTBOX_H_
#define _2DGUI_INPUTBOX_H_
/**
 * @class gui2d::InputBox
 * A GUI component that presents the user with the ability to enter text
 * @todo Implement getSelectionText()
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iUntexturedQuadRenderable.h"
#include "2dgui/String.h"
#include "input/KeyEvent.h"

namespace gui2d {

class InputBox : public iUntexturedQuadRenderable {
private:
	static const int MARGIN_LEFT = 0;
	static const int MARGIN_RIGHT = 1;
	static const int MARGIN_TOP = 2;
	static const int MARGIN_BOTTOM = 3;

	// Relevant pointers to objects this manipulates
	Manager* _m;
	String* _string;

	// Display state variables
	bool _active;
	bool _visible;

	// Editing variables
	int _cursor;
	bool _inSelection;
	int _selectStart;

	// Graphics variables
	float _x, _y, _w, _h;
	float _z;
	float _margin[4];		// left, right, top, bottom

	// Color options for our states
	glm::vec4 _activeColor;
	glm::vec4 _inactiveColor;

	void init(void);

public:
	InputBox(Manager* m, Font* f);
	~InputBox(void);

	// Accessors that are basically forwarded
	std::string getSelectionText(void);
	void setText(const std::string& text);
	void clearText(void);

	// Padding between the edges of the box and the string being edited
	void setMargin(float margin);
	void setMarginLeft(float margin);
	void setMarginRight(float margin);
	void setMarginTop(float margin);
	void setMarginBottom(float margin);

	// Other graphical attributes
	void setBackgroundColor(const glm::vec4& color);
	void setActiveColor(const glm::vec4& color);
	void setInactiveColor(const glm::vec4& color);
	void setZ(float z);
	void setPosition(float normX, float normY);
	void setInnerHeight(float h);
	void setHeight(float h);

	// Control display state
	void activate(void);
	void deactivate(void);
	void show(void);
	void hide(void);

	// Callback hook
	void keyPressed(const input::KeyEvent &e);

	/**
	 * Retrieve the current text value of this input box
	 */
	const std::string& getText(void) const { return _string->getText(); }

	/**
	 * Set which Manager is used to control this InputBox
	 * @param m The manager we should refer to
	 */
	void setManager(Manager* m) { _m = m; }

	/**
	 * Retrieve this InputBox's lower-left x coordinate
	 */
	float getX(void) const { return _x; }
	
	/**
	 * Retrieve this InputBox's lower-left y coordinate
	 */
	float getY(void) const { return _y; }
};

};

#endif
