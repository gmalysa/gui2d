#ifndef _I_VISIBLE_H_
#define _I_VISIBLE_H_
/**
 * @class gui2d::iVisible
 * This interface is used to specify classes that can be told to alternate
 * between show/hide states, as a feature abstraction.
 */

// Standard headers
// None

// Project definitions
// None

namespace gui2d {
	
class iVisible {
protected:
	bool _visible;			//!< State for this "interface"

	/**
	 * Show implementation that adjusts our flag
	 */
	void _show(void) {
		_visible = true;
	}

	/**
	 * Hide implementation that adjusts our flag
	 */
	void _hide(void) {
		_visible = false;
	}

public:
	/**
	 * Constructor merely initializes visibility state to a default visible
	 */
	iVisible(void) : _visible(true) {}

	/**
	 * Alternatively, parameterized constructor lets us pick what we want
	 * @param visible Should this default to visible?
	 */
	iVisible(bool visible) : _visible(visible) {}
	
	/**
	 * Destructor is empty, but virtual to support proper forwarding
	 */
	virtual ~iVisible(void) {}

	/**
	 * Determine if this is currently visible or not
	 * @return true if visible, false otherwise
	 */
	bool isVisible(void) const {
		return _visible;
	}

	/**
	 *  Default show implementation merely calls the concrete function
	 */
	virtual void show(void) {
		_show();
	}

	/**
	 * Default hide implementation merely calls the concrete function
	 */
	virtual void hide(void) {
		_hide();
	}

};

};

#endif