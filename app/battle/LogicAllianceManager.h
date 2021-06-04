#ifndef LOGIC_ALLIANCE_MANAGER_H_
#define LOGIC_ALLIANCE_MANAGER_H_

#include "ServerInc.h"

class OtherAllianceSaveControl
{
public:
	OtherAllianceSaveControl(unsigned aid);

	~OtherAllianceSaveControl();
private:
	unsigned aid_;
};

class LogicAllianceManager : public BattleSingleton, public CSingleton<LogicAllianceManager>
{
private:
	friend class CSingleton<LogicAllianceManager>;
	LogicAllianceManager();
	CDataAllianceMapping allianceMapping;

	virtual ~LogicAllianceManager(){}
public:
	enum
	{
		functional_type_apply = 1,  //申请信息
		functional_type_donation = 2,  //捐收信息
		functional_type_help = 3,  //帮助信息
		functional_type_invite = 4,  //邀请列表
		functional_type_all = 5,  //全部

		join_type_anyone = 1, //任何人可以加入
		join_type_apply = 2,  //申请加入
		join_type_invite = 3, //邀请加入
		join_type_max, //

		approve_operate_pass = 1,  //批准通过
		approve_operate_reject = 2, //拒绝

		accept_operate_yes = 1,  //接受邀请
		accept_operate_no = 2, //拒绝邀请

		operate_promotion = 1, //升职
		operate_demotion = 2, //降职

		manipute_type_committee = 1, //委员
		manipute_type_director	= 2, //理事

		apply_join_allow = 1, // 允许入会
		apply_join_apply = 2, // 申请入会
		apply_join_nonexist = 3, // 联盟不存在
		apply_join_invite_only = 4, // 仅限邀请
		apply_join_level_limit = 5, // 等级限制
		apply_join_already_apply = 6, // 已经申请过
		apply_join_member_full = 7, // 联盟已满

		race_order_operate_accept = 1, 	// 接受竞赛订单
		race_order_operate_del = 2, 	// 撕掉竞赛订单
		race_order_operate_cut_cd = 3, 	// 秒竞赛订单CD


		race_order_operate_success = 0, 	// 竞赛订单操作成功
		race_order_operate_id_not_exist = 1, 	// 竞赛订单不存在
		race_order_operate_id_exist = 2, 	// 竞赛订单已存在
		race_order_operate_in_cd = 3, 		// 竞赛订单在CD中

		flag_id_race_order_buy_chance = 0,				// 购买竞赛订单
		flag_id_race_order_confirm_grade_reward = 1,	// 查看竞赛等级奖励

		race_member_order_status_doing = 1,	// 订单进行中
		race_member_order_status_finish = 2,	// 订单完成
		race_member_order_status_cancel = 3,	// 订单取消

		watch_ad_type_0 = 0,	//0:请求是否可以观看广告
		watch_ad_type_1 = 1,	//1:看完广告后领取奖励

		watch_ad_result_0 = 0,	//0:可以观看
		watch_ad_result_1 = 1,	//1:不可以观看,当天任务已经用完
		watch_ad_result_2 = 2,	//2:成功领取奖励
		watch_ad_result_3 = 3,	//3:不可以观看,商会竞赛还未开始
		watch_ad_result_4 = 4	//4:不可以观看,未参加商会竞赛
	};

	virtual void CallDestroy() {Destroy();}
	virtual int OnInit();
	void OnRaceSettle();

	int Online(unsigned uid);
	int Offline(unsigned uid);
	// 设置联盟成员数据
	int UpdateMemberNow(unsigned uid);
	int UpdateMember(unsigned uid, unsigned aid, unsigned onlineTs, unsigned helpTs, unsigned level, const string& name, unsigned vipLevel);
	int UpdateMemberHelpTs(unsigned uid);
	int UpdateMemberLevel(unsigned uid);

	int Process(unsigned uid, ProtoAlliance::RequestAlliance* req);
	int Process(ProtoAlliance::RequestAllianceBC* req);
	int Process(ProtoAlliance::ReplyAllianceBC* req);

	//获取商会功能信息
	int Process(unsigned uid, ProtoAlliance::GetAllianceFunctionReq* req);
	int Process(ProtoAlliance::RequestAllianceFunctionBC* req);
	int Process(ProtoAlliance::ReplyAllianceFunctionBC* req);

