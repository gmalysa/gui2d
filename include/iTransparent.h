#ifndef _I_TRANSPARENT_H_
#define _I_TRANSPARENT_H_
/**
 * @class gui2d::iTransparent
 * Interface for all gui elements that support transparency, along with the usual
 * standardization of methods and formats
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>

// Project definitions
// None

namespace gui2d {

class iTransparent {
protected:
	GLubyte _alpha;

	/**
	 * Actual alpha-changing function
	 * @param alpha The new alpha value, as a normalized byte
	 */
	void _setAlpha(GLubyte alpha) {
		_alpha = alpha;
	}

public:
	/**
	 * Default opacity is 1, fully visible
	 */
	iTransparent(void) : _alpha(255) {}

	/**
	 * Optionally, a different opacity
	 * @param alpha Initial alpha value to use
	 */
	iTransparent(float alpha) : _alpha(0) {
		_setAlpha(static_cast<GLubyte>(glm::clamp(alpha, 0.0f, 1.0f)*255));
	}

	/**
	 * Optionally, a different opacity
	 * @param alpha Initial alpha value to use
	 */
	iTransparent(GLubyte alpha) : _alpha(alpha) {}

	/**
	 * Virtual destructor, even though it's empty
	 */
	virtual ~iTransparent(void) {}

	/**
	 * Client accessor for changing this element's opacity
	 * @param alpha The new opacity to use
	 */
	virtual void setOpacity(float alpha) {
		setOpacity(static_cast<GLubyte>(glm::clamp(alpha, 0.0f, 1.0f)*255));
	}

	/**
	 * Client accessor for changing this element's opacity
	 * @param alpha The new opacity to use
	 */
	virtual void setOpacity(GLubyte alpha) {
		_setAlpha(alpha);
	}

};

};

#endif