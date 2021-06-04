/*
 * BattleSingleton.h
 *
 *  Created on: 2016-8-16
 *      Author: Ralf
 */

#ifndef BATTLESINGLETON_H_
#define BATTLESINGLETON_H_

#include "Common.h"

/*
 *  逻辑管理器的基类，继承该类后会在LogicManager中调用各个函数和虚函数
 */
class BattleSingleton
{
public:
	virtual ~BattleSingleton(){}
	BattleSingleton(){}
	virtual int OnInit(){return 0;}
	virtual int Init(){return 0;}
	virtual void OnExit(){}
	virtual void OnTimer1(){}
	virtual void OnTimer2(){}
	virtual void CallDestroy() = 0;
private:
};


#endif /* BATTLESINGLETON_H_ */