	//获取商会通知
	int Process(unsigned uid, ProtoAlliance::GetNotifyReq* req);
	int Process(ProtoAlliance::RequestAllianceNotifyBC* req);
	int Process(ProtoAlliance::ReplyAllianceNotifyBC* req);

	//获取商会成员
	int Process(unsigned uid, ProtoAlliance::GetMemberReq* req);
	int Process(ProtoAlliance::RequestAllianceMemberBC* req);
	int Process(ProtoAlliance::ReplyAllianceMemberBC* req);

	//商会名称校验--
	int Process(unsigned uid, ProtoAlliance::CheckNameAvailableReq* req, ProtoAlliance::CheckNameAvailableResp* resp);

	//创建商会--
	int Process(unsigned uid, ProtoAlliance::CreateAllianceReq* req, ProtoAlliance::CreateAllianceResp* resp);

	//商会推荐--本地共享内存
	int Process(unsigned uid, ProtoAlliance::RecommendllianceReq* req, ProtoAlliance::RecommendllianceResp* resp);

	//商会批量获取--禁止跨服
	int Process(unsigned uid, ProtoAlliance::GetPartAllianceInfoReq* req, ProtoAlliance::GetPartAllianceInfoResp* resp);

	//申请加入商会
	int Process(unsigned uid, ProtoAlliance::ApplyJoinReq* req);
	int Process(ProtoAlliance::RequestApplyJoinBC* req);
	int Process(ProtoAlliance::ReplyApplyJoinBC* req);

	//批准入会操作
	int Process(unsigned uid, ProtoAlliance::ApproveJoinReq* req);
	int Process(ProtoAlliance::RequestApproveJoinAllianceBC* req);
	int Process(ProtoAlliance::RequestApproveJoinUserBC* req);
	int Process(ProtoAlliance::ReplyApproveJoinBC* req);

	//退出商会
	int Process(unsigned uid, ProtoAlliance::ExitAllianceReq* req);
	int Process(ProtoAlliance::RequestExitAllianceBC* req);
	int Process(ProtoAlliance::ReplyExitAllianceBC* req);

	//邀请入会
	int Process(unsigned uid, ProtoAlliance::InviteJoinReq* req);
	int Process(ProtoAlliance::RequestInviteJoinBC* req);
	int Process(ProtoAlliance::ReplyInviteJoinBC* req);
	int Process(ProtoAlliance::RequestInviteJoinUserBC* req);

	//接受邀请
	int Process(unsigned uid, ProtoAlliance::AcceptInviteReq* req);
	int Process(ProtoAlliance::RequestAcceptInviteBC* req);
	int Process(ProtoAlliance::ReplyAcceptInviteBC* req);

	//成员职位调整
	int Process(unsigned uid, ProtoAlliance::ManipulateMemberReq* req);
	int Process(ProtoAlliance::RequestManipulateMemberBC* req);
	int Process(ProtoAlliance::ReplyManipulateMemberBC* req);

	//踢出成员
	int Process(unsigned uid, ProtoAlliance::KickOutReq* req);
	int Process(ProtoAlliance::RequestKickOutBC* req);
	int Process(ProtoAlliance::ReplyKickOutBC* req);
	int Process(ProtoAlliance::RequestKickOutMemberBC* req);

	//转任会长
	int Process(unsigned uid, ProtoAlliance::TransferReq* req);
	int Process(ProtoAlliance::RequestTransferBC* req);
	int Process(ProtoAlliance::ReplyTransferBC* req);

	//编辑商会
	int Process(unsigned uid, ProtoAlliance::EditAllianceReq* req);
	int Process(ProtoAlliance::RequestEditAllianceBC* req);
	int Process(ProtoAlliance::ReplyEditAllianceBC* req);

	//发起捐收
	int Process(unsigned uid, ProtoAlliance::SeekDonationReq* req);
	int Process(ProtoAlliance::RequestSeekDonationBC* req);
	int Process(ProtoAlliance::ReplySeekDonationBC* req);

	//秒捐收cd
	int Process(unsigned uid, ProtoAlliance::CutUpDonationCDReq* req, ProtoAlliance::CutUpDonationCDResp* resp);

