#ifndef _GUI2D_STATISTICS_H_
#define _GUI2D_STATISTICS_H_
/**
 * @class gui2d::Statistics
 * This class handles updating and displaying the statistics that are calculated by
 * the graphics thread.
 */

// Standard headers
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <string>

// Project definitions
#include "sks.h"
#include "2dgui/gui2d.h"
#include "2dgui/iUntexturedQuadRenderable.h"
#include "2dgui/iVisible.h"
#include "2dgui/iZOrderable.h"

namespace gui2d {

class Statistics : public iUntexturedQuadRenderable, public iVisible, public iZOrderable {
private:
	// Quad numbers to simplify management
	static const int QUAD_BG = 0;			///< Quad number for the background
	static const int QUAD_LBORDER = 1;		///< Quad number for the left border
	static const int QUAD_RBORDER = 2;		///< Quad number for the right border
	static const int QUAD_TBORDER = 3;		///< Quad number for the top border
	static const int QUAD_BBORDER = 4;		///< Quad number for the bottom border

	// Important pointers and configuration
	GraphicsEngine *_ge;
	Manager *_m;
	int _font;

	// Drawn on screen strings
	String *_fpsDisplay;
	String *_glPrimitives;
	String *_glTime;
	String *_glSamples;
	String *_physicsTime;

	// Visibility status for either just fps display, or full display
	bool _extended;

	// Dimensions/position information to simplify border recalculations
	// Note these positions apply to the bottom left corner of the text,
	// the box (including border) is drawn two pixels wider in each direction
	float _x, _y, _w, _h;

	void updatePositions(void);

public:
	Statistics(GraphicsEngine *ge, Manager *m, int fontId);
	~Statistics(void);

	void show(void);
	void hide(void);
	void showExtended(void);
	void hideExtended(void);
	void setZ(float z);
	void setZ(GLushort z);

	void update(float ts);

	void setPhysicsTime(float pt);

	/**
	 * Returns extended status visibility information
	 */
	bool isExtendedVisible(void) const { return _extended; }
};

};

#endif