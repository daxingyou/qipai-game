#include "StdAfx.h"
#include "TableFrameSink.h"

//////////////////////////////////////////////////////////////////////////

//静态变量
const WORD			CTableFrameSink::m_wPlayerCount=GAME_PLAYER;			//游戏人数
const enStartMode	CTableFrameSink::m_GameStartMode=enStartMode_FullReady;	//开始模式

//////////////////////////////////////////////////////////////////////////

//构造函数
CTableFrameSink::CTableFrameSink()
{
	//游戏变量

	m_wCurrentHousecarl = -1;
	m_wCurrentEnthronement = -1;
	m_IsKeep = false;
	m_IsByOneself = false;
	m_wCurrentUser=INVALID_CHAIR;
	m_EndOrder = 0;
	m_PassIndex = 0;
	//运行信息
	m_bTurnCardCount=0;
	m_wTurnWiner=INVALID_CHAIR;
	memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));

	//扑克信息
	memset(m_bCardCount,0,sizeof(m_bCardCount));
	memset(m_bOutCardCount,0,sizeof(m_bOutCardCount));
	memset(m_bHandCardData,0,sizeof(m_bHandCardData));
	memset(m_bGameEnd,0,sizeof(m_bGameEnd));

	//组件变量
	m_pITableFrame=NULL;
	m_pGameServiceOption=NULL;

	return;
}

//析构函数
CTableFrameSink::~CTableFrameSink(void)
{
}

//接口查询
void * __cdecl CTableFrameSink::QueryInterface(const IID & Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(ITableFrameSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(ITableFrameSink,Guid,dwQueryVer);
	return NULL;
}

//初始化
bool __cdecl CTableFrameSink::InitTableFrameSink(IUnknownEx * pIUnknownEx)
{
	//查询接口
	ASSERT(pIUnknownEx!=NULL);
	m_pITableFrame=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,ITableFrame);
	if (m_pITableFrame==NULL) return false;

	//获取参数
	m_pGameServiceOption=m_pITableFrame->GetGameServiceOption();
	ASSERT(m_pGameServiceOption!=NULL);

	return true;
}

//复位桌子
void __cdecl CTableFrameSink::RepositTableFrameSink()
{
	//游戏变量

	m_wCurrentUser=INVALID_CHAIR;

	//运行信息
	m_bTurnCardCount=0;
	m_wTurnWiner=INVALID_CHAIR;
	memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));

	//扑克信息
	
	memset(m_bCardCount,0,sizeof(m_bCardCount));
	memset(m_bOutCardCount,0,sizeof(m_bOutCardCount));
	memset(m_bHandCardData,0,sizeof(m_bHandCardData));

	return;
}

//开始模式
enStartMode __cdecl CTableFrameSink::GetGameStartMode()
{
	return m_GameStartMode;
}

//游戏状态
bool __cdecl CTableFrameSink::IsUserPlaying(WORD wChairID)
{
	return true;
}

//游戏开始
bool __cdecl CTableFrameSink::OnEventGameStart()
{
	//设置状态
	
	//混乱扑克
	BYTE bRandCard[165];
	m_GameLogic.RandCardList(bRandCard,sizeof(bRandCard)/sizeof(bRandCard[0]));

	//分发扑克
	for (WORD i=0;i<m_wPlayerCount;i++)
	{
		m_bCardCount[i]=PLAYER_CARDS;
		CopyMemory(&m_bHandCardData[i],&bRandCard[i*m_bCardCount[i]],sizeof(BYTE)*m_bCardCount[i]);
		m_GameLogic.SortCardList(m_bHandCardData[i],m_bCardCount[i]);
	}
	
	
	//设置用户
	/*m_wCurrentUser=m_wCurrentEnthronement=rand()%GAME_PLAYER;
	srand( (unsigned)time( NULL ) );	
	m_wCurrentHousecarl=rand()%GAME_PLAYER;;*/
	for (WORD i=0;i<m_wPlayerCount;i++)
	{
		for (WORD j=0;j<m_bCardCount[j];j++)
		{
			if (m_bHandCardData[i][j] == 0x44)
				m_wCurrentUser=m_wCurrentEnthronement = i;
			else if (m_bHandCardData[i][j] == 0x43)
				m_wCurrentHousecarl = i;

		}
	}
	//发送扑克
	CMD_S_SendCard SendCard;
	SendCard.wCurrentUser=m_wCurrentUser;
	
	for (WORD i=0;i<m_wPlayerCount;i++)
	{
		CopyMemory(SendCard.bCardData,m_bHandCardData[i],sizeof(SendCard.bCardData));
		
		SendCard.wCurrentEnthronement = m_wCurrentEnthronement;	
		SendCard.wCurrentHousecarl = m_wCurrentHousecarl;
		
		m_pITableFrame->SendTableData(i,SUB_S_SEND_CARD,&SendCard,sizeof(SendCard));
		m_pITableFrame->SendLookonData(i,SUB_S_SEND_CARD,&SendCard,sizeof(SendCard));
	}
	//设置状态
	m_pITableFrame->SetGameStatus(GS_WK_PLAYING);
	return true;
}