	//提供捐收
	int Process(unsigned uid, ProtoAlliance::OfferDonationReq* req);
	int Process(ProtoAlliance::RequestOfferDonationBC* req);
	int Process(ProtoAlliance::ReplyOfferDonationBC* req);

	//提取已捐收物品
	int Process(unsigned uid, ProtoAlliance::FetchDonationReq* req);
	int Process(ProtoAlliance::RequestFetchDonationBC* req);
	int Process(ProtoAlliance::ReplyFetchDonationBC* req);

	//发布通知
	int Process(unsigned uid, ProtoAlliance::SendNotifyReq* req);
	int Process(ProtoAlliance::RequestSendNotifyBC* req);
	int Process(ProtoAlliance::ReplySendNotifyBC* req);

	//删除通知
	int Process(unsigned uid, ProtoAlliance::DelNotifyReq* req);
	int Process(ProtoAlliance::RequestDelNotifyBC* req);
	int Process(ProtoAlliance::ReplyDelNotifyBC* req);

	//更新成员信息
	int Process(ProtoAlliance::RequestUpdateMemberBC* req);
	int Process(ProtoAlliance::RequestAddMemberHelpTimesBC* req);

	//设置商会成员竞赛标志
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceSetFlag* req);
	int Process(ProtoAlliance::RequestAllianceRaceSetFlagBC* req);

	//竞赛订单完成进度
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceMemberProgress* req);
	int Process(ProtoAlliance::RequestAllianceRaceMemberProgressBC* req);


	//查询竞赛信息
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceInfo* req);
	int Process(ProtoAlliance::RequestAllianceRaceInfoBC* req);

	//查询竞赛订单
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceOrder* req);
	int Process(ProtoAlliance::RequestAllianceRaceOrderBC* req);

	//操作竞赛订单(接受、拒绝、秒cd)
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceOperateOrder* req);
	int Process(ProtoAlliance::RequestAllianceRaceOperateOrderBC* req);
	int Process(ProtoAlliance::ReplyAllianceRaceOperateOrder* req);

	//删除商会竞赛成员订单
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceMemberDelOrder* req);
	int Process(ProtoAlliance::RequestAllianceRaceMemberDelOrderBC* req);

	//更新商会竞赛成员订单
	int Process(ProtoAlliance::RequestAllianceRaceMemberUpdateOrderBC* req);

	//竞赛购买订单
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceBuyOrder* req);
	int Process(ProtoAlliance::RequestAllianceRaceBuyOrderBC* req);
	int Process(ProtoAlliance::ReplyAllianceRaceBuyOrder* req);

	//查询竞赛奖励
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceReward* req);
	int Process(ProtoAlliance::RequestAllianceRaceRewardBC* req);

	//领取竞赛等级奖励
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceTakeGradeReward* req);
	int Process(ProtoAlliance::RequestAllianceRaceTakeGradeRewardBC* req);
	int Process(ProtoAlliance::ReplyAllianceRaceTakeGradeReward* req);

	//领取竞赛阶段奖励
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceTakeStageReward* req);
	int Process(ProtoAlliance::RequestAllianceRaceTakeStageRewardBC* req);
	int Process(ProtoAlliance::ReplyAllianceRaceTakeStageReward* req);

	//刷新竞赛阶段奖励
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceRefreshStageReward* req);
	int Process(ProtoAlliance::RequestAllianceRaceRefreshStageRewardBC* req);
	int Process(ProtoAlliance::ReplyAllianceRaceRefreshStageReward* req);

	//竞赛成员日志
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceMemberOrderLog* req);
	int Process(ProtoAlliance::RequestAllianceRaceMemberOrderLogBC* req);

	//竞赛个人日志
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRacePersonOrderLog* req);
	int Process(ProtoAlliance::RequestAllianceRacePersonOrderLogBC* req);

	//跨服设置商会竞赛分组积分
	int Process(ProtoAlliance::SetAllianceRaceGroupPointBC* req);

	//请求商会竞赛分组成员
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceGroupMember* req);
	int Process(ProtoAlliance::RequestAllianceRaceGroupMemberBC* req);

	//看广告获得任务奖励
	int Process(unsigned uid, ProtoAlliance::RequestAllianceRaceWatchAd* req);
	int Process(ProtoAlliance::RequestAllianceRaceWatchAdBC* req);

	// 跨服设置联盟成员数据
	int AddMemberHelpTimesLocal(unsigned uid, unsigned aid);
	int LoadAlliance(unsigned alliance_id);
	int SaveAlliance(unsigned alliance_id);
	//更新商会竞赛订单完成进度
	int AddRaceOrderProgress(uint32_t uid, uint32_t orderType, uint32_t count, uint32_t target = 0);
	// 检查商会竞赛版本
	int CheckRace(uint32_t aid);

	//观看广告任务每日更新
	bool UpdateWatchAd();
