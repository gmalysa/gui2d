/**
 * @file 2dgui/screen.cpp
 * @todo License/copyright statement
 */

// Standard headers
// None

// Project definitions
#include "2dgui/Screen.h"

/**
 * The constructor is empty, this screen comes up visible by default
 */
gui2d::Screen::Screen(void) {}

/**
 * The screen takes ownership of any items that are added to it, so all pointers
 * are deallocated during the destructor
 */
gui2d::Screen::~Screen(void) {
	VisibleListIter iter;

	for (iter = _items.begin(); iter != _items.end(); ++iter) {
		delete *iter;
	}
}

/**
 * This overloads the show method to show all sub-items as well
 */
void gui2d::Screen::show(void) {
	VisibleListIter iter;

	_show();

	for (iter = _items.begin(); iter != _items.end(); ++iter) {
		(*iter)->show();
	}
}

/**
 * This overloads the hide method to hide all sub-items as well
 */
void gui2d::Screen::hide(void) {
	VisibleListIter iter;

	_hide();

	for (iter = _items.begin(); iter != _items.end(); ++iter) {
		(*iter)->hide();
	}
}

/**
 * Adds an item to the tracking/management list for this screen
 * @param item The item to add
 */
void gui2d::Screen::add(iVisible *item) {
	_items.push_back(item);
}

/**
 * Removes an item from tracking/management by this screen. If you do this,
 * the screen WILL NOT call the destructors, unless it is re-added before cleanup
 * time.
 * @param item The item to remove
 */
void gui2d::Screen::remove(iVisible *item) {
	_items.remove(item);
}