/*
 * ParseManager.h
 *
 *  Created on: 2015-11-12
 *      Author: Administrator
 */

#ifndef PARSEMANAGER_H_
#define PARSEMANAGER_H_

#include "Parser.h"
#include "Kernel.h"

class ParseManager
{
public:
	static ParseManager * getInstance(){
		static ParseManager pm;
		return & pm;
	}
	~ParseManager();
private:
	ParseManager();
public:
	bool Init();
	Parser * getParser() {return m_parsers;}
private:
	Parser* m_parsers;
};

#endif /* PARSEMANAGER_H_ */
