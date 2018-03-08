#ifndef _2DGUI_BUTTON_H_
#define _2DGUI_BUTTON_H_
/**
 * @class gui2d::Button
 * An optionally textured button with optional text in the middle
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <OIS/OIS.h>

#include <string>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iMouseHandler.h"
#include "2dgui/iTexturedQuadRenderable.h"
#include "2dgui/iTransparent.h"
#include "2dgui/iZOrderable.h"
#include "2dgui/iVisible.h"

namespace gui2d {

class Button : public iMouseHandler, public iTexturedQuadRenderable,
				public iTransparent, public iZOrderable, public iVisible {
private:
	// Resources used
	Manager *_m;
	TexturedQuadRenderer *_tqr;
	String *_str;
	ClickListenerList _clickListeners;

	// Drawing modifiers
	bool _enabled;
	bool _clicked;

	// Text alignment value
	int _textAlign;
	
	// Recalculates stuff
	void recalculateLocations(void);

public:
	Button(void);
	~Button(void);

	// Overridden methods from interfaces
	void setZ(float z);
	void setOpacity(float alpha);

	// Graphical interface
	void setTextAlignment(int align);
	void setText(const std::string& text);
	void setBounds(float normX, float normY, float width, float height);
	void setTexture(const std::string& name);

	// Toggle enabled and visible status
	void show(void);
	void hide(void);

	// Mouse callbacks, defined in iMouseHandler
	bool mousePressed(float x, float y, OIS::MouseButtonID id);
	bool mouseReleased(float x, float y, OIS::MouseButtonID id);
	
	// Click listener interface
	void addClickListener(iClickListener *listener);
	void removeClickListener(iClickListener *listener);

	/**
	 * Enables this button for input
	 */
	void enable(void) {
		_enabled = true;
	}
	
	/**
	 * Disables this button for input
	 */
	void disable(void) {
		_enabled = false;
	}

};

};

#endif