#ifndef _IMBR_H_
#define _IMBR_H_
/**
 * @class gui2d::iMBR
 * Interface for all elements that have a minimum bounding rectangle
 */

// Standard headers
#include <glm/glm.hpp>

// Project definitions
#include "2dgui/gui2d.h"

namespace gui2d {

class iMBR {
protected:
	//! The minimum bounding rectangle
	glm::vec4 _bounds;

	/**
	 * Adjust the MBR based on the given description, does the actual work
	 * @param normX Normalized x coordinate for boundary
	 * @param normY Normalized y coordinate for boundary
	 * @param width Width in normalized coordinates for the button
	 * @param height Height in normalized coordinates for the button
	 */
	void _setBounds(float normX, float normY, float width, float height) {
		// Sanity check on width and height
		width = width < 0.0f ? 0.0f : width;
		height = height < 0.0f ? 0.0f : height;

		// Update our positions with the MBR parent
		_bounds[MIN_X] = glm::clamp(normX, -1.0f, 1.0f);
		_bounds[MAX_X] = glm::clamp(normX + width, -1.0f, 1.0f);
		_bounds[MIN_Y] = glm::clamp(normY, -1.0f, 1.0f);
		_bounds[MAX_Y] = glm::clamp(normY + height, -1.0f, 1.0f);
	}

public:
	/**
	 * Saves the given bounding rectangle
	 * @param bounds The bounding rectangle that describes this MBR
	 */
	iMBR(const glm::vec4& bounds) : _bounds(bounds) {}

	/**
	 * The destructor does nothing since this has no allocated memory
	 */
	virtual ~iMBR() {}

	/**
	 * Adjust the MBR to the new one given
	 * @param bounds The new MBR to use
	 */
	virtual void setBounds(const glm::vec4& bounds) {
		this->setBounds(bounds[MIN_X], bounds[MIN_Y], bounds[MAX_X]-bounds[MIN_X], bounds[MAX_Y]-bounds[MIN_Y]);
	}

	/**
	 * Bland interface bounds adjustor
	 * @param normX Normalized x coordinate for boundary
	 * @param normY Normalized y coordinate for boundary
	 * @param width Width in normalized coordinates for the button
	 * @param height Height in normalized coordinates for the button
	 */
	virtual void setBounds(float normX, float normY, float width, float height) {
		_setBounds(normX, normY, width, height);
	}

	/**
	 * Retrieves the MBR
	 * @return 4-float vector specifying min/max for x and y, indexed by the constants given
	 */
	const glm::vec4& getMBR() const {
		return _bounds;
	}

	/**
	 * Tests if the given coordinates are contained within this MBR
	 * @param x The x coordinate to test
	 * @param y The y coordinate to test
	 * @return True if it is contained, false otherwise
	 */
	bool contains(float x, float y) const {
		return (x >= _bounds[MIN_X] && x <= _bounds[MAX_X]) && (y >= _bounds[MIN_Y] && y <= _bounds[MAX_Y]);
	}

	/**
	 * Determines if these two MBRs overlap, which is necessary for sorting them into QuadTrees
	 * @param other The other MBR object to compare
	 * @return True if they overlap, false otherwise
	 */
	bool overlaps(const iMBR& other) const {
		const glm::vec4& otherBounds = other.getMBR();

		// Individual tests
		// Too far to the left
		bool leftFail = otherBounds[MAX_X] < _bounds[MIN_X];

		// Too far to the right
		bool rightFail = otherBounds[MIN_X] > _bounds[MAX_X];

		// Too far below
		bool botFail = otherBounds[MAX_Y] < _bounds[MIN_Y];

		// Too far above
		bool topFail = otherBounds[MIN_Y] > _bounds[MAX_Y];

		return !(leftFail || rightFail || botFail || topFail);
	}

public:
	static const int MIN_X = 0;		//!< Index of the min x coordinate
	static const int MAX_X = 1;		//!< Index of the max x coordinate
	static const int MIN_Y = 2;		//!< Index of the min y coordinate
	static const int MAX_Y = 3;		//!< Index of the max y coordinate

};

};

#endif
