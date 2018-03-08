#ifndef _GUI2D_QUADTREE_H_
#define _GUI2D_QUADTREE_H_
/**
 * @class gui2d::QuadTree
 * This is an implementation of a QuadTree, and it is used to store mouse handlers for efficient
 * lookup of which object to pass events to when they are generated
 * @tparam T The type of item that this QuadTree tracks. It MUST implement iMBR
 */

// Standard headers
#include <list>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

// Project definitions
#include "2dgui/gui2d.h"
#include "2dgui/iMBR.h"

namespace gui2d {

template <class T>
class QuadTree : public iMBR {
public:
	/**
	 * The maximum depth of the quad tree before a child node will simply stack into a list
	 */
	static const int MAX_DEPTH = 10;

	typedef typename std::list<T*> ImmediateList;
	typedef typename std::list<T*>::iterator ImmediateIter;

private:
	QuadTree *_parent;
	QuadTree *_children[4];
	int _depth;
	ImmediateList _immediates;
	bool _useImmediate;

public:
	/**
	 * Constructor, saves parameters given
	 * @param bounds The minimum bounding rectangle for this node, used to determine what to save
	 * @param parent Pointer to the parent QuadTree, may be zero for a root node
	 * @param depth The depth of this QuadTree node relative to the root, used to determine when to stop subdividing
	 */
	QuadTree(const glm::vec4& bounds, QuadTree<T> *parent, int depth)
			: iMBR(bounds), _useImmediate(true), _parent(parent), _depth(depth) {
		_children[0] = 0;
		_children[1] = 0;
		_children[2] = 0;
		_children[3] = 0;
	}

	/**
	 * Deallocate a quadtree, this also recursively deallocates quadtrees if appropriate.
	 * It never deletes the stored elements, as they may do other things
	 */
	~QuadTree(void) {
		int i;

		if (!_useImmediate) {
			for (i = 0; i < 4; ++i) {
				delete _children[i];
			}
		}
	}

	/**
	 * Checks if this node is empty, to speed some things up
	 * @return True if this QuadTree is empty, false otherwise
	 */
	bool empty(void) const {
		if (_useImmediate) {
			return _immediates.size() == 0;
		}
		else {
			return _children[0]->empty() && _children[1]->empty() && _children[2]->empty() && _children[3]->empty();
		}
	}

	/**
	 * Inserts an item into the QuadTree, doing subdivision if necessary
	 * @param node The item to insert
	 */
	void insert(T *node) {
		ImmediateIter iter;

		// Make sure that this node fits us
		if (!overlaps(*node))
			return;

		// Save it to the lists appropriately
		if (_useImmediate) {
			// If we already have four nodes here, subdivide
			if (_depth < MAX_DEPTH && _immediates.size() == 4) {
				_useImmediate = false;
				divide();
				for (iter = _immediates.begin(); iter != _immediates.end(); ++iter) {
					insert(*iter);
				}
				_immediates.clear();
				insert(node);
			}
			else {
				// Otherwise, just save it locally
				_immediates.push_back(node);
			}
		}
		else {
			// Push it off on our children
			_children[0]->insert(node);
			_children[1]->insert(node);
			_children[2]->insert(node);
			_children[3]->insert(node);
		}
	}

	/**
	 * Removes an item from the QuadTree, and if this node is empty after, it will prompt the parent
	 * to rebalance
	 * @param node The item to remove
	 */
	void remove(T *node) {
		ImmediateIter iter;
		int i;

		// Make sure this overlaps
		if (!overlaps(*node))
			return;

		// Do the removal itself
		if (_useImmediate) {
			_immediates.remove(node);
		}
		else {
			// Forward to each child (a node may be in multiple children, they will test)
			for (i = 0; i < 4; ++i) {
				_children[i]->remove(node);
			}
		}

		// Try to undivide ourselves
		undivide();
	}

	/**
	 * Locates all items whose MBRs overlap with the x, y coordinates given
	 * @param x The x coordinate of the test
	 * @param y The y coordinate of the test
	 * @param results The vector in which to store results
	 * @return The number of items added to results
	 */
	int locate(float x, float y, std::vector<T*>& results) {
		ImmediateIter iter;
		int i;
		int found = 0;

		// Make sure the children here might even hold this point
		if (!contains(x, y))
			return 0;

		if (_useImmediate) {
			for (iter = _immediates.begin(); iter != _immediates.end(); ++iter) {
				if ((*iter)->contains(x, y)) {
					found += 1;
					results.push_back(*iter);
				}
			}
		}
		else {
			for (i = 0; i < 4; ++i) {
				found += _children[i]->locate(x, y, results);
			}
		}

		return found;
	}

	/**
	 * Subdivide this node into four child nodes
	 */
	void divide(void) {
		glm::vec4 newBounds = glm::vec4(0.0f);

		newBounds[MIN_X] = _bounds[MIN_X];
		newBounds[MIN_Y] = _bounds[MIN_Y];
		newBounds[MAX_X] = (_bounds[MIN_X]+_bounds[MAX_X])/2;
		newBounds[MAX_Y] = (_bounds[MIN_Y]+_bounds[MAX_Y])/2;
		_children[0] = new QuadTree<T>(newBounds, this, _depth+1);

		newBounds[MIN_X] = newBounds[MAX_X];
		newBounds[MAX_X] = _bounds[MAX_X];
		_children[1] = new QuadTree<T>(newBounds, this, _depth+1);

		newBounds[MIN_Y] = newBounds[MAX_Y];
		newBounds[MAX_Y] = _bounds[MAX_Y];
		_children[2] = new QuadTree<T>(newBounds, this, _depth+1);

		newBounds[MAX_X] = newBounds[MIN_X];
		newBounds[MIN_X] = _bounds[MIN_X];
		_children[3] = new QuadTree<T>(newBounds, this, _depth+1);
	}

	/**
	 * If all four of our children have become empty, it will be faster to simply keep
	 * new nodes on our level, for a short while. Check for this and deallocate children
	 * if we meet the criteria. Also attempt to propagate upwards since we are now empty
	 * too.
	 */
	void undivide(void) {
		int i;

		// Only perform undivision if we're empty
		if (!empty())
			return;

		// Delete child nodes if they had been allocated
		if (!_useImmediate) {
			for (i = 0; i < 4; ++i) {
				if (!_children[i]->empty())
					return;
			}
			for (i = 0; i < 4; ++i) {
				delete _children[i];
			}

			_useImmediate = true;
		}

		// Tell our parent to try to undivide, because we're empty too now
		if (_parent)
			_parent->undivide();
	}

};

};

#endif
