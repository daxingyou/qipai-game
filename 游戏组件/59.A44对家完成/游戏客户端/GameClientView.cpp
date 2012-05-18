#include "StdAfx.h"
#include "Resource.h"
#include "GameClientView.h"

//////////////////////////////////////////////////////////////////////////
//按钮标识 

#define IDC_START						100								//开始按钮
#define IDC_ONE_SCORE					101								//1 分按钮
#define IDC_TWO_SCORE					102								//2 分按钮
#define IDC_THREE_SCORE					103								//3 分按钮
#define IDC_GIVE_UP_SCORE				104								//放弃按钮
#define IDC_OUT_CARD					105								//出牌按钮
#define IDC_PASS_CARD					106								//PASS按钮
#define IDC_AUTO_OUTCARD                107                             //提示按纽
#define IDC_AUTOPLAY_ON					108								//托管按钮
#define IDC_AUTOPLAY_OFF				109								//取消按钮
#define IDC_SORT_CARD			        110                             //托管按纽

///////////////////////////////////////////////////////////////////////////////
//添加立棍、撅棍按钮标识
#define IDC_LI_GUN                      111
#define IDC_JUE_GUN                     112
#define IDC_AGREE                       113
#define IDC_DISAGREE                    114
#define IDC_NOT_LI_GUN                  115
#define IDC_NOT_JUE_GUN                 116
///////////////////////////////////////////////////////////////////////////////

//爆炸数目
#define BOMB_EFFECT_COUNT				6								//爆炸数目

//爆炸标识
#define IDI_BOMB_EFFECT					101								//爆炸标识
//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameClientView, CGameFrameView)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_OUT_CARD, OnOutCard)
	ON_BN_CLICKED(IDC_PASS_CARD, OnPassCard)
	ON_BN_CLICKED(IDC_ONE_SCORE, OnOneScore)
	ON_BN_CLICKED(IDC_TWO_SCORE, OnTwoScore)
	ON_BN_CLICKED(IDC_THREE_SCORE, OnThreeScore)
	ON_BN_CLICKED(IDC_AUTO_OUTCARD, OnAutoOutCard)
	ON_BN_CLICKED(IDC_GIVE_UP_SCORE, OnGiveUpScore)
	ON_BN_CLICKED(IDC_AUTOPLAY_ON, OnAutoPlayerOn)
	ON_BN_CLICKED(IDC_AUTOPLAY_OFF,OnAutoPlayerOff)
	ON_BN_CLICKED(IDC_SORT_CARD, OnBnClickedSortCard)
	//////////////////////////////////添加消息映射//////////////////////////////////
	ON_BN_CLICKED(IDC_LI_GUN, OnHitLiGun)
	ON_BN_CLICKED(IDC_JUE_GUN, OnHitJueGun)
	ON_BN_CLICKED(IDC_AGREE, OnHitAgree)
	ON_BN_CLICKED(IDC_DISAGREE, OnHitDisagree)
	ON_BN_CLICKED(IDC_NOT_LI_GUN, OnHitNotLiGun)
	ON_BN_CLICKED(IDC_NOT_JUE_GUN, OnHitNotJueGun)
	////////////////////////////////////////////////////////////////////////////////
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

//构造函数
CGameClientView::CGameClientView(void) : CGameFrameView(true,24)
{
	//游戏变量
	m_lBaseScore=0;
	m_wBombTime=1;
	m_cbLandScore=0;
	m_wLandUser=INVALID_CHAIR;

	//状态变量
	m_bLandTitle=false;
	memset(m_bPass,0,sizeof(m_bPass));
	memset(m_bScore,0,sizeof(m_bScore));
	memset(m_bCardCount,0,sizeof(m_bCardCount));
	memset(m_bUserTrustee, 0, sizeof( m_bUserTrustee ) );

	//爆炸动画
	m_bBombEffect=false;
	m_cbBombFrameIndex=0;

	//位置信息
	memset(m_ptScore,0,sizeof(m_ptScore));
	memset(m_ptLand,0,sizeof(m_ptLand));
	m_bDeasilOrder = false;

	//加载资源
	HINSTANCE hInstance=AfxGetInstanceHandle();
	m_ImageBomb.SetLoadInfo(IDB_BOMB,hInstance);
	m_ImageBack.SetLoadInfo(IDB_VIEW_BACK,hInstance);
	m_ImageCenter.SetLoadInfo(IDB_VIEW_CENTER,hInstance);
	m_ImageBombEffect.SetLoadInfo(IDB_BOMB_EFFECT,hInstance);
	m_ImageScore.LoadImage(hInstance, TEXT( "SCORE" ) );
	m_ImageLand.LoadImage(hInstance, TEXT( "BANKER" ) );
	m_ImageUserTrustee.LoadImage(hInstance,  TEXT("USER_TRUSTEE"));
	m_Imagetime.SetLoadInfo(IDB_time,hInstance);/////////////////////时间
	m_Imagetimech.SetLoadInfo(IDB_time1,hInstance);/////////////////////时间
	//计算位置
	m_LandSize.cx=m_ImageLand.GetWidth();
	m_LandSize.cy=m_ImageLand.GetHeight();

	return;
}

//析构函数
CGameClientView::~CGameClientView(void)
{
}

