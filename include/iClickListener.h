#ifndef _I_CLICK_LISTENER_H_
#define _I_CLICK_LISTENER_H_
/**
 * @class gui2d::iClickListener
 * The interface for all things that can be added as complete click listeners
 * to iMouseHandler-implementing classes
 */

// Standard headers
#include <OIS/OIS.h>

// Project definitions
// None

namespace gui2d {

class iClickListener {
public:
	virtual void operator()(OIS::MouseButtonID id) = 0;
};

};

#endif