//游戏结束
bool __cdecl CTableFrameSink::OnEventGameEnd(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason)
{
	switch (cbReason)
	{
	case GER_NORMAL:		//常规结束
		{
			//定义变量
			CMD_S_GameEnd GameEnd;
			memset(&GameEnd,0,sizeof(GameEnd));
			tagScoreInfo ScoreInfo[m_wPlayerCount];
			memset(&ScoreInfo,0,sizeof(ScoreInfo));
			LONG lTax=0;

			//剩余扑克
			BYTE bCardPos=0;
			for (WORD i=0;i<m_wPlayerCount;i++)
			{
				GameEnd.bCardCount[i]=m_bCardCount[i];
				CopyMemory(&GameEnd.bCardData[bCardPos],m_bHandCardData[i],m_bCardCount[i]*sizeof(BYTE));
				bCardPos+=m_bCardCount[i];
			}

			//变量定义
			LONG lCellScore=m_pGameServiceOption->lCellScore;
			//统计积分
			BYTE c =false;
			for (WORD i=0;i<m_wPlayerCount;i++)
			{
				 if (i == m_wCurrentHousecarl-1 && i == m_wCurrentEnthronement-1)
				 {
					if (m_bGameEnd[i] == 1)
						GameEnd.lGameScore[i] = 12;
					else
						GameEnd.lGameScore[i] = -12;					
				 }
				else if (i==m_wCurrentEnthronement-1) 
				{
					GameEnd.lGameScore[i] = (6 - (m_bGameEnd[m_wCurrentEnthronement] + m_bGameEnd[m_wCurrentHousecarl])) * 2;
				}
				else if (i == m_wCurrentHousecarl-1)
				{
					GameEnd.lGameScore[i] = (6 - (m_bGameEnd[m_wCurrentEnthronement] + m_bGameEnd[m_wCurrentHousecarl]));
				}
				else 
				{
					if (m_bGameEnd[i] == 1)
					{
						GameEnd.lGameScore[i] = 3;
						c = true;
					}
					else
					{
						GameEnd.lGameScore[i] = -3;
					}
				}
			}
			bool b = false;
			if (m_bGameEnd[m_wCurrentEnthronement-1] == 2 || m_bGameEnd[m_wCurrentHousecarl-1] == 2)
				b =true;
			if (c)
			{
				for (WORD i=0;i<m_wPlayerCount;i++)
				{
					if (b)
						GameEnd.lGameScore[i] = 0;
					else if ((GameEnd.lGameScore[i] == -3) && (i!=m_wCurrentEnthronement-1) && (i!=m_wCurrentHousecarl-1) && !b)
						GameEnd.lGameScore[i] = 3;
				}
			}

			//计算税收
			if (m_pGameServiceOption->wServerType&GAME_GENRE_GOLD)
			{
				for (WORD i=0;i<m_wPlayerCount;i++)
				{
					if (GameEnd.lGameScore[i]>=100L)
					{
						lTax=GameEnd.lGameScore[i]*m_pGameServiceOption->wRevenue/100L;
						GameEnd.lGameTax += lTax;
						GameEnd.lGameScore[i] -= lTax;
					}
				}
			}

			//发送信息
			m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));
			m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));

			//修改积分

			for (WORD i=0;i<m_wPlayerCount;i++)
			{
				ScoreInfo[i].lScore=GameEnd.lGameScore[i];
				ScoreInfo[i].ScoreKind=(GameEnd.lGameScore[i]>0L)?enScoreKind_Win:enScoreKind_Lost;
				m_pITableFrame->WriteUserScore(i,ScoreInfo[i].lScore,ScoreInfo[i].lRevenue,ScoreInfo[i].ScoreKind);
			}
			//m_pITableFrame->WriteTableScore(ScoreInfo,m_wPlayerCount,GameEnd.lGameTax);

			//切换用户
			m_wCurrentUser=wChairID;

			//结束游戏
			m_pITableFrame->ConcludeGame();

			return true;
		}
	case GER_USER_LEFT:		//用户强退
		{
			//效验参数
			ASSERT(pIServerUserItem!=NULL);
			ASSERT(wChairID<m_wPlayerCount);

			//构造数据
			LONG lCellScore=m_pGameServiceOption->lCellScore;
			CMD_S_GameEnd GameEnd;
			memset(&GameEnd,0,sizeof(GameEnd));
			LONG lTax=0;
			
			//皇帝强退
			GameEnd.lGameScore[wChairID] = -50;
			for (WORD i=0;i<m_wPlayerCount;i++)
			{
				if(i==wChairID)
				{
					GameEnd.lGameScore[i] = -50;
				}
				else
					GameEnd.lGameScore[i]=0;
			}

			//计算税收
			if (m_pGameServiceOption->wServerType&GAME_GENRE_GOLD)
			{
				for (WORD i=0;i<m_wPlayerCount;i++)
				{
					if (GameEnd.lGameScore[i]>=100L)
					{
						lTax=GameEnd.lGameScore[i] * m_pGameServiceOption->wRevenue/100L;
						GameEnd.lGameTax+=lTax;
						GameEnd.lGameScore[i]-=lTax;
					}
				}
			}

			//发送信息
			m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));
			m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));
			////剩余扑克
			//BYTE bCardPos=0;
			//for (WORD i=0;i<m_wPlayerCount;i++)
			//{
			//	GameEnd.bCardCount[i]=m_bCardCount[i];
			//	CopyMemory(&GameEnd.bCardData[bCardPos],m_bHandCardData[i],m_bCardCount[i]*sizeof(BYTE));
			//	bCardPos+=m_bCardCount[i];
			//}

			////发送信息
			//m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));
			//m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_GAME_END,&GameEnd,sizeof(GameEnd));

			//修改积分
			tagScoreInfo ScoreInfo;
			ZeroMemory(&ScoreInfo,sizeof(ScoreInfo));
			ScoreInfo.ScoreKind=enScoreKind_Draw;
			ScoreInfo.lScore=GameEnd.lGameScore[wChairID];
			m_pITableFrame->WriteUserScore(wChairID,ScoreInfo.lScore,lTax,ScoreInfo.ScoreKind);

			//结束游戏
			m_pITableFrame->ConcludeGame();

			return true;
		}
	}
	m_EndOrder = 0;
	m_PassIndex = 0;
	ASSERT(FALSE);

	return false;
}