//建立消息
int CGameClientView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct)==-1) return -1;

	//创建控件
	CRect CreateRect(0,0,0,0);

	//创建扑克
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		//用户扑克
		m_UserCardControl[i].SetDirection(true);
		m_UserCardControl[i].SetDisplayFlag(true);	
		m_UserCardControl[i].SetCardSpace( 16, 20, 0 );

		m_UserCardControl[i].Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CreateRect,this,50+i);

		if ( i == 0 || i == ME_VIEW_CHAIR )
			m_UserCardControl[i].SetDirection(true);
		else
			m_UserCardControl[i].SetDirection(false);

		if ( i ==ME_VIEW_CHAIR || i == 0 )
			m_HandCardControl[i].SetDirection(true);
		else
			m_HandCardControl[i].SetDirection(false);

		//用户扑克
		if (i!=ME_VIEW_CHAIR)
		{
			m_HandCardControl[i].SetCardSpace(8,8,0);
			m_HandCardControl[i].SetDisplayFlag(false);
			m_HandCardControl[i].Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CreateRect,this,30+i);
		}
		else
		{
			m_HandCardControl[i].SetDisplayFlag(false);
			m_HandCardControl[i].Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CreateRect,this,30+i);
		}
	}

	//设置扑克
	//m_BackCardControl.SetCardSpace(14,0,0);
	//m_BackCardControl.SetDisplayFlag(false);
	m_HandCardControl[ME_VIEW_CHAIR].SetSinkWindow(AfxGetMainWnd());
	//m_BackCardControl.Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CreateRect,this,41);

	//创建按钮
	m_btStart.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_START);
	m_btOutCard.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_OUT_CARD);
	m_btPassCard.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_PASS_CARD);
	m_btOneScore.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_ONE_SCORE);
	m_btTwoScore.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_TWO_SCORE);
	m_btGiveUpScore.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_GIVE_UP_SCORE);
	m_btAutoOutCard.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_AUTO_OUTCARD);
	m_btThreeScore.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_THREE_SCORE);
	m_btAutoPlayOn.Create(TEXT(""), WS_CHILD,CreateRect,this,IDC_AUTOPLAY_ON);
	m_btAutoPlayOff.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_AUTOPLAY_OFF);
	m_btSortCard.Create(NULL,WS_CHILD|WS_DISABLED|WS_VISIBLE,CreateRect,this,IDC_SORT_CARD);

	////////////////////////////////////////添加创建按钮////////////////////////////////////
	m_btLiGun.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_LI_GUN);
	m_btJueGun.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_JUE_GUN);
	m_btAgree.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_AGREE);
	m_btDisagree.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_DISAGREE);
	m_btNotLiGun.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_NOT_LI_GUN);
	m_btNotJueGun.Create(TEXT(""),WS_CHILD,CreateRect,this,IDC_NOT_JUE_GUN);
	////////////////////////////////////////////////////////////////////////////////////////
    

	//设置按钮
	HINSTANCE hInstance=AfxGetInstanceHandle();
	m_btStart.SetButtonImage(IDB_START,hInstance,false);
	m_btOutCard.SetButtonImage(IDB_OUT_CARD,hInstance,false);
	m_btPassCard.SetButtonImage(IDB_PASS,hInstance,false);
	m_btOneScore.SetButtonImage(IDB_ONE_SCORE,hInstance,false);
	m_btTwoScore.SetButtonImage(IDB_TWO_SCORE,hInstance,false);
	m_btGiveUpScore.SetButtonImage(IDB_GIVE_UP,hInstance,false);
	m_btAutoOutCard.SetButtonImage(IDB_AUTO_OUT_CARD,hInstance,false);
	m_btThreeScore.SetButtonImage(IDB_THREE_SCORE,hInstance,false);
	m_btAutoPlayOn.SetButtonImage(IDB_AUTOPLAY_ON,hInstance,false);
	m_btAutoPlayOff.SetButtonImage(IDB_AUTOPLAY_OFF,hInstance,false);
	m_btSortCard.SetButtonImage(IDB_COUNT_SORT,hInstance,false);
	//////////////////////////////////////////加载按钮图片//////////////////////////////////////
	m_btLiGun.SetButtonImage(IDB_LI_GUN,hInstance,false);
	m_btJueGun.SetButtonImage(IDB_JUE_GUN,hInstance,false);
	m_btAgree.SetButtonImage(IDB_AGREE,hInstance,false);
	m_btDisagree.SetButtonImage(IDB_DISAGREE,hInstance,false);
	m_btNotLiGun.SetButtonImage(IDB_NOT_LI_GUN,hInstance,false);
	m_btNotJueGun.SetButtonImage(IDB_NOT_JUE_GUN,hInstance,false);
	////////////////////////////////////////////////////////////////////////////////////////////

    m_btAutoPlayOn.ShowWindow(SW_SHOW);
    m_btAutoPlayOff.ShowWindow(SW_HIDE);

#ifdef VIDEO_GAME

	//创建视频
	for (WORD i=0; i<GAME_PLAYER; i++)
	{
		//创建视频
		m_DlgVedioService[i].Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CreateRect,this,200+i);
		m_DlgVedioService[i].InitVideoService(i==ME_VIEW_CHAIR,i==ME_VIEW_CHAIR);

		//设置视频
		m_VedioServiceManager.SetVideoServiceControl(i,&m_DlgVedioService[i]);
	}

	for ( WORD wChairID = 0; wChairID < GAME_PLAYER; ++wChairID )
	{
		if ( wChairID != ME_VIEW_CHAIR )
			m_HandCardControl[ wChairID ].ShowWindow( SW_HIDE );
	}

#endif

	return 0;
}

