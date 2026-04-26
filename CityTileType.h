#pragma once

#ifndef CITYTILETYPE_H_
#define CITYTILETYPE_H_

/**
* @brief Enumeration of the types of tiles.
*/
enum class CityTileType 
{ 
	Park,				/// Park tile.
	Skyscraper,			/// Skyscraper tile.
	StreetVertical,		/// Street vertical (-Z <-> +Z) tile.
	StreetHorizontal,	/// Street horizontal (-X <-> +X) tile.
	Intersection,		/// Intersection tile.
	CornerNE,			/// Road enters from North and East (Bottom-Left corner)
	CornerNW,			/// Road enters from North and West (Bottom-Right corner)
	CornerSE,			/// Road enters from South and East (Top-Left corner)
	CornerSW,			/// Road enters from South and West (Top-Right corner)
	COUNT				/// Number of elements.
};

#endif // CITYTILETYPE_H_