//发送场景
bool __cdecl CTableFrameSink::SendGameScene(WORD wChiarID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret)
{
	switch (cbGameStatus)
	{
	case GS_WK_FREE:		//空闲状态
		{
			//构造数据
			CMD_S_StatusFree StatusFree;
			memset(&StatusFree,0,sizeof(StatusFree));

			//设置变量
			StatusFree.lBaseScore=m_pGameServiceOption->lCellScore;

			//发送场景
			return m_pITableFrame->SendGameScene(pIServerUserItem,&StatusFree,sizeof(StatusFree));
		}
	
	case GS_WK_PLAYING:		//游戏状态
		{
			//构造数据
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay,0,sizeof(StatusPlay));

			//设置变量
			StatusPlay.wCurrentUser=m_wCurrentUser;
			StatusPlay.wLastOutUser=m_wTurnWiner;
			StatusPlay.bTurnCardCount=m_bTurnCardCount;
			StatusPlay.lBaseScore=m_pGameServiceOption->lCellScore;
		
			CopyMemory(StatusPlay.bTurnCardData,m_bTurnCardData,sizeof(BYTE)*m_bTurnCardCount);
			CopyMemory(StatusPlay.bCardData,m_bHandCardData[wChiarID],sizeof(BYTE)*m_bCardCount[wChiarID]);

			//发送场景
			return m_pITableFrame->SendGameScene(pIServerUserItem,&StatusPlay,sizeof(StatusPlay));
		}
	}

	return false;
}

//定时器事件
bool __cdecl CTableFrameSink::OnTimerMessage(WORD wTimerID, WPARAM wBindParam)
{
	return false;
}

//游戏消息处理
bool __cdecl CTableFrameSink::OnGameMessage(WORD wSubCmdID, const void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	switch (wSubCmdID)
	{
	case SUB_C_CHAIRID:
		{
			if (wDataSize != sizeof(CMD_C_ChairID)) return false;
			CMD_C_ChairID *pChairID = (CMD_C_ChairID *)pDataBuffer;
			if (pChairID->IsChair)
			{
				m_IsByOneself = pChairID->IsGet;
			}
			else
			{
				m_IsKeep = pChairID->IsGet;
			}
			break;
		}
	case SUB_C_OUT_CART:		//用户出牌
		{
			//变量定义
			CMD_C_OutCard * pOutCard=(CMD_C_OutCard *)pDataBuffer;
			WORD wHeadSize=sizeof(CMD_C_OutCard)-sizeof(pOutCard->bCardData);

			//效验数据
			ASSERT(wDataSize>=wHeadSize);
			if (wDataSize<wHeadSize) return false;
			ASSERT(wDataSize==(wHeadSize+pOutCard->bCardCount*sizeof(pOutCard->bCardData[0])));
			if (wDataSize!=(wHeadSize+pOutCard->bCardCount*sizeof(pOutCard->bCardData[0]))) return false;

			//用户效验
			tagServerUserData * pUserData=pIServerUserItem->GetUserData();
			if (pUserData->cbUserStatus!=US_PLAY) return true;

			//消息处理
			return OnUserOutCard(pUserData->wChairID,pOutCard->bCardData,pOutCard->bCardCount);
		}
	case SUB_C_PASS_CARD:
		{
			//用户效验
			tagServerUserData * pUserData=pIServerUserItem->GetUserData();
			if (pUserData->cbUserStatus!=US_PLAY) return true;

			//消息处理
			return OnUserPassCard(pUserData->wChairID);
		}
	}

	return true;
}

//框架消息处理
bool __cdecl CTableFrameSink::OnFrameMessage(WORD wSubCmdID, const void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	return false;
}
//用户出牌
bool CTableFrameSink::OnUserOutCard(WORD wChairID, BYTE bCardData[], BYTE bCardCount)
{
	//效验状态
	if (m_pITableFrame->GetGameStatus()!=GS_WK_PLAYING) return true;
	if (wChairID!=m_wCurrentUser) return false;

	//类型判断
	BYTE bCardType=m_GameLogic.GetCardType(bCardData,bCardCount);
	if (bCardType==CT_INVALID) return false;

	//更随出牌
	if (m_bTurnCardCount==0)
		m_bTurnCardCount=bCardCount;
	else if 
		(m_GameLogic.CompareCard(bCardData,m_bTurnCardData,bCardCount,m_bTurnCardCount)==false) 
		return false;

	//删除扑克
	if (m_GameLogic.RemoveCard(bCardData,bCardCount,m_bHandCardData[wChairID],m_bCardCount[wChairID])==false) 
		return false;
	m_bCardCount[wChairID]-=bCardCount;

	//出牌记录
	m_bTurnCardCount=bCardCount;
	m_bOutCardCount[wChairID]++;
	CopyMemory(m_bTurnCardData,bCardData,sizeof(BYTE)*bCardCount);
	
	//切换用户
	m_wTurnWiner=wChairID;
	if (m_bGameEnd[wChairID]<m_wPlayerCount-1)
	{
		if (bCardType!=CT_HAVE_D)
		{
			 BYTE i = (m_wCurrentUser+1)%m_wPlayerCount;
			while(1)
			{
				if (i < GAME_PLAYER && m_bCardCount[i]>0)
				{
					m_wCurrentUser=i;
					break;
				}
				else
					i = (i++)%m_wPlayerCount;
			}
		}
	}
	else 
		m_wCurrentUser=INVALID_CHAIR;

	m_PassIndex = 0;
	//构造数据
	CMD_S_OutCard OutCard;
	OutCard.bCardCount=bCardCount;
	OutCard.wOutCardUser=wChairID;
	OutCard.wCurrentUser=m_wCurrentUser;
	CopyMemory(OutCard.bCardData,m_bTurnCardData,m_bTurnCardCount*sizeof(BYTE));

	//发送数据
	WORD wSendSize=sizeof(OutCard)-sizeof(OutCard.bCardData)+OutCard.bCardCount*sizeof(BYTE);
	m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_OUT_CARD,&OutCard,wSendSize);
	m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_OUT_CARD,&OutCard,wSendSize);

	//出牌最大
	if (bCardType==CT_HAVE_D) m_bTurnCardCount=0;

	//结束判断
	//if (m_wCurrentUser==INVALID_CHAIR) OnEventGameEnd(wChairID,NULL,GER_NORMAL);
	
	if (m_bCardCount[wChairID] <= 0)
	{
		m_EndOrder++;
		m_bGameEnd[wChairID] = m_EndOrder;
		if (m_EndOrder >= m_wPlayerCount-1 || m_wCurrentUser == INVALID_CHAIR)
		{
			m_EndOrder = 0;
			OnEventGameEnd(wChairID,NULL,GER_NORMAL);
		}
	}
	
	return true;
}

