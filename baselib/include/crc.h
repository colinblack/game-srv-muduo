/*
 * crc.h
 *
 *  Created on: 2011-1-9
 *      Author: zhihaoliu
 */

#ifndef CRC_H_
#define CRC_H_

#include <string>

class CCRC {
public:
	static unsigned int GetCrc32(std::string s);
	static unsigned int GetCrc32(const char* p,unsigned int len);
	static unsigned short GetCrc16(std::string s);
	static unsigned short GetCrc16(const char* p,unsigned int len);

private:
	static bool bcrc32init;
	static bool bcrc16init;
};

#endif /* CRC_H_ */