//鼠标消息
void CGameClientView::OnRButtonUp(UINT nFlags, CPoint Point)
{
	__super::OnRButtonUp(nFlags, Point);

	//收起扑克
		//设置扑克
	m_btOutCard.EnableWindow(FALSE);
	m_HandCardControl[ME_VIEW_CHAIR].ShootAllCard(false);

	return;
}

//鼠标双击
void CGameClientView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	__super::OnLButtonDblClk(nFlags, point);

	//设置扑克
	m_btOutCard.EnableWindow(FALSE);
	m_HandCardControl[ME_VIEW_CHAIR].ShootAllCard(false);

	return;
}

//重置界面
void CGameClientView::ResetGameView()
{
	//游戏变量
	m_lBaseScore=0;
	m_wBombTime=1;
	m_cbLandScore=0;
	m_wLandUser=INVALID_CHAIR;

	//爆炸动画
	m_bBombEffect=false;
	m_cbBombFrameIndex=0;

	//状态变量
	m_bLandTitle=false;
	memset(m_bPass,0,sizeof(m_bPass));
	memset(m_bScore,0,sizeof(m_bScore));
	memset(m_bCardCount,0,sizeof(m_bCardCount));
	memset(m_bUserTrustee, 0, sizeof( m_bUserTrustee ) );
    
	//隐藏控件
	if(m_ScoreView.m_hWnd) m_ScoreView.ShowWindow(SW_HIDE);

	//隐藏按钮
	m_btStart.ShowWindow(SW_HIDE);
	m_btOutCard.ShowWindow(SW_HIDE);
	m_btPassCard.ShowWindow(SW_HIDE);
	m_btGiveUpScore.ShowWindow(SW_HIDE);
	m_btAutoOutCard.ShowWindow(SW_HIDE);
	m_btThreeScore.ShowWindow(SW_HIDE);
	m_btAutoPlayOn.ShowWindow(SW_SHOW);
	m_btAutoPlayOff.ShowWindow(SW_HIDE);

	m_btLiGun.ShowWindow(SW_HIDE);
	m_btJueGun.ShowWindow(SW_HIDE);
	m_btNotLiGun.ShowWindow(SW_HIDE);
	m_btNotJueGun.ShowWindow(SW_HIDE);
	m_btAgree.ShowWindow(SW_HIDE);
	m_btDisagree.ShowWindow(SW_HIDE);

	//禁用控件
	m_btOutCard.EnableWindow(FALSE);
	m_btPassCard.EnableWindow(FALSE);
	m_btAutoOutCard.EnableWindow(FALSE);
	m_btSortCard.EnableWindow(FALSE);
	m_btSortCard.SetButtonImage(IDB_COUNT_SORT,AfxGetInstanceHandle(),false);

	m_btLiGun.EnableWindow(FALSE);
	m_btNotLiGun.EnableWindow(FALSE);
	m_btJueGun.EnableWindow(FALSE);
	m_btNotJueGun.EnableWindow(FALSE);
	m_btAgree.EnableWindow(FALSE);
	m_btDisagree.EnableWindow(FALSE);

	//扑克控件
	//m_BackCardControl.SetCardData(NULL,0);
	m_HandCardControl[ME_VIEW_CHAIR].SetPositively(false);
	m_HandCardControl[ME_VIEW_CHAIR].SetDisplayFlag(false);

	//设置扑克
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_UserCardControl[i].SetCardData(NULL,0);
		m_HandCardControl[i].SetCardData(NULL,0);
		if (i!=ME_VIEW_CHAIR)
			m_HandCardControl[i].SetCardSpace(8,8,0);
	}

#ifdef VIDEO_GAME
	//隐藏扑克
	m_HandCardControl[ 0 ].ShowWindow( SW_HIDE );
	m_HandCardControl[ 1 ].ShowWindow( SW_HIDE );
	m_HandCardControl[ 3 ].ShowWindow( SW_HIDE );
#endif

	return;
}

