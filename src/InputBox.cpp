/**
 * @file 2dgui/InputBox.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <algorithm>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iUntexturedQuadRenderable.h"
#include "2dgui/Manager.h"
#include "2dgui/String.h"
#include "2dgui/InputBox.h"
#include "input/Manager.h"

/**
 * This constructor initializes all of the resources used to draw this input box.
 * @param m The Manager that controls this input box
 * @param f The Font to use to draw the text inside this input box.
 */
gui2d::InputBox::InputBox(gui2d::Manager* m, gui2d::Font* f) : iUntexturedQuadRenderable(2) {
	init();
	_w = 0.3f;
	_m = m;
	_string = m->createString(f->getId(), "");
	_string->setZ(_z - 5.0f);
	setInnerHeight(_string->getHeightf());
	_m->showQuads(this);
}

/**
 * Initialization routine just sets sane values for everything. Most needs
 * to be customized by the user eventually.
 */
void gui2d::InputBox::init(void) {
	_m = NULL;
	_string = NULL;
	_x = _y = _h = _w = 0.0f;
	_margin[0] = _margin[1] = _margin[2] = _margin[3] = 0.0f;
	_z = 100;
	_cursor = 0;
	_inSelection = false;
	_selectStart = 0;
	_activeColor = glm::vec4(1.0);
	_inactiveColor = glm::vec4(0.5, 0.5, 0.5, 1.0);
	_visible = true;
	_active = true;
}

/**
 * Destructor simply deallocates our string
 */
gui2d::InputBox::~InputBox(void) {
	delete _string;
}

/**
 * Set the text for the underlying String, and update our cursor to the end of the string.
 * @param text The string to assign to the input
 */
void gui2d::InputBox::setText(const std::string& text) {
	_string->drawText(text);
	_cursor = text.length();
	_inSelection = false;
}

/**
 * Clears the underlying string, done for convenience
 */
void gui2d::InputBox::clearText(void) {
	_string->remove(0);
	_cursor = 0;
	_inSelection = false;
}

/**
 * Set the margin for the input box. Note that this margin applies to all four
 * edges, when set with this method.
 * @param margin The value of margin, in normalized units
 */
void gui2d::InputBox::setMargin(float margin) {
	_margin[0] = _margin[1] = _margin[2] = _margin[3] = margin;
	_string->setPosition(_x+margin, _y+margin);
}

/**
 * Set the inner height of this input box, that is the height excluding margins
 * @param h The height to use for the inner area of the input box
 */
void gui2d::InputBox::setInnerHeight(float h) {
	_h = h + _margin[MARGIN_BOTTOM] + _margin[MARGIN_TOP];
	_string->setPosition(_x + _margin[MARGIN_LEFT], _y + _margin[MARGIN_BOTTOM]);
	setQuadXY(0, _x, _y, _w, _h);
}


/**
 * Changes the background color for the input box
 * @param color The color to use
 */
void gui2d::InputBox::setBackgroundColor(const glm::vec4& color) {
	setQuadColor(0, color);
}

/**
 * Set the color used by the underlying string in active mode
 * @param color The color of active input text
 */
void gui2d::InputBox::setActiveColor(const glm::vec4& color) {
	_activeColor = color;
	if (_active) {
		_string->setColor(color);
	}
}

/**
 * Set the color, but this time for inactive mode
 * @param color The color of inactive input text
 */
void gui2d::InputBox::setInactiveColor(const glm::vec4& color) {
	_inactiveColor = color;
	if (!_active) {
		_string->setColor(color);
	}
}

/**
 * Update our position as well as that of the underlying string.
 * @param normX The normalized x screen coordinate (-1, 1)
 * @param normY The normalized y screen coordiante (-1, 1)
 */
void gui2d::InputBox::setPosition(float normX, float normY) {
	_string->setPosition(normX + _margin[MARGIN_LEFT], normY + _margin[MARGIN_BOTTOM]);
	_x = normX;
	_y = normY;
	setQuadXY(0, _x, _y, _w, _h);
}

/**
 * Update the Z-coordinates for the input box, etc. to make it overlap with text properly
 * @param z New Z coordinate, in a non-normalized float (i.e. not 0-1).
 */
