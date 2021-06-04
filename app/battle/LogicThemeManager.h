/*
 * LogicThemeManager.h
 *
 *  Created on: 2018年6月19日
 *      Author: hector
 */

#ifndef APP_BATTLE_LOGICTHEMEMANAGER_H_
#define APP_BATTLE_LOGICTHEMEMANAGER_H_

#include "ServerInc.h"

class LogicThemeManager : public BattleSingleton, public CSingleton<LogicThemeManager>
{
private:
	friend class CSingleton<LogicThemeManager>;
	LogicThemeManager(): actid(e_Activity_Theme){}
	int actid;
public:
	enum{
		theme_id_use_max        = 8,//(activity data 中前8个uint32共32字节存储子项当前使用的主题ID)
	};

	void CallDestroy() {Destroy();}

	int CheckLogin(unsigned uid);

	//首充奖励主题
	int FirstChargeRewardTheme(unsigned uid,ProtoTheme::ThemeInfoResp *resp);

	//查询
	int Process(unsigned uid, ProtoTheme::ThemeInfoReq *req, ProtoTheme::ThemeInfoResp *resp);
	//购买
	int Process(unsigned uid, ProtoTheme::ThemeBuyReq *req, ProtoTheme::ThemeBuyResp *resp);
	//应用
	int Process(unsigned uid, ProtoTheme::ThemeUseReq *req, ProtoTheme::ThemeInfoResp *resp);
public:
	int FillTheme(DataGameActivity& activity, ProtoTheme::ThemeInfoResp *resp);
};

#endif /* APP_BATTLE_LOGICTHEMEMANAGER_H_ */
