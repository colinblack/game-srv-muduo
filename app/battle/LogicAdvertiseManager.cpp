#include "ServerInc.h"


int LogicAdvertiseManager::Process(unsigned uid, ProtoAdvertise::UpdateAdCdReq *reqmsg, ProtoAdvertise::UpdateAdCdResp * respmsg)
{
	DBCUserBaseWrap user(uid);

	//获取秒cd需要的钻石
	unsigned cash  = AdCfgWrap().GetAdInfoCfg().cooladcost();

	//扣钻设置返回信息
	user.CostCash(cash,"create_ad");
	DataCommon::CommonItemsCPP *common = respmsg->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
	base->set_change(-cash);
	base->set_totalvalue(user.GetCash());
	base->set_type(type_cash);

	//更新ts
	unsigned ts = AdCfgWrap().GetAdInfoCfg().adcdts();
	user.UpdateCreateAdTs(Time::GetGlobalTime());
	respmsg->set_ts(user.GetCreateAdTs());
	return 0;
}


int LogicAdvertiseManager::Process(unsigned uid, ProtoAdvertise::GetAdInfoReq *reqmsg, ProtoAdvertise::GetAdInfoResp * respmsg)
{
	unsigned ad_show_max = AdCfgWrap().GetAdInfoCfg().adcnt();//最多多少个广告位
	unsigned ad_show_cnt = 0;//实际可以展示的广告条数

	//获取广告信息
	AdvertiseItem* adItem = NULL;
	adItem = (AdvertiseItem*)malloc(sizeof(AdvertiseItem)*ad_show_max);
	if(adItem == NULL)
	{
		error_log("init is failed");
		throw std::runtime_error("init is failed");
	}

	ad_show_cnt = GetAdInfo(uid,adItem,ad_show_max);

	//设置返回信息
	for(int i = 0; i < ad_show_cnt; i++)
	{
		SetAdRespMsg(adItem[i],respmsg->add_adinfo());
	}

	delete [] adItem;
	return 0;
}

int LogicAdvertiseManager::GetAdInfo(unsigned uid,AdvertiseItem adItem [], int max_count)
{
	//获取广告链表信息
	AdvertiseList m_adheadList = AdvertiseManager::Instance()->GetAdHeadList();
	AdvertiseList m_adtailList = AdvertiseManager::Instance()->GetAdTailList();

	//判定是否为空链表
	if(m_adheadList->next == m_adtailList)
		return 0;

	//获取广告总数
	unsigned total = AdvertiseManager::Instance()->GetAdCnt();

	//读取广告中合法的开始与结束位置
	unsigned start_pos = 1;
	unsigned end_pos = total;

	//在合法的区域中生成随机数
	srand((unsigned)time(NULL));
	unsigned random = rand()%(end_pos- start_pos + 1) + start_pos;//[start_pos-end_pos]区间的随机数

	int index = 0;//匹配随机数
	int cnt   = 0;//记录符合条件的广告数
	AdvertiseList node = NULL;//匹配随机数对应的节点

	AdvertiseList p = m_adheadList->next;
	while(p != m_adtailList)
	{
		index ++;
		if(random == index)//匹配随机数对应的索引位置
		{
			node = p;
			while(p != m_adtailList)//从随机节点向后遍历、查找符合条件的广告
			{
				bool unlock = LogicAdvertiseManager::Instance()->isQueryOtherAd(uid,p->data.uid,p->data.shelf_ud);
				if(p->data.uid != uid && unlock)
					adItem[cnt++]  = p->data;

				if(cnt == max_count)
					break;
				else
					p = p->next;
			}
			break;
		}
		else
			p = p->next;
	}

	//如果查找到的广告数不足、则继续往随机节点前进行遍历
	if(cnt < max_count && node != NULL)
	{
		AdvertiseList p = node->prior;
		while(p != m_adheadList)//往头结点方向遍历查找
		{
			bool unlock = LogicAdvertiseManager::Instance()->isQueryOtherAd(uid,p->data.uid,p->data.shelf_ud);
			if(p->data.uid != uid && unlock)
				adItem[cnt++]  = p->data;

			if(cnt == max_count)
				break;
			else
				p = p->prior;
		}
	}

	return cnt;
}

int LogicAdvertiseManager::SetAdRespMsg(const AdvertiseItem& adItem,ProtoAdvertise::AdInfoCPP *msg)
{
	//玩家信息
	string headUrl              = ResourceManager::Instance()->GetHeadUrl(adItem.uid);
	string name                 = ResourceManager::Instance()->GetUserName(adItem.uid);
	int level                   = ResourceManager::Instance()->GetUserLevel(adItem.uid);

	//货架物品信息
	DataShop &shop              = DataShopManager::Instance()->GetData(adItem.uid,adItem.shelf_ud);
	unsigned props_id           = shop.props_id;
	unsigned props_cnt          = shop.props_cnt;
	unsigned props_price        = shop.props_price;

	//设置返回信息
	msg->set_id(adItem.id);
	msg->set_uid(adItem.uid);
	msg->set_headurl(headUrl);
	msg->set_level(level);
	msg->set_name(name);
	msg->set_propsid(props_id);
	msg->set_propscnt(props_cnt);
	msg->set_propsprice(props_price);
	msg->set_ts(adItem.ts);
	msg->set_helpflag(adItem.help_flag);


	return 0;
}

