#ifndef _GUI2D_H_
#define _GUI2D_H_
/**
 * @file 2dgui/gui2d.h
 * Forward declarations of everything declared in the gui2d namespace. Useful
 * for places where the methods aren't being called on any objects. Also helps
 * to resolve interdependency needs.
 */

// Standard headers
#include <list>
#include <map>
#include <string>

namespace gui2d {

	// Forward class declarations
	class Manager;
	class Font;
	class String;
	class InputBox;
	class Screen;
	class Button;
	template<typename T, typename U> class QuadRendererBase;
	class QuadRenderer;
	class TexturedQuadRenderer;
	class Statistics;
	template<typename T> class QuadTree;

	// Interfaces and interface-like classes
	class iQuadRenderable;
	class iUntexturedQuadRenderable;
	class iTexturedQuadRenderable;
	class iMouseHandler;
	class iMouseMotionHandler;
	class iClickListener;
	class iMBR;
	class iZOrderable;
	class iTransparent;
	class iVisible;

	// Interfaces I am thinking of adding, but not sure yet
	class iEnableable;	// Adds enable/disable(

	// Namespace constants
	const int SHADER_TEXT_SLOT = 2;			//!< ID used for the text shader
	const int SHADER_2DGUI_SLOT = 3;		//!< ID used for the textured quad shader
	const int SHADER_UNTEX_QUAD_SLOT = 4;	//!< ID used for the untextured quad shader

	// Text alignment constants
	const int TEXT_ALIGN_LEFT = 1;			//!< Indicates that a displayed string should be left-aligned
	const int TEXT_ALIGN_CENTER = 2;		//!< Indicates that a displayed string should be center-aligned
	const int TEXT_ALIGN_RIGHT = 3;			//!< Indicates that a displayed string should be right-aligned

	// Some namespace-wide typedefs
	typedef std::list<String*> StringList;			//!< Shorthand for a list of string instances
	typedef StringList::iterator StringListIter;	//!< Iterator type for a list of string instances

	typedef std::pair<std::string, int> FontName;	//!< Complete name for a font (size+file name)
	typedef std::map<FontName, int> FontIdMap;		//!< Maping from font name to manager-assigned font id

	typedef std::map<int, Font*> FontMap;			//!< Mapping from manager-assigned id to font name
	typedef FontMap::iterator FontMapIter;			//!< Iterator for font id->name map
	
	typedef std::map<int, StringList*> FontStringList;		//!< Mapping from font id to a list of strings using that font
	typedef FontStringList::iterator FontStringListIter;	//!< Iterator for font id->list of strings map

	typedef std::list<InputBox*> InputList;			//!< Shorthand for a list of Input instances
	typedef InputList::iterator InputListIter;		//!< Iterator for a list of Input instances

	typedef std::list<Button*> ButtonList;			//!< Shorthand for a list of Button instances
	typedef ButtonList::iterator ButtonListIter;	//!< Iterator for a list of Button instances

	typedef std::list<iClickListener *> ClickListenerList;			//!< Shorthand for a list of click listener functors
	typedef ClickListenerList::iterator ClickListenerListIter;		//!< Iterator for a list of click listeners

	typedef std::list<iVisible *> VisibleList;		//!< Shorthand for a list of items that implement the visible interface
	typedef VisibleList::iterator VisibleListIter;	//!< Iterator for a list of visibility-controllable items

};

#endif
