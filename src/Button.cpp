/**
 * @file 2dgui/Button.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <iostream>

// Project definitions
#include "2dgui/Button.h"
#include "2dgui/Manager.h"
#include "2dgui/TexturedQuadRenderer.h"
#include "2dgui/Font.h"
#include "2dgui/String.h"
#include "2dgui/iClickListener.h"

/**
 * Constructor for a new button
 */
gui2d::Button::Button() : iMouseHandler(glm::vec4(0.0f)), iTexturedQuadRenderable(1),
		_m(0), _str(0), _enabled(true), _clicked(false), _textAlign(gui2d::TEXT_ALIGN_LEFT) {
	// Get singleton pointers
	_m = &gui2d::Manager::getSingleton();
	_tqr = &gui2d::TexturedQuadRenderer::getSingleton();
			
	// Create a string to store our label
	_str = _m->createString("", 0.0f, 0.0f);

	// Initialize texture coordinates
	setQuadUV(0, 0.0f, 1.0f, 0.0f, 1.0f);

	// Initialize vertex coordinates
	recalculateLocations();
	setOpacity(1.0f);
	setZ(100.0f);
	show();
}

/**
 * When this button goes away, it should clean up its text resources
 */
gui2d::Button::~Button(void) {
	// If we're visible, we gotta tell the quad renderer to stop trying to draw us
	if (_visible)
		_tqr->hide(this);

	// Let the manager know we're done with this texture
	if (getTextureId(0))
		_m->textureRemoveRef(getTextureId(0));

	// Clean up our string as well
	delete _str;
}

/**
 * Configure this button to use a specific texture
 * @param name The string name that identifies the texture used
 */
void gui2d::Button::setTexture(const std::string& name) {
	if (getTextureId(0))
		_m->textureRemoveRef(getTextureId(0));

	setTextureId(0, _m->loadTexture(name));
}

/**
 * Show this button
 */
void gui2d::Button::show(void) {
	_tqr->show(this);
	_m->addMouseHandler(this);
	_modified = true;
	_str->show();
	_show();
}

/**
 * Hide this button
 */
void gui2d::Button::hide(void) {
	_tqr->hide(this);
	_m->removeMouseHandler(this);
	_modified = true;
	_str->hide();
	_hide();
}

/**
 * Change the text that is in our main label
 * @param text The text to draw
 */
void gui2d::Button::setText(const std::string& text) {
	_str->drawText(text);
	recalculateLocations();
}


/**
 * Update positioning for the button and the string, if the string is used
 * @param normX Normalized x coordinate for boundary
 * @param normY Normalized y coordinate for boundary
 * @param width Width in normalized coordinates for the button
 * @param height Height in normalized coordinates for the button
 */
void gui2d::Button::setBounds(float normX, float normY, float width, float height) {
	// Remove from the mouse handlers if necessary
	if (_visible) {
		_m->removeMouseHandler(this);
	}

	// Adjust tracked bounds and then update button position stuff
	_setBounds(normX, normY, width, height);
	recalculateLocations();

	//Make it visible again
	if (_visible) {
		_m->addMouseHandler(this);
	}
}

/**
 * Update the opacity by changing its storage in our texture coordinates
 * @param alpha The new opacity alpha value
 */
void gui2d::Button::setOpacity(float alpha) {
	setQuadAlpha(0, alpha);
	_setAlpha(static_cast<GLubyte>(glm::clamp(alpha, 0.0f, 1.0f)*255));
}

/**
 * Update our Z index by changing its value in our position vector. This Z index is an unnormalized
 * float, just like everywhere else
 * @param z The new z-index to use
 */
void gui2d::Button::setZ(float z) {
	setQuadZ(0, static_cast<GLshort>(z));
	_str->setZ(z - 1.0f);
}

/**
 * Change the text alignment policy used to draw this button and then update things
 * @param align The new text alignment, should be one of the constants declared in gui2d
 */
void gui2d::Button::setTextAlignment(int align) {
	_textAlign = align;
	recalculateLocations();
}

/**
 * Mouse clicked will forward to a mouse listener on this button and/or call a Lua
 * function.
 * @param x X coordinate of mouse press, normalized
 * @param y Y coordinate of mouse press, normalized
 * @param id The button ID telling us which button was pressed
 * @return bool True if we should continue processing, false otherwise
 */
bool gui2d::Button::mousePressed(float x, float y, OIS::MouseButtonID id) {
	// Only click if we're enabled
	if (!_enabled)
		return true;

	// Mark that the click actually started within our bounds
	_clicked = true;

	// We absorb the button click, do not continue event-ing
	return false;
}

/**
 * Mouse unclicked will forward to a mouse listener on this button and/or call a Lua function.
 * @param x X coordinate of mouse press, normalized
 * @param y Y coordinate of mouse press, normalized
 * @param id The button ID telling us which button was pressed
 * @return bool True if we should continue processing, false otherwise
 */
bool gui2d::Button::mouseReleased(float x, float y, OIS::MouseButtonID id) {
	gui2d::ClickListenerListIter iter;
	gui2d::iClickListener *listener;

	// If not enabled, don't handle the event
	if (!_enabled)
		return true;

	// Forward completed click events to all the listeners if this was a properly completed click
	if (_clicked && contains(x, y)) {
		for (iter = _clickListeners.begin(); iter != _clickListeners.end(); ++iter) {
			listener = *iter;
			listener->operator()(id);
		}
	}

	_clicked = false;

	// We always absorb the click when we handle it
	return false;
}

/**
 * Adds a click listener to our tracking list
 * @param listener The listener to add
 */
void gui2d::Button::addClickListener(iClickListener *listener) {
	_clickListeners.push_back(listener);
}

/**
 * Removes a click listener from our tracking list
 * @param listener The listener to remove
 */
void gui2d::Button::removeClickListener(iClickListener *listener) {
	_clickListeners.remove(listener);
}

/**
 * Recalculates the vertex coordinates used for rendering, and also repositions the label if it is used
 */
void gui2d::Button::recalculateLocations(void) {
	// Update the quad vertex positions
	setQuadXY(0, _bounds[MIN_X], _bounds[MIN_Y], _bounds[MAX_X]-_bounds[MIN_X], _bounds[MAX_Y]-_bounds[MIN_Y]);

	// Reposition the text, if we have any
	switch(_textAlign) {
		case gui2d::TEXT_ALIGN_CENTER:
			_str->setPosition((_bounds[MIN_X] + _bounds[MAX_X] - _str->getWidthf())/2, (_bounds[MIN_Y] + _bounds[MAX_Y] - _str->getHeightf())/2.0f);
			break;
		case gui2d::TEXT_ALIGN_RIGHT:
			_str->setPosition(_bounds[MAX_X] - 2.0f*_m->getPixelWidth() - _str->getWidthf(), (_bounds[MIN_Y] + _bounds[MAX_Y] - _str->getHeightf())/2.0f);
			break;
		case gui2d::TEXT_ALIGN_LEFT:
		default:
			_str->setPosition(_bounds[MIN_X] + 2.0f*_m->getPixelWidth(), (_bounds[MIN_Y] + _bounds[MAX_Y] - _str->getHeightf())/2.0f);
			break;
	}
}