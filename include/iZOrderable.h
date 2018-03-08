#ifndef _I_ZORDERABLE_H_
#define _I_ZORDERABLE_H_
/**
 * @class gui2d::iZOrderable
 * Interface for objects which support changing their z-indices, along with storage
 */

// Standard headers
#include <gl/glew.h>

// Project definitions
// None

namespace gui2d {

class iZOrderable {
protected:
	GLushort _z;

	/**
	 * Behind the scenes concrete implementation, this method adjusts stored z values
	 * properly
	 * @param z The new z value, as an unnormalized unsigned short
	 */
	void _setZ(GLushort z) {
		_z = z;
	}

public:
	/**
	 * Default constructor does nothing
	 */
	iZOrderable(void) : _z(0) {}
	
	/**
	 * Optional constructor with starting/default z-index
	 * @param z Starting Z index to use
	 */
	iZOrderable(float z) : _z(0) {
		_z = static_cast<GLushort>(z);
	}

	/**
	 * Optional constructor with starting/default z-index
	 * @param z Starting Z index to use
	 */
	iZOrderable(GLushort z) : _z(z) {}

	/**
	 * Virtual destructor because this is a virtual class, but this one doesn't do anything
	 */
	virtual ~iZOrderable(void) {}

	/**
	 * The actual interface: adjust Z value
	 * @param z The new z to use, this is an unnormalized float
	 */
	virtual void setZ(float z) {
		setZ(static_cast<GLushort>(z));
	}

	/**
	 * The actual interface, adjust Z value
	 * @param z The new z to use, this is an unnormalized short
	 */
	virtual void setZ(GLushort z) {
		_setZ(z);
	}

};

};

#endif