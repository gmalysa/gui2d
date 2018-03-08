#ifndef _SCREEN_H_
#define _SCREEN_H_
/**
 * @class gui2d::Screen
 * A screen is a logical grouping of gui2d elements that enables them to be controlled
 * and have certain properties modified en masse
 */

// Standard headers
// None

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iVisible.h"

namespace gui2d {

class Screen : public iVisible {
private:
	VisibleList _items;

public:
	Screen(void);
	~Screen(void);

	void show();
	void hide();

	void add(iVisible *item);
	void remove(iVisible *item);
};

};

#endif