//用户放弃
bool CTableFrameSink::OnUserPassCard(WORD wChairID)
{
	//效验状态
	if (m_pITableFrame->GetGameStatus()!=GS_WK_PLAYING) return true;
	if ((wChairID!=m_wCurrentUser)||(m_bTurnCardCount==0)) return false;

	//设置变量
	BYTE i = (m_wCurrentUser+1)%m_wPlayerCount;
	while(1)
	{
		if (i < GAME_PLAYER && m_bCardCount[i]>0)
		{
			m_wCurrentUser=i;
			break;
		}
		else
			i = (i++)%m_wPlayerCount;
	}
	if ( m_EndOrder > 0)
	{
		m_PassIndex++;
		if (m_wPlayerCount-m_EndOrder <= m_PassIndex)
		{
			BYTE i = (wChairID+1)%m_wPlayerCount;
			if (i < GAME_PLAYER && m_bCardCount[i]>0)
			{
				m_wCurrentUser=i;
				m_bTurnCardCount=0;
			}
			else
			{
				i = (i++)%m_wPlayerCount;
				if (i< GAME_PLAYER && m_bCardCount[i]>0)
				{
					m_wCurrentUser=i;
					m_bTurnCardCount=0;
				}
				else
				{
					i = (i++)%m_wPlayerCount;
					if (i< GAME_PLAYER && m_bCardCount[i]>0)
					{
						m_wCurrentUser=i;
						m_bTurnCardCount=0;
					}
					else
					{
						i = (i++)%m_wPlayerCount;
						if (i< GAME_PLAYER && m_bCardCount[i]>0)
						{
							m_wCurrentUser=i;
							m_bTurnCardCount=0;
						}
					}
				}
			}
		}
	}
	
	if (m_wCurrentUser==m_wTurnWiner) m_bTurnCardCount=0;
	//if (m_bCardCount[wChairID]==0) m_bTurnCardCount=0;

	//发送数据
	CMD_S_PassCard PassCard;
	PassCard.wPassUser=wChairID;
	PassCard.wCurrentUser=m_wCurrentUser;
	PassCard.bNewTurn=(m_bTurnCardCount==0)?TRUE:FALSE;
	m_pITableFrame->SendTableData(INVALID_CHAIR,SUB_S_PASS_CARD,&PassCard,sizeof(PassCard));
	m_pITableFrame->SendLookonData(INVALID_CHAIR,SUB_S_PASS_CARD,&PassCard,sizeof(PassCard));

	return true;
}

//////////////////////////////////////////////////////////////////////////
