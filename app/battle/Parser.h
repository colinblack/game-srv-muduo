#ifndef _WWM_PARSER_H_
#define _WWM_PARSER_H_

#include <string>
#include <map>
#include <vector>
#include "Kernel.h"

using namespace std;

#define MAP_MOVEABLE 0x8000
struct Rect;
class Parser
{
public:
	Parser();
	~Parser();
	bool init();
public:
	bool parse(const string& path);
	uint32_t getWidth(){return m_width;};
	uint32_t getHeight(){return m_height;};
	vector<int> & getCells(){return m_cells;};
	bool isMovable(uint32_t x, uint32_t y, uint32_t type);
private:
	uint32_t m_width;
	uint32_t m_height;
	vector<int> m_cells;   //每位对应每格的占位情况
};

#endif