private:
	//获取商会功能信息
	int GetAllianceFunc(unsigned uid,  unsigned type);
	//获取本地商会功能信息
	int GetAllianceFuncLocal(unsigned alliance_id, unsigned uid,  unsigned type, ProtoAlliance::GetAllianceFunctionResp * resp);

	//获取商会通知
	int GetAllianceNotify(unsigned uid);

	//获取商会成员
	int GetMembers(unsigned uid, unsigned alliance_id, ProtoAlliance::GetMemberResp *resp);

	int FullMessage(unsigned alliance_id, google::protobuf::RepeatedPtrField<ProtoAlliance::AllianceMemberCPP >* msg);

	//商会名称校验
	int CheckName(unsigned uid, string &name, ProtoAlliance::CheckNameAvailableResp * resp);

	//创建商会
	int CreateAlliance(unsigned uid, ProtoAlliance::CreateAllianceReq * req, ProtoAlliance::CreateAllianceResp * resp);

	//商会推荐
	int RecommendAlliance(unsigned uid, ProtoAlliance::RecommendllianceResp * resp);

	//批量获取商会
	int GetBatchAllianceInfo(unsigned uid, vector<unsigned> & allianceids, ProtoAlliance::GetPartAllianceInfoResp * resp);

	//申请加入商会
	int ApplyJoin(unsigned uid, unsigned alliance_id, string & reason);
	int ApplyJoinLocal(DataAllianceMember& member, const string & reason, ProtoAlliance::ApplyJoinResp * resp);

	//批准入会操作
	int ApproveJoin(unsigned uid, unsigned apply_uid, unsigned operate);
	int ApproveJoinUpdateAlliance(unsigned uid, unsigned operate, DataAllianceMember& member, ProtoAlliance::ApproveJoinResp * resp);
	int ApproveJoinUpdateUser(unsigned uid, unsigned aid, unsigned apply_uid, unsigned operate);

	//退出商会
	int ExitAlliance(unsigned uid);
	int ExitAllianceUpdateAlliance(unsigned uid, unsigned aid);
	int ExitAllianceUpdateUser(unsigned uid, unsigned aid, ProtoAlliance::ExitAllianceResp * resp);

	//邀请入会
	int InviteJoin(unsigned uid, unsigned invited_uid);
	int InviteJoinUpdateAlliance(unsigned uid, unsigned aid, unsigned invited_uid, const string& userName);
	int InviteJoinUpdateUser(unsigned uid, unsigned aid, unsigned invited_uid, uint8_t aflag, const string& inviteName, const string& aname, ProtoAlliance::InviteJoinResp * resp);

	//接受邀请
	int AcceptInvite(unsigned uid, unsigned alliance_id, unsigned operate);
	int AcceptInviteUpdateAlliance(unsigned invite_uid, DataAllianceMember& member, ProtoAlliance::AcceptInviteResp * resp);

	//成员职位调整
	int ManipulateMember(unsigned uid, ProtoAlliance::ManipulateMemberReq* req);
	int ManipulateMemberLocal(unsigned uid, unsigned aid, unsigned member_uid, unsigned operate, unsigned type, unsigned dest, ProtoAlliance::ManipulateMemberResp * resp);

	//踢出成员
	int KickOut(unsigned uid, unsigned member_uid);
	int KickOutUpdateAlliance(unsigned uid, unsigned aid, unsigned member_uid);
	int KickOutUpdateMember(unsigned aid, unsigned member_uid);

	//转任会长
	int TransformChief(unsigned uid, unsigned member_uid, const string& name);
	int TransformChiefLocal(unsigned uid, unsigned aid, unsigned member_uid, const string& otherName, ProtoAlliance::TransferResp * resp);

	//编辑商会
	int EditAlliance(unsigned uid, ProtoAlliance::EditAllianceReq* req);
	int EditAllianceLocal(unsigned uid, unsigned aid, ProtoAlliance::EditAllianceReq* req, ProtoAlliance::EditAllianceResp* resp);

	//发起捐收
	int SeekDonation(unsigned uid, unsigned propsid, unsigned count);
	int SeekDonationUpdateAlliance(unsigned uid, unsigned aid, unsigned level, const string& name, unsigned propsid, unsigned count, ProtoAlliance::SeekDonationResp * resp);
	int SeekDonationUpdateUser(unsigned uid, unsigned cdtime, ProtoAlliance::SeekDonationResp * resp);

	//秒捐收cd
	int CutCD(unsigned uid, unsigned type,ProtoAlliance::CutUpDonationCDResp * resp);

	//提供捐收
	int OfferHelp(unsigned uid, unsigned apply_uid, unsigned propsid);
	int OfferHelpUpdateUser(unsigned uid, unsigned aid, unsigned propsid, ProtoAlliance::OfferDonationResp * resp);
	int OfferHelpUpdateAlliance(unsigned uid, const string& name, unsigned aid, unsigned apply_uid, unsigned propsid, ProtoAlliance::OfferDonationResp * resp);

	//提取已捐收物品
	int FetchDonation(unsigned uid);
	int FetchDonationAllianceLocal(unsigned uid, unsigned aid, unsigned& propsid, ProtoAlliance::FetchDonationResp * resp);
	int AddDonation(unsigned uid, unsigned propsid, ProtoAlliance::FetchDonationResp * resp);

	//发布通知
	int SendNotify(unsigned uid, string & content);
	int SendNotifyLocal(unsigned uid, unsigned aid, const string& userName, const string & content, ProtoAlliance::SendNotifyResp * resp);

	//删除通知
	int DelNofity(unsigned uid, unsigned id);
	int DelNofityLocal(unsigned uid, unsigned aid, unsigned id, ProtoAlliance::DelNotifyResp* resp);

	//添加新成员
	int AddNewMember(DataAllianceMember& member, ProtoAlliance::AllianceMemberCPP * msg);
	//填充成员数据
	int FillMember(unsigned uid, unsigned aid, unsigned pos, DBCUserBaseWrap& userwrap, DataAllianceMember& member);

	//通知用户职位的变化
	int NotifyPositionChange(unsigned uid, unsigned oldpos, unsigned newpos, unsigned alliance_id);

	//删除公会
	int DropAlliance(unsigned uid, unsigned alliance_id);

	//删除成员在商会内的任意数据
	int ClearMemberData(unsigned alliance_id, unsigned member_uid);

	//校验成员是否在指定公会
	int CheckMember(unsigned uid, unsigned alliance_id, unsigned member_uid);
	bool CheckMember(unsigned alliance_id, unsigned member_uid);
	void FillAlliance(uint32_t aid, ProtoAlliance::AllianceCPP* resp);
	void FixSelf(unsigned uid);

	//权限校验
	int CheckPrivilege(unsigned uid, unsigned privilege);
	int CheckMemberPrivilege(unsigned uid, unsigned alliance_id, unsigned privilege);

	int PositionChange(DataAllianceMember &member, unsigned newposition);

	int SetMemberMessage(DataAllianceMember &member, ProtoAlliance::AllianceMemberCPP * msg);
	int SetMemberMessage(DataAllianceMember &member, ProtoAlliance::ReplyAllianceRaceOrder * msg);

	//判断公会名称是否可用
	bool IsNameAvailable(unsigned uid, string & name, string & reason);
	//是否参与竞赛
	bool IsInRace(unsigned joinTs);
	int SensitiveFilter(string & str);

	bool IsMemberFull(unsigned uid, unsigned alliance_id);
	bool CheckMemberFull(unsigned uid, unsigned alliance_id);

	//设置对应职位的成员信息
	int SetMemberByPostion(unsigned allianceid, unsigned uid, unsigned position,
			unsigned helptimes, unsigned level, const string& name,
			unsigned online, unsigned needHelp, const string& fig,
			DataAllianceMember & member);

	int GetAuthorityByPosition(unsigned position);

	void SetUserAllianceMsg(unsigned old, unsigned alliance_id, DataCommon::CommonItemsCPP * msg);

	// 跨服设置商会成员数据
	int UpdateMemberLocal(unsigned uid, unsigned aid, unsigned onlineTs, unsigned helpTs, unsigned level, const string& name, unsigned vipLevel);
	// 设置商会竞赛标志
	int SetAllianceRaceFlag(uint32_t uid, uint32_t aid, uint32_t id, ProtoAlliance::ReplyAllianceRaceInfo* resp);
	// 商会竞赛信息
	int FillAllianceRaceInfo(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceInfo* resp);
	// 商会竞赛订单
	int FillAllianceRaceOrder(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceOrder* resp);
	// 操作商会订单
	int OperateAllianceRaceOrder(uint32_t uid, uint32_t aid, uint32_t slot, uint32_t operate, ProtoAlliance::ReplyAllianceRaceOperateOrder* resp);
	uint32_t GetRaceOrderCutCdCost(uint32_t uid, uint32_t cdTs);
	//删除商会竞赛订单
	int DelAllianceRaceMemberOrder(uint32_t uid, uint32_t aid, unsigned type,ProtoAlliance::ReplyAllianceRaceOrder* resp);
	int AddRaceOrderProgressLocal(uint32_t uid, uint32_t aid, uint32_t orderType, uint32_t count, uint32_t target);
	int SyncRaceOrderProgress(uint32_t uid, uint32_t aid);
	// 商会竞赛购买订单
	int BuyAllianceRaceOrder(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceBuyOrder* resp);
	bool isFlagSet(uint8_t flag, uint8_t id);
	void setFlag(uint8_t& flag, uint8_t id);
	bool isRaceOrderFinish(DataAllianceMember& member);
	uint8_t getRaceOrderMaxChance(uint32_t uid, uint8_t raceLevel, uint8_t flag, uint32_t vipLevel);
	int AddRaceMemberOrderLog(DataAllianceMember& member, uint16_t id, uint8_t status);
	int UpdateRaceMemberOrderLog(DataAllianceMember& member, uint16_t id, uint8_t status);
	//获取竞赛奖励
	int FillAllianceRaceReward(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceReward* resp);
	//竞赛获取等级奖励
	int TakeAllianceRaceGradeReward(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceTakeGradeReward* resp);
	//竞赛获取阶段奖励
	int TakeAllianceRaceStageReward(uint32_t uid, uint32_t aid,
			const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& id, ProtoAlliance::ReplyAllianceRaceTakeStageReward* resp);
	//竞赛刷新阶段奖励
	int RefreshAllianceRaceStageReward(uint32_t uid, uint32_t aid,
			const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& id, ProtoAlliance::ReplyAllianceRaceRefreshStageReward* resp);
	int RefreshMemberRaceStageReward(const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& id,
			uint32_t levelId, DataAllianceMember & member);
//	int RefreshAllMemberRaceStageReward(uint32_t aid);

	//填充竞赛成员日志
	int FillAllianceRaceMemberOrderLog(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRaceMemberOrderLog* resp);
	//填充竞赛个人日志
	int FillAllianceRacePersonOrderLog(uint32_t uid, uint32_t aid, ProtoAlliance::ReplyAllianceRacePersonOrderLog* resp);

	//竞赛结束时间
	uint32_t GetRaceOverTs();
	//重置竞赛玩家数据
	int ResetRaceMemberInfo(DataAllianceMember & member);
	//竞赛是否开始
	bool IsRaceOpen();

	//看广告增加商会积分
	int WatchAdPlusRacePointLocal(uint32_t uid, uint32_t aid, uint32_t addPoint);

	//同服邀请他人加入商会添加动态消息
	bool AddInviteDyInfo(uint32_t uid,uint32_t other_uid,uint32_t aid);

	//跨服邀请他人加入商会添加动态消息
	bool AddInviteDyInfoOverServer(uint32_t uid,uint32_t other_uid,uint32_t aid);


private:
	set<unsigned> m_set_watch_ad; //每日接受观看广告任务的玩家
};

#endif //LOGIC_ALLIANCE_MANAGER_H_
