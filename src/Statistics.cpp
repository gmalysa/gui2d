/**
 * @file 2dgui/Statistics.cpp
 * @todo License/copyright statement
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Project definitions
#include "2dgui/Statistics.h"
#include "2dgui/Manager.h"
#include "2dgui/String.h"
#include "GraphicsEngine.h"

/**
 * The constructor will create the default strings for all of the message that it must display
 * as well as configure them all to be invisible for the time being
 * @param ge A pointer to the GraphicsEngine, which we will later use to retrieve runtime statistics
 * @param m The gui2d::Manager that owns the relevant drawing resources
 * @param fontId The font ID to use when creating the strings
 */
gui2d::Statistics::Statistics(GraphicsEngine *ge, gui2d::Manager *m, int fontId) : iUntexturedQuadRenderable(5), _ge(ge),
		_m(m), _font(fontId), _extended(false) {
	float px, py;

	// Get kludgey pixel widths to use for padding
	px = m->getPixelWidth();
	py = m->getPixelHeight();

	// String initial offsets
	_x = -1.0f + 2*px;
	_y = -1.0f + 1*py;

	// Create the strings
	_fpsDisplay = _m->createString(fontId, "FPS: <>", _x, _y);
	_glPrimitives = _m->createString(fontId, "GL Primitives: <>", _x, _fpsDisplay->getY() + _fpsDisplay->getHeightf());
	_glSamples = _m->createString(fontId, "GL Samples: <>", _x, _glPrimitives->getY() + _glPrimitives->getHeightf());
	_glTime = _m->createString(fontId, "GL Time: <>", _x, _glSamples->getY() + _glSamples->getHeightf());
	_physicsTime = _m->createString(fontId, "Phys Time: <>", _x, _glTime->getY() + _glTime->getHeightf());

	// Set strings to be the right colors
	_fpsDisplay->setColor(glm::vec4(1.0f));
	_glPrimitives->setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	_glSamples->setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	_glTime->setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	_physicsTime->setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

	// Set up quad positions
	_w = 0.325f;
	_h = _fpsDisplay->getHeightf();
	updatePositions();

	// Set quad colors
	setQuadColor(QUAD_BG, glm::vec4(0.2f, 0.2f, 0.2f, 0.8f));
	setQuadColor(QUAD_LBORDER, glm::vec4(0.0f, 1.0f, 0.0f, 0.8f));
	setQuadColor(QUAD_RBORDER, glm::vec4(0.0f, 1.0f, 0.0f, 0.8f));
	setQuadColor(QUAD_TBORDER, glm::vec4(0.0f, 1.0f, 0.0f, 0.8f));
	setQuadColor(QUAD_BBORDER, glm::vec4(0.0f, 1.0f, 0.0f, 0.8f));

	// By default, we show nothing
	_fpsDisplay->hide();
	_glPrimitives->hide();
	_glSamples->hide();
	_glTime->hide();
	_physicsTime->hide();
}

/**
 * Destructor for this class will remove it from the Manager's quad display and deallocate
 * all of the contained strings
 */
gui2d::Statistics::~Statistics(void) {
	_m->hideQuads(this);
	delete _fpsDisplay;
	delete _glPrimitives;
	delete _glTime;
	delete _glSamples;
	delete _physicsTime;
}

/**
 * Show the statistics on screen
 */
void gui2d::Statistics::show(void) {
	_show();
	_fpsDisplay->show();
	_m->showQuads(this);

	if (_extended) {
		_glPrimitives->show();
		_glSamples->show();
		_glTime->show();
		_physicsTime->show();
	}
}

/**
 * Hide statistics on screen
 */
void gui2d::Statistics::hide(void) {
	_hide();
	_fpsDisplay->hide();
	_m->hideQuads(this);

	if (_extended) {
		_glPrimitives->hide();
		_glSamples->hide();
		_glTime->hide();
		_physicsTime->hide();
	}
}

/**
 * Set the quad positions to surround an area given by x, y, w, h with 1px padding and 1px border
 */
void gui2d::Statistics::updatePositions(void) {
	float px, py;

	// Get kludgey pixel widths to use for padding
	px = _m->getPixelWidth();
	py = _m->getPixelHeight();

	setQuadXY(QUAD_BG, _x - px, _x - py, _w + 2*px, _h + 2*py);
	setQuadXY(QUAD_LBORDER, _x - 2*px, _y - 2*py, px, _h + 4*py);
	setQuadXY(QUAD_RBORDER, _x + _w + 1*px, _y - 2*py, px, _h + 4*py);
	setQuadXY(QUAD_BBORDER, _x - 2*px, _y - 2*py, _w + 4*px, py);
	setQuadXY(QUAD_TBORDER, _x - 2*px, _y + _h + 2*py, _w + 4*px, py);
}

/**
 * Show extended stats and redraw the box to surround them
 */
void gui2d::Statistics::showExtended(void) {
	// Redo quad positioning for a taller box
	_h = 5*_fpsDisplay->getHeightf();
	updatePositions();
	_extended = true;

	// If stats are displayed, show extended now too
	if (_visible) {
		_glPrimitives->show();
		_glTime->show();
		_glSamples->show();
		_physicsTime->show();
	}
}

/**
 * Hide the extended stats and redraw the smaller box
 */
void gui2d::Statistics::hideExtended(void) {
	// Redo quad positioning for a taller box
	_h = _fpsDisplay->getHeightf();
	updatePositions();
	_extended = false;

	// If stats are displayed, show extended now too
	if (_visible) {
		_glPrimitives->hide();
		_glTime->hide();
		_glSamples->hide();
		_physicsTime->hide();
	}
}

/**
 * Change the Z-index for this element. The standard rules apply, and the text is drawn at z-5 compared
 * to where this element is set to. Note that Z is given in natural values, NOT normalized.
 * @param z The new z index to use
 */
void gui2d::Statistics::setZ(float z) {
	setZ(static_cast<GLushort>(z));
}

/**
 * Change the Z-index for this element, using a short instead of a float,
 * @param z The new z index to use
 */
void gui2d::Statistics::setZ(GLushort z) {
	GLushort adjz = z - 5;
	setQuadsZ(z);
	_fpsDisplay->setZ(adjz);
	_glPrimitives->setZ(adjz);
	_glTime->setZ(adjz);
	_glSamples->setZ(adjz);
	_physicsTime->setZ(adjz);
}

/**
 * Update will prompt us to read out the previous values for the stats we're tracking
 * from the GraphicsEngine and update our strings.
 * @param ts The timestep since last update. This parameter is required for compatibility, but ignored
 */
void gui2d::Statistics::update(float ts) {
	std::stringstream ss;

	if (_visible) {
		// Update raw FPS
		ss << _ge->getFPS();
		_fpsDisplay->remove(5, 4);
		_fpsDisplay->append(ss.str());

		// Update extended stats
		if (_extended) {
			_glPrimitives->remove(15);
			_glSamples->remove(12);
			_glTime->remove(9);
	
			ss.str(std::string());
			ss.clear();
			ss << _ge->getPrimitivesGenerated();
			_glPrimitives->append(ss.str());

			ss.str(std::string());
			ss.clear();
			ss << _ge->getSamplesPassed();
			_glSamples->append(ss.str());

			ss.str(std::string());
			ss.clear();
			ss << _ge->getTimeElapsed()/1000;
			_glTime->append(ss.str());
		}
	}
}

/**
 * Because physics time is received by a message, and not every frame, it is updated
 * separately, on receipt of message
 * @param pt The physics time to update to, in usec
 */
void gui2d::Statistics::setPhysicsTime(float pt) {
	std::stringstream ss;

	ss << pt;
	_physicsTime->remove(11);
	_physicsTime->append(ss.str());
}