void gui2d::InputBox::setZ(float z) {
	_string->setZ(z - 5.0f);
	setQuadsZ(static_cast<GLshort>(z));
	_z = z;
}

/**
 * Set this input box to be active, changing the font color
 */
void gui2d::InputBox::activate(void) {
	_active = true;
	_string->setColor(_activeColor);
}

/**
 * Set this input box to be inactive, changing the font color
 */
void gui2d::InputBox::deactivate(void) {
	_active = false;
	_string->setColor(_inactiveColor);
}

/**
 * Show this input box; this is independent from being active or not
 */
void gui2d::InputBox::show(void) {
	_visible = true;
	_string->show();
	_m->showQuads(this);
}

/**
 * Hide this input box, this is independent from being active or not
 */
void gui2d::InputBox::hide(void) {
	_visible = false;
	_string->hide();
	_m->hideQuads(this);
}

/**
 * Handle an input event from input::KeyEvent by either adding characters or modifying our cursor position
 * @param e The input::KeyEvent keyboard event that has information about what happened
 */
void gui2d::InputBox::keyPressed(const input::KeyEvent &e) {
	std::string str;

	// If we're not active, why are we receiving events?
	if (!_active) {
		return;
	}

	// Backspace key deletes characters, as does the delete key
	if (e.ois.key == OIS::KC_BACK || e.ois.key == OIS::KC_DELETE) {
		if (_inSelection) {
			_inSelection = false;
			if (_cursor < _selectStart) {
				_string->remove(_cursor, _selectStart - _cursor);
			}
			else {
				_string->remove(_selectStart, _cursor - _selectStart);
				_cursor = _selectStart;
			}
		}
		else {
			if (e.ois.key == OIS::KC_BACK && _cursor > 0) {
				_string->remove(_cursor-1, 1);
				_cursor -= 1;
			}
			else if (e.ois.key == OIS::KC_DELETE) {
				_string->remove(_cursor, 1);
			}
		}
	}
	else if (e.ois.key == OIS::KC_LEFT) {
		if (input::Manager::getSingleton().Keyboard().isKeyDown(OIS::KC_LSHIFT)) {
			if (!_inSelection) {
				_selectStart = _cursor;
				_inSelection = true;
			}
			else {
				if (_selectStart == _cursor-1) {
					_inSelection = false;
				}
			}
		}
		else {
			_inSelection = false;
		}

		if (_cursor > 0) {
			_cursor -= 1;
		}
	}
	else if (e.ois.key == OIS::KC_RIGHT) {
		if (input::Manager::getSingleton().Keyboard().isKeyDown(OIS::KC_LSHIFT)) {
			if (!_inSelection) {
				_selectStart = _cursor;
				_inSelection = true;
			}
			else {
				if (_selectStart == _cursor+1) {
					_inSelection = false;
				}
			}
		}
		else {
			_inSelection = false;
		}

		if (_cursor < _string->length()) {
			_cursor += 1;
		}
	}
	else if ((e.ois.key >= OIS::KC_1 && e.ois.key <= OIS::KC_EQUALS) ||
			 (e.ois.key >= OIS::KC_Q && e.ois.key <= OIS::KC_RBRACKET) ||
			 (e.ois.key >= OIS::KC_A && e.ois.key <= OIS::KC_GRAVE) ||
			 (e.ois.key >= OIS::KC_BACKSLASH && e.ois.key <= OIS::KC_SLASH)) {

		// Prepare the insertion string
		if (e.ois.key == OIS::KC_SPACE) {
			str = " ";
		}
		else {
			str = std::string(input::Manager::getSingleton().Keyboard().getAsString(e.ois.key));
			if (!input::Manager::getSingleton().Keyboard().isKeyDown(OIS::KC_LSHIFT)) {
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			}
		}

		// If there's a selection, delete it because we overwrite in that case
		if (_inSelection) {
			_inSelection = false;
			if (_cursor < _selectStart) {
				_string->remove(_cursor, _selectStart - _cursor);
			}
			else {
				_string->remove(_selectStart, _cursor - _selectStart);
				_cursor = _selectStart;
			}
		}
	
		// Insert the string wherever we end up
		_string->insert(str, _cursor);
		_cursor += 1;
	}
}