//调整控件
void CGameClientView::RectifyGameView(int nWidth, int nHeight)
{

	int nVDPosY = 0;

#ifdef VIDEO_GAME
	nVDPosY = 50;
#endif

	//设置坐标
	m_ptFace[0].x=nWidth/2-m_nXFace/2;
	m_ptFace[0].y=m_nYBorder+5;
	m_ptName[0].x=nWidth/2+5+m_nXFace/2;
	m_ptName[0].y=m_nYBorder+2;
	m_ptTimer[0].x=nWidth/2;//nWidth/2-m_nXFace/2-m_nXTimer/2-5;
	m_ptTimer[0].y=nHeight/2;//m_nYBorder+m_nYTimer/2+2;

	m_ptReady[0].x=nWidth/2+5+m_nXFace/2+120;
	m_ptReady[0].y=m_nYBorder+25;
	m_ptScore[0].x=nWidth/2-21;
	m_ptScore[0].y=m_nYBorder+m_nYFace+130;
	m_ptLand[0].x=nWidth/2-m_nXFace/2-m_LandSize.cx-m_nXTimer*2;
	m_ptLand[0].y=m_nXBorder+5;

	//设置坐标
	m_ptFace[1].x=m_nXBorder+5;
	m_ptFace[1].y=nHeight/2-m_nYFace-nVDPosY;
	m_ptName[1].x=m_nXBorder+5;
	m_ptName[1].y=nHeight/2+5-nVDPosY;
	m_ptTimer[1].x=nWidth/2;//m_nXBorder+2+m_nXTimer/2;
	m_ptTimer[1].y=nHeight/2;//nHeight/2-m_nYFace-5-m_nYTimer/2-nVDPosY;
	m_ptReady[1].x=m_nXBorder+5+m_nXTimer;
	m_ptReady[1].y=nHeight/2-m_nYFace-m_nYTimer/2-5-nVDPosY;
	m_ptScore[1].x=m_nXBorder+m_nXFace+147-10;
	m_ptScore[1].y=nHeight/2-m_nYFace-30-nVDPosY;
	m_ptLand[1].x=m_nXBorder+5;
	m_ptLand[1].y=nHeight/2+40-nVDPosY;

	//设置坐标
	m_ptFace[3].x=nWidth-m_nXBorder-m_nXFace-5;
	m_ptFace[3].y=nHeight/2-m_nYFace-nVDPosY;
	m_ptName[3].x=nWidth-m_nXBorder-5;
	m_ptName[3].y=nHeight/2+5-nVDPosY;
	m_ptTimer[3].x=nWidth/2;//nWidth-m_nXBorder-m_nXTimer/2-2;
	m_ptTimer[3].y=nHeight/2;//nHeight/2-m_nYFace-m_nYTimer/2-5-nVDPosY;
	m_ptReady[3].x=nWidth-m_nXBorder-m_nXTimer-5;
	m_ptReady[3].y=nHeight/2-m_nYFace-m_nYTimer/2-5-nVDPosY;
	m_ptScore[3].x=nWidth-m_nXBorder-m_nXFace-190-7;
	m_ptScore[3].y=nHeight/2-m_nYFace-30-nVDPosY;
	m_ptLand[3].x=nWidth-m_nXBorder-m_LandSize.cx-5;
	m_ptLand[3].y=nHeight/2+40-nVDPosY;

	//用户扑克
	m_UserCardControl[3].SetBenchmarkPos(nWidth-m_nXFace-m_nXBorder-147,nHeight/2-40,enXRight,enYCenter);
	m_UserCardControl[1].SetBenchmarkPos(m_nXBorder+m_nXFace+147,nHeight/2-40,enXLeft,enYCenter);
	m_UserCardControl[0].SetBenchmarkPos(nWidth/2,160,enXCenter,enYTop);
	m_HandCardControl[3].SetBenchmarkPos(nWidth-m_nXBorder-m_nXFace-50,nHeight/2,enXRight,enYCenter);
	m_HandCardControl[1].SetBenchmarkPos(m_nXBorder+m_nXFace+50,nHeight/2,enXLeft,enYCenter);
	m_HandCardControl[0].SetBenchmarkPos(nWidth/2+40,100,enXCenter,enYCenter);	

	////////////////////////////////////////////////////////////////////////////
//	BYTE cbCardData[ ] = { 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01 };
//	m_BackCardControl.SetCardData( cbCardData, 8 );
//	for ( WORD wChairID = 0; wChairID < GAME_PLAYER; ++wChairID ) {
//	//	if ( wChairID == ME_VIEW_CHAIR )
//		m_HandCardControl[ wChairID ].SetCardData( cbCardData, 25 );
//		m_HandCardControl[ wChairID ].SetDisplayFlag( true );
//		m_UserCardControl[ wChairID ].SetCardData( cbCardData, 10 ); 
//		m_bPass[ wChairID ] = true; 
//	}
//	
//	m_BackCardControl.SetDisplayFlag( true );
//	m_HandCardControl[ ME_VIEW_CHAIR ].SetDisplayFlag( true );
//	if ( m_ScoreView.m_hWnd == NULL ) m_ScoreView.Create(IDD_GAME_SCORE,this);
////	m_ScoreView.ShowWindow( SW_SHOW );
//	for ( WORD wChairID = 0; wChairID < GAME_PLAYER; ++wChairID ) {
//		m_ScoreView.SetGameScore( wChairID, "dkfjdkfjdf", 00 );
//	}
	//////////////////////////////////////////////////////////////////////////////

#ifdef VIDEO_GAME

	//视频组件
	CRect VDRect;
	m_DlgVedioService[ 0 ].GetWindowRect( &VDRect );

	m_DlgVedioService[ 0 ].MoveWindow( nWidth - m_nXBorder - 5 - VDRect.Width(),m_nYBorder + 5 , VDRect.Width(), VDRect.Height() );
	m_DlgVedioService[ 1 ].MoveWindow( m_nXBorder + 5, nHeight / 2 - 10, VDRect.Width(), VDRect.Height() );
	m_DlgVedioService[ 3 ].MoveWindow( nWidth - VDRect.Width() - m_nXBorder - 5, nHeight / 2 - 10, VDRect.Width(), VDRect.Height() );

	m_DlgVedioService[ ME_VIEW_CHAIR ].GetWindowRect( &VDRect );
	m_DlgVedioService[ ME_VIEW_CHAIR ].MoveWindow( m_nXBorder + 5, nHeight - VDRect.Height() - m_nYBorder - 5, VDRect.Width(), VDRect.Height() );

#endif

	//设置坐标
	m_ptFace[ME_VIEW_CHAIR].x=nWidth/2-m_nXFace/2;
	m_ptFace[ME_VIEW_CHAIR].y=nHeight-m_nYBorder-m_nYFace-5;
	m_ptName[ME_VIEW_CHAIR].x=nWidth/2+5+m_nXFace/2;
	m_ptName[ME_VIEW_CHAIR].y=nHeight-m_nYBorder-m_nYFace-2;
	m_ptTimer[ME_VIEW_CHAIR].x=nWidth/2;//nWidth/2-m_nXFace/2-m_nXTimer/2-5;
	m_ptTimer[ME_VIEW_CHAIR].y=nHeight/2;//nHeight-m_nYBorder-m_nYTimer/2-2;
	m_ptReady[ME_VIEW_CHAIR].x=nWidth/2 + 10;
	m_ptReady[ME_VIEW_CHAIR].y=nHeight-217-m_nYBorder;
	m_ptScore[ME_VIEW_CHAIR].x=nWidth/2-21;
	m_ptScore[ME_VIEW_CHAIR].y=nHeight-m_nYBorder-m_nYFace-210;
	m_ptLand[ME_VIEW_CHAIR].x=nWidth/2-m_nXFace/2-m_LandSize.cx-m_nXTimer*2;
	m_ptLand[ME_VIEW_CHAIR].y=nHeight-m_nXBorder-m_LandSize.cy-5;

	//移动按钮
	CRect rcButton;
	HDWP hDwp=BeginDeferWindowPos(32);
	m_btStart.GetWindowRect(&rcButton);
	const UINT uFlags=SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOSIZE;
	DeferWindowPos(hDwp,m_btStart,NULL,(nWidth-rcButton.Width())/2,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btOutCard,NULL,nWidth/2-rcButton.Width()*3/2-10,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btAutoOutCard,NULL,nWidth/2-rcButton.Width()/2,nHeight-217-m_nYBorder+99999,0,0,uFlags);//提示
	DeferWindowPos(hDwp,m_btPassCard,NULL,nWidth/2+rcButton.Width()/2+10,nHeight-217-m_nYBorder,0,0,uFlags);

	DeferWindowPos(hDwp,m_btOneScore,	NULL,nWidth/2-rcButton.Width()*2-12+30,						nHeight-217-m_nYBorder,0,0,uFlags);//1分+9999
	DeferWindowPos(hDwp,m_btTwoScore,	NULL,nWidth/2+10+30,nHeight-217-m_nYBorder,					0,0,uFlags);//2分
	DeferWindowPos(hDwp,m_btThreeScore,	NULL,nWidth/2-rcButton.Width()*2-12+30,nHeight-217-m_nYBorder,	0,0,uFlags);//3分
	DeferWindowPos(hDwp,m_btGiveUpScore,NULL,nWidth/2+10+30,nHeight-217-m_nYBorder,					0,0,uFlags);//不叫

	DeferWindowPos(hDwp,m_btSortCard,NULL,nWidth-m_nXBorder-rcButton.Width()-15,nHeight-rcButton.Height()*2-m_nYBorder-40,0,0,uFlags);
	///////////////////////////////////////////////////添加按钮的位置///////////////////////////////////////////
	DeferWindowPos(hDwp,m_btLiGun,NULL,nWidth/2-rcButton.Width()*3/2-10,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btNotLiGun,NULL,nWidth/2+rcButton.Width()/2+10,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btJueGun,NULL,nWidth/2-rcButton.Width()*3/2-10,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btNotJueGun,NULL,nWidth/2+rcButton.Width()/2+10,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btAgree,NULL,nWidth/2-rcButton.Width()*3/2-10,nHeight-217-m_nYBorder,0,0,uFlags);
	DeferWindowPos(hDwp,m_btDisagree,NULL,nWidth/2+rcButton.Width()/2+10,nHeight-217-m_nYBorder,0,0,uFlags);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	EndDeferWindowPos(hDwp);

	CRect rcBtAutoPlay;
	m_btAutoPlayOn.GetWindowRect( rcBtAutoPlay );
	m_btAutoPlayOn .MoveWindow( nWidth - m_nXBorder - 5 - rcBtAutoPlay.Width() , nHeight-5-rcBtAutoPlay.Height(), rcBtAutoPlay.Width(), rcBtAutoPlay.Height(), FALSE);
    m_btAutoPlayOff.MoveWindow( nWidth - m_nXBorder - 5 - rcBtAutoPlay.Width(), nHeight-5-rcBtAutoPlay.Height(), rcBtAutoPlay.Width(), rcBtAutoPlay.Height(), FALSE);
	m_btSortCard.MoveWindow( nWidth - m_nXBorder - 5 - rcBtAutoPlay.Width(), nHeight-10-2*rcBtAutoPlay.Height(), rcBtAutoPlay.Width(), rcBtAutoPlay.Height(), FALSE);

	//移动扑克
	//m_BackCardControl.SetBenchmarkPos(m_nXBorder+5,m_nYBorder+5,enXLeft,enYTop);
	m_HandCardControl[ME_VIEW_CHAIR].SetBenchmarkPos(nWidth/2+15,nHeight-m_nYFace-m_nYBorder-25,enXCenter,enYBottom);
	m_UserCardControl[ME_VIEW_CHAIR].SetBenchmarkPos(nWidth/2,nHeight-223-m_nYBorder,enXCenter,enYBottom);

	return;
}

//绘画界面
void CGameClientView::DrawGameView(CDC * pDC, int nWidth, int nHeight)
{
	//绘画背景
	DrawViewImage(pDC,m_ImageBack,enMode_Spread);
	DrawViewImage(pDC,m_ImageCenter,enMode_Centent);

	//绘画用户
	TCHAR szBuffer[32];
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		//变量定义
		WORD wUserTimer=GetUserTimer(i);
		const tagUserData * pUserData=GetUserInfo(i);

		//绘画用户
		if (pUserData!=NULL)
		{
			//用户名字
			pDC->SetTextAlign( ( ( i == 3 && ! m_bDeasilOrder ) || ( i == 0 && m_bDeasilOrder ) ) ? TA_RIGHT : TA_LEFT );
			pDC->SetTextColor((wUserTimer>0)?RGB(250,250,250):RGB(220,220,220));
			DrawTextString( pDC, pUserData->szName, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), m_ptName[i].x,m_ptName[i].y );

			//剩余扑克
			{
				_snprintf(szBuffer,sizeof(szBuffer),TEXT("剩余：%d 张"),m_bCardCount[i]);
				DrawTextString( pDC, szBuffer, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), m_ptName[i].x,m_ptName[i].y+18 );
			}

			//CImageHandle HandleCard(&m_Imagetime);
			//m_Imagetime.AlphaDrawImage(pDC,m_ptTimer[i].x,m_ptTimer[i].y,/*nWidth/2-465, nHeight/2-355,*/RGB(255,0,255));

			//其他信息
			if (wUserTimer!=0)
			{ 
				DrawUserTimer(pDC,m_ptTimer[i].x,m_ptTimer[i].y,wUserTimer);
				//CImageHandle HandleCard(&m_Imagetimech);
				//for(int i=wUserTimer-2; i>=0; i--)
				//m_Imagetimech.AlphaDrawImage(pDC,nWidth/2-396+(i*10), nHeight/2-350,RGB(255,0,255));
			}
			if (pUserData->cbUserStatus==US_READY) DrawUserReady(pDC,m_ptReady[i].x,m_ptReady[i].y);
			DrawUserFace(pDC,pUserData,m_ptFace[i].x,m_ptFace[i].y,pUserData->cbUserStatus==US_OFFLINE);

			//托管标识
			if ( m_bUserTrustee[ i ] ) {
			if ( i == ME_VIEW_CHAIR )
				m_ImageUserTrustee.DrawImage( pDC, m_ptLand[ i ].x + 40, m_ptLand[ i ].y - 1 );
			else if ( i == 0 )
				m_ImageUserTrustee.DrawImage( pDC, m_ptLand[ i ].x + 40, m_ptFace[ i ].y + m_nYFace -
				m_ImageUserTrustee.GetHeight() + 3 );
			else if ( i == 1 )
				m_ImageUserTrustee.DrawImage( pDC, m_ptFace[ i ].x + m_nXFace + 8, m_ptFace[ i ].y + m_nYFace -
				m_ImageUserTrustee.GetHeight() + 3 );
			else
				m_ImageUserTrustee.DrawImage( pDC, m_ptFace[ i ].x - m_nXFace - 8, m_ptFace[ i ].y + m_nYFace -
				m_ImageUserTrustee.GetHeight() + 3 );
		}
		}
	}

	//叫分标志
	int nXImagePos,nImageWidth=m_ImageScore.GetWidth(),nImageHeight=m_ImageScore.GetHeight();
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		if ((m_bScore[i]!=0)||(m_bPass[i]==true))
		{
			if ( m_bPass[i]==true ) nXImagePos=m_ImageScore.GetWidth()*4/5;
			else if ( m_bScore[i]==255 ) nXImagePos=m_ImageScore.GetWidth()*3/5;
			else nXImagePos=(m_bScore[i]-1)*m_ImageScore.GetWidth()/5;
			m_ImageScore.DrawImage(pDC,m_ptScore[i].x,m_ptScore[i].y,nImageWidth/5,nImageHeight,nXImagePos,0);
		}
	}

	//庄家位置
	if (m_wLandUser!=INVALID_CHAIR)
	{
		//获取用户
		const tagUserData * pUserData=GetUserInfo(m_wLandUser);
		//tagUserData *pUserData = new tagUserData;
		//strcpy( (char*)pUserData->szName, "dkjfdkfjdkfj" );
		if (pUserData!=NULL)
		{
			//变量定义
			TCHAR szBuffer[64];

			//设置 DC
			pDC->SetTextAlign(TA_LEFT|TA_TOP);
			pDC->SetTextColor(RGB(250,250,250));

			//扑克高度
			CRect CardRect;
			//m_BackCardControl.GetWindowRect( &CardRect );
			ScreenToClient( &CardRect );

			//绘画信息
			m_ImageLand.DrawImage(pDC,m_ptLand[m_wLandUser].x,m_ptLand[m_wLandUser].y);
			_snprintf(szBuffer,sizeof(szBuffer),TEXT("地主：%s"),pUserData->szName);
			DrawTextString( pDC, szBuffer, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), m_nXBorder+10,CardRect.bottom+10 );

			//绘画头像
		//	DrawUserFace(pDC,pUserData->wFaceID,m_nXBorder+5,m_nYBorder+5,false);

			//叫牌分数
			LONG lScore=m_cbLandScore*m_lBaseScore;
			_snprintf(szBuffer,sizeof(szBuffer),TEXT("分数：%ld 分"),lScore);
			DrawTextString( pDC, szBuffer, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), m_nXBorder+10,CardRect.bottom+30 );
		}
	}

	//绘画爆炸
	if (m_bBombEffect==true)
	{
		//加载资源
		CImageHandle HandleBombEffect(&m_ImageBombEffect);

		//绘画效果
		INT nImageHeight=m_ImageBombEffect.GetHeight();
		INT nImageWidth=m_ImageBombEffect.GetWidth()/BOMB_EFFECT_COUNT;
		m_ImageBombEffect.AlphaDrawImage(pDC,(nWidth-nImageWidth)/2,0,nImageWidth,nImageHeight,
			nImageWidth*(m_cbBombFrameIndex%BOMB_EFFECT_COUNT),0,RGB(255,0,255));
	}

	//炸弹倍数
	if (m_wBombTime>1)
	{
		//扑克高度
		CRect CardRect;
		//m_BackCardControl.GetWindowRect( &CardRect );
		ScreenToClient( &CardRect );

		//炸弹标志
		CImageHandle ImageHandle(&m_ImageBomb);
		m_ImageBomb.AlphaDrawImage(pDC,m_nXBorder+5,50+CardRect.bottom,RGB(255,0,255));

		//设置 DC
		pDC->SetTextColor(RGB(250,250,250));
		pDC->SetTextAlign(TA_LEFT|TA_TOP);

		//炸弹倍数
		TCHAR szBuffer[64];
		_snprintf(szBuffer,sizeof(szBuffer),TEXT("炸弹翻倍：%d"),m_wBombTime);
		DrawTextString( pDC, szBuffer, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), m_nXBorder+m_ImageBomb.GetWidth()+10,CardRect.bottom+60 );
	}

	//描述信息
	if (m_bLandTitle==true)
	{
		//创建资源
		CFont InfoFont;
		InfoFont.CreateFont(-36,0,0,0,400,0,0,0,134,3,2,1,2,TEXT("黑体"));

		//设置 DC
		pDC->SetTextColor(RGB(201,229,133));
		pDC->SetTextAlign(TA_CENTER|TA_TOP);
		CFont * pOldFont=pDC->SelectObject(&InfoFont);

		//绘画信息
		pDC->TextOut(nWidth/2,200,"正等待用户叫分");

		//清理资源
		pDC->SelectObject(pOldFont);
		InfoFont.DeleteObject();
	}

	return;
}