int LogicAdvertiseManager::CreateAd(unsigned uid,unsigned shelf_ud)
{
	//校验参数
	bool is_exsit = DataShopManager::Instance()->IsExistItem(uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("param error.shelf_ud is not exsit.shelf_ud = %u",shelf_ud);
		throw std::runtime_error("param error");
	}

	//获取货架上的商品信息
	DataShop &shop = DataShopManager::Instance()->GetData(uid,shelf_ud);
	unsigned ad_flag      = shop.ad_flag;
	unsigned props_id     = shop.props_id;
	unsigned props_cnt    = shop.props_cnt;
	unsigned props_price  = shop.props_price;

	if(ad_flag == 0 || props_id == 0 || props_cnt == 0 || props_price == 0)
	{
		error_log("param error.needn't create ad.ad_flag = %u",ad_flag);
		throw std::runtime_error("param error");
	}


	AddAd(uid,shelf_ud,props_id,props_cnt,props_price);

	return 0;
}

int LogicAdvertiseManager::AddAd(unsigned uid,unsigned shelf_ud,unsigned props_id,unsigned props_cnt,unsigned props_price)
{
	unsigned ad_id =  AdvertiseManager::Instance()->GetNewAdUd();

	unsigned help_flag = LogicUserManager::Instance()->IsUserNeedHelp(uid);

	//构造广告基本信息
	AdvertiseItem ad_item(ad_id,uid,shelf_ud,help_flag);

	//添加广告信息
	AdvertiseManager::Instance()->AddAdInfo(ad_item);

	//更新货架上的广告标志信息
	DataShop &shop = DataShopManager::Instance()->GetData(uid,shelf_ud);
	DataShopManager::Instance()->UpdateItem(shop);
	return 0;
}

int LogicAdvertiseManager::DelOldAd()
{
	set<unsigned > uidset;//记录load用户，以免重复load档
	uidset.clear();

	AdvertiseList phead = AdvertiseManager::Instance()->GetAdHeadList();
	AdvertiseList ptail = AdvertiseManager::Instance()->GetAdTailList();
	unsigned time_cfg = AdCfgWrap().GetAdInfoCfg().adtime();

	while(phead ->next != ptail)
	{
		unsigned ts       = phead->next->data.ts;
		unsigned cur_ts   = Time::GetGlobalTime();
		bool deleteAd     = false;//是否需要删除广告
		int ret = 0;
		debug_log("show_all_ad.uid=%u,shelf_ud=%u",phead->next->data.uid,phead->next->data.shelf_ud);

		if(!UserManager::Instance()->IsOnline(phead->next->data.uid))
		{
			set<unsigned > ::iterator it = uidset.find(phead->next->data.uid);
			if(it == uidset.end())
			{
				ret = UserManager::Instance()->LoadArchives(phead->next->data.uid);
				uidset.insert(phead->next->data.uid);
			}
		}

		if(ret == 0)
		{
			bool is_exsit = DataShopManager::Instance()->IsExistItem(phead->next->data.uid,phead->next->data.shelf_ud);
			if(is_exsit) {
				DataShop & shop = DataShopManager::Instance()->GetData(phead->next->data.uid,phead->next->data.shelf_ud);

				//如果时间已到、或者物品已被购买(此时需删除广告，并更新对应的商店数据)
				if(cur_ts - ts >= time_cfg || shop.sell_flag == 1)
				{
					deleteAd = true;
					debug_log("normal_delete_ad.uid=%u,shelf_ud=%u",phead->next->data.uid,phead->next->data.shelf_ud);
				}

				//是否需要更新商店数据
				if(deleteAd) {
					//更新商店里的广告标记、并且由系统购买商店里的东西
					LogicShopManager::Instance()->UpdateAdFlag(phead->next->data.uid, phead->next->data.shelf_ud);

					//保存对离线用户数据的更改
					UserManager::Instance()->SyncSave(phead->next->data.uid);
				}
			}

		}else{
			//如果load档失败,删除广告
			deleteAd = true;
			debug_log("abnormal_delete_ad.uid=%u,shelf_ud=%u",phead->next->data.uid,phead->next->data.shelf_ud);
		}

		//是否需要删除广告
		if(deleteAd)
		{
			//更新对应的缓存数据
			AdvertiseManager::Instance()->Del(phead->next->data.id);

			//删除对应的广告链表节点
			AdvertiseList node = phead ->next;
			node->prior->next  = node->next;
			node->next->prior  = node->prior;
			delete node;
		}else {
			phead = phead->next;
		}
	}

	return 0;
}


int LogicAdvertiseManager::DelAd(unsigned uid,unsigned shelf_ud)
{
	unsigned id  = AdvertiseManager::Instance()->GetAdId(uid,shelf_ud);
	bool is_exsit = AdvertiseManager::Instance()->Has(id);
	if(is_exsit)
	{
		//删除广告
		AdvertiseManager::Instance()->DelAdInfo(uid,shelf_ud);
	}
	return 0;
}

bool LogicAdvertiseManager::isQueryOtherAd(unsigned uid,unsigned otheruid,unsigned shelf_ud)
{
	bool rlt = false;

	//加载别的玩家数据
	UserManager::Instance()->LoadArchives(otheruid);
	bool is_exsit = DataShopManager::Instance()->IsExistItem(otheruid,shelf_ud);

	if(is_exsit)
	{
		//获取物品解锁等级
		DataShop &shop    = DataShopManager::Instance()->GetData(otheruid,shelf_ud);
		unsigned propsid  = shop.props_id;
		const ConfigItem::PropItem &props_cfg = ItemCfgWrap().GetPropsItem(propsid);
		unsigned unlock_level = props_cfg.unlock_level();

		//获取玩家当前等级
		DBCUserBaseWrap userwrap(uid);
		unsigned user_level = userwrap.Obj().level;

		//商店物品已解锁、并且没有被购买、则返回true
		if(user_level >= unlock_level)
			rlt = true;
	}
	return rlt;
}