//炸弹倍数
void CGameClientView::SetBombTime(WORD wBombTime)
{
	//设置变量
	m_wBombTime=wBombTime;

	//刷新界面
	UpdateGameView(NULL);

	return;
}

//基础分数
void CGameClientView::SetBaseScore(LONG lBaseScore)
{
	//设置变量
	m_lBaseScore=lBaseScore;

	//刷新界面
	UpdateGameView(NULL);

	return;
}

//显示提示
void CGameClientView::ShowLandTitle(bool bLandTitle)
{
	//设置变量
	m_bLandTitle=bLandTitle;

	//刷新界面
	UpdateGameView(NULL);

	return;
}

//设置放弃
void CGameClientView::SetPassFlag(WORD wChairID, bool bPass)
{
	//设置变量
	if (wChairID==INVALID_CHAIR)
	{
		for (WORD i=0;i<GAME_PLAYER;i++) m_bPass[i]=bPass;
	}
	else m_bPass[wChairID]=bPass;

	//更新界面
	UpdateGameView(NULL);

	return;
}

//扑克数目
void CGameClientView::SetCardCount(WORD wChairID, BYTE bCardCount)
{
	//设置变量
	if (wChairID==INVALID_CHAIR)
	{
		for (WORD i=0;i<GAME_PLAYER;i++) m_bCardCount[i]=bCardCount;
	}
	else m_bCardCount[wChairID]=bCardCount;

	//更新界面
	UpdateGameView(NULL);

	return;
}

//挖坑分数
void CGameClientView::SetLandUser(WORD wChairID, BYTE bLandScore)
{
	//设置变量
	m_wLandUser=wChairID;
	m_cbLandScore=bLandScore;

	//更新界面
	UpdateGameView(NULL);

	return;
}

//挖坑分数
void CGameClientView::SetLandScore(WORD wChairID, BYTE bLandScore)
{
	//设置变量
	if (wChairID!=INVALID_CHAIR) m_bScore[wChairID]=bLandScore;
	else memset(m_bScore,0,sizeof(m_bScore));

	//更新界面
	UpdateGameView(NULL);

	return;
}

//开始按钮
void CGameClientView::OnStart()
{

#ifdef VIDEO_GAME
	//隐藏扑克
	m_HandCardControl[ 0 ].ShowWindow( SW_HIDE );
	m_HandCardControl[ 1 ].ShowWindow( SW_HIDE );
	m_HandCardControl[ 3 ].ShowWindow( SW_HIDE );
#endif

	AfxGetMainWnd()->SendMessage(IDM_START,0,0);
	return;
}

//出牌按钮
void CGameClientView::OnOutCard()
{
	AfxGetMainWnd()->SendMessage(IDM_OUT_CARD,1,1);
	return;
}

//放弃按钮
void CGameClientView::OnPassCard()
{
	AfxGetMainWnd()->SendMessage(IDM_PASS_CARD,1,1);
	return;
}

//叫1分按钮//现在为叉
void CGameClientView::OnOneScore()
{
	AfxGetMainWnd()->SendMessage(IDM_SORT_HAND_CHA,1,1);
	return;
}

//叫2分按钮//现在为不叉
void CGameClientView::OnTwoScore()
{
	AfxGetMainWnd()->SendMessage(IDM_SORT_HAND_NOCHA,1,1);
	return;
}

//叫3分按钮//现在为点
void CGameClientView::OnThreeScore()
{
	AfxGetMainWnd()->SendMessage(IDM_SORT_HAND_DIAN,1,1);
	return;
}
//放弃按钮//现在为不点
void CGameClientView::OnGiveUpScore()
{
	AfxGetMainWnd()->SendMessage(IDM_SORT_HAND_NODIAN,1,1);
	return;
}

//出牌提示
void CGameClientView::OnAutoOutCard()
{
	AfxGetMainWnd()->SendMessage(IDM_AUTO_OUTCARD,0,0);
	return;
}
//////////////////////////////////////////////////////////////////////////
// 自动托管
void CGameClientView::OnAutoPlayerOn()
{
    m_btAutoPlayOn.ShowWindow(SW_HIDE);
    m_btAutoPlayOff.ShowWindow(SW_SHOW);
	AfxGetMainWnd()->SendMessage(IDM_LAND_AUTOPLAY,1,1);
    return;
}

// 取消托管
void CGameClientView::OnAutoPlayerOff()
{
    m_btAutoPlayOn.ShowWindow(SW_SHOW);
    m_btAutoPlayOff.ShowWindow(SW_HIDE);
	AfxGetMainWnd()->SendMessage(IDM_LAND_AUTOPLAY,0,0);
    return;
}

//设置顺序
void CGameClientView::SetUserOrder(bool bDeasilOrder) {

	//状态判断
	if (m_bDeasilOrder==bDeasilOrder) return;

	//设置变量
	m_bDeasilOrder=bDeasilOrder;
	AfxGetApp()->WriteProfileInt(TEXT("GameOption"),TEXT("DeasilOrder"),m_bDeasilOrder?TRUE:FALSE);

	//设置界面
	CRect rcClient;
	GetClientRect(&rcClient);

	RectifyGameView(rcClient.Width(),rcClient.Height());

	//刷新界面
	UpdateGameView(NULL);
}

//设置爆炸
bool CGameClientView::SetBombEffect(bool bBombEffect)
{
	if (bBombEffect==true)
	{
		//设置变量
		m_bBombEffect=true;
		m_cbBombFrameIndex=0;

		//启动时间
		SetTimer(IDI_BOMB_EFFECT,100,NULL);
	}
	else
	{
		//停止动画
		if (m_bBombEffect==true)
		{
			//删除时间
			KillTimer(IDI_BOMB_EFFECT);

			//设置变量
			m_bBombEffect=false;
			m_cbBombFrameIndex=0;

			//更新界面
			UpdateGameView(NULL);
		}
	}

	return true;
}

//时间消息
VOID CGameClientView::OnTimer(UINT nIDEvent)
{
	//爆炸动画
	if (nIDEvent==IDI_BOMB_EFFECT)
	{
		//停止判断
		if (m_bBombEffect==false)
		{
			KillTimer(IDI_BOMB_EFFECT);
			return;
		}

		//设置变量
		if ((m_cbBombFrameIndex+1)>=BOMB_EFFECT_COUNT)
		{
			//删除时间
			KillTimer(IDI_BOMB_EFFECT);

			//设置变量
			m_bBombEffect=false;
			m_cbBombFrameIndex=0;
		}
		else m_cbBombFrameIndex++;

		//更新界面
		UpdateGameView(NULL);

		return;
	}

	__super::OnTimer(nIDEvent);
}


//艺术字体
void CGameClientView::DrawTextString(CDC * pDC, LPCTSTR pszString, COLORREF crText, COLORREF crFrame, int nXPos, int nYPos)
{
	//变量定义
	int nStringLength=lstrlen(pszString);
	int nXExcursion[8]={1,1,1,0,-1,-1,-1,0};
	int nYExcursion[8]={-1,0,1,1,1,0,-1,-1};

	//绘画边框
	pDC->SetTextColor(crFrame);
	for (int i=0;i<CountArray(nXExcursion);i++)
	{
		pDC->TextOut(nXPos+nXExcursion[i],nYPos+nYExcursion[i],pszString,nStringLength);
	}

	//绘画字体
	pDC->SetTextColor(crText);
	pDC->TextOut(nXPos,nYPos,pszString,nStringLength);

	return;
}

//玩家托管
void CGameClientView::SetUserTrustee( WORD wChairID, bool bTrustee ) {

	//设置变量
	if ( INVALID_CHAIR == wChairID ) {
		for ( WORD wChairIdx = 0; wChairIdx < GAME_PLAYER; wChairIdx++ ) {
			m_bUserTrustee[ wChairIdx ] = bTrustee;
		}
	}
	else {
		m_bUserTrustee[ wChairID ] = bTrustee;
	}

	//更新界面
	UpdateGameView( NULL );
}

//排序按钮
VOID CGameClientView::OnBnClickedSortCard()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_SORT_HAND_CARD,0,0);

	return;
}

///////////////////////////////////////////////////////////////////////////////////
void CGameClientView::OnHitLiGun()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_LI_GUN,0,0);

	return;	
}

void CGameClientView::OnHitJueGun()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_JUE_GUN,0,0);

	return;	
}

void CGameClientView::OnHitAgree()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_AGREE,1,1);

	return;	
}

void CGameClientView::OnHitDisagree()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_AGREE,0,0);

	return;	
}

void CGameClientView::OnHitNotLiGun()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_NOT_LI_GUN,0,0);

	return;	
}

void CGameClientView::OnHitNotJueGun()
{
	//发送消息
	AfxGetMainWnd()->SendMessage(IDM_NOT_JUE_GUN,0,0);

	return;	
}