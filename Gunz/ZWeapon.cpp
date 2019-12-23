#include "stdafx.h"

#include "ZGame.h"
#include "ZWeapon.h"
#include "RBspObject.h"

#include "ZEffectBillboard.h"
#include "ZEffectSmoke.h"

#include "ZSoundEngine.h"
#include "ZApplication.h"

#include "MDebug.h"
#include "ZConfiguration.h"
#include "RealSoundEffect.h"

#include "ZEffectFlashBang.h"
#include "ZPost.h"
#include "ZStencilLight.h"

#include "ZGameConst.h"

#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"



#define BOUND_EPSILON	5
#define LANDING_VELOCITY	20
#define MAX_ROT_VELOCITY	50


#define ROCKET_VELOCITY			2700.f	// 2004년 9월 21일자 이전은 2500.f
#define ROCKET_SPLASH_RANGE		350.f	// 로켓 스플래시 범위
#define ROCKET_MINIMUM_DAMAGE	.3f		// 로켓 미니멈 데미지
#define ROCKET_KNOCKBACK_CONST	.5f		// 로켓에 튕겨져 나가는 상수

MImplementRootRTTI(ZWeapon);

ZWeapon::ZWeapon() : m_pVMesh(NULL), m_nWorldItemID(0), m_WeaponType(ZWeaponType_None), m_SLSid(0) {
	m_nItemUID = -1;
}

ZWeapon::~ZWeapon() {
	if(m_pVMesh){
		delete m_pVMesh;
		m_pVMesh = NULL;
	}

	if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
		ZGetStencilLight()->DeleteLightSource( m_SLSid );
		m_SLSid = 0;
	}
}

void ZWeapon::Create(RMesh* pMesh) {
	m_pVMesh = new RVisualMesh;
	m_pVMesh->Create(pMesh);
}

void ZWeapon::Render() {
	m_pVMesh->Frame();
	m_pVMesh->Render();
}

bool ZWeapon::Update(float fElapsedTime)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////

MImplementRTTI(ZMovingWeapon, ZWeapon);

ZMovingWeapon::ZMovingWeapon() : ZWeapon() {
	m_WeaponType = ZWeaponType_MovingWeapon;
	m_PostPos = rvector(-1,-1,-1);
}

ZMovingWeapon::~ZMovingWeapon() {
}

void ZMovingWeapon::Explosion() {
}

/////////////////////////// 로켓

MImplementRTTI(ZWeaponRocket,ZMovingWeapon);

ZWeaponRocket::ZWeaponRocket() :ZMovingWeapon() {

}

ZWeaponRocket::~ZWeaponRocket() {

}

void ZWeaponRocket::Create(RMesh* pMesh,rvector &pos, rvector &dir,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_Position=pos;
	m_Velocity=dir*ROCKET_VELOCITY;

	m_fStartTime = ZGetGame()->GetTime();
	m_fLastAddTime = ZGetGame()->GetTime();

	m_Dir=dir;
	m_Up=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		//_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage.Ref();
	
	if( Z_VIDEO_DYNAMICLIGHT ) {
		//_ASSERT(m_SLSid==0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}
	CHECK_RETURN_CALLSTACK(Create);
	//ZGetEffectManager()->AddRocketSmokeEffect(m_Position,-RealSpace2::RCameraDirection);
}

#define ROCKET_LIFE			10.f		// 10초 뒤에 터짐


bool ZWeaponRocket::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	if(ZGetGame()->GetTime() - m_fStartTime > ROCKET_LIFE ) {
		// 수류탄은 터지는 순간에 이펙트 추가.. 삭제...
		Explosion();

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}

		return false;
	}

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos;
		ZPICKINFO zpi;
		bool bPicked=ZGetGame()->Pick(ZGetObjectManager()->GetObject(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
				pickpos=zpi.bpi.PickPos;
			else
				if(zpi.pObject)
					pickpos=zpi.info.vOut;

		}
/*
		RBSPPICKINFO bpi;
		bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position,dir,&bpi,dwPickPassFlag);
*/
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			Explosion();

			if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
			{
				ZGetStencilLight()->DeleteLightSource( m_SLSid );
				m_SLSid = 0;
				ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
			}

			return false;
		}else
			m_Position+=diff;
	}

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetWorldMatrix(mat);

	float this_time = ZGetGame()->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

#define _ROCKET_RAND_CAP 10

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

		ZGetEffectManager()->AddRocketSmokeEffect(pos);
//		ZGetEffectManager()->AddSmokeEffect(NULL,pos,rvector(0,0,0),rvector(0,0,0),60.f,80.f,1.5f);
		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);

	return true;
}

void ZWeaponRocket::Render()
{
	ZWeapon::Render();

	//add

//	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);
//	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

//	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
//	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
//	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//alpha
/*
	RGetDevice()->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
	RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TFACTOR );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
*/

}

void ZWeaponRocket::Explosion()
{
	// 자신의 이펙트 그리고..
	// 주변 유닛들에게 데미지 주는 메시지 날리고..
	// ( 메시지 받은곳에서 주변 유닛들 체크하고.. 
	// 죽은 유닛이 있다면 발사유닛의 킬수를 올려주기)

	rvector v = m_Position;

	//	if(v.z < 0.f)	v.z = 0.f;//높이

	rvector dir = -RealSpace2::RCameraDirection;
	ZGetEffectManager()->AddRocketEffect(v,dir);

	// 메시지를 날리거나~
	// 각각의 클라이언트의 무기 정보를 믿고 처리하거나~

	ZGetGame()->OnExplosionGrenade(m_uidOwner,v,m_fDamage,ROCKET_SPLASH_RANGE,ROCKET_MINIMUM_DAMAGE,ROCKET_KNOCKBACK_CONST,m_nTeamID);

	ZGetSoundEngine()->PlaySound("fx_explosion01",v);

	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );

}

////////////////////////// 메디킷

MImplementRTTI(ZWeaponItemKit,ZMovingWeapon);

ZWeaponItemKit::ZWeaponItemKit() :ZMovingWeapon()
{
//	m_nWeaponType = 0;
	m_fDeathTime = 0;
	m_nWorldItemID = 0;

	m_bInit = false;
	m_bDeath = false;
	m_bSendMsg = false;
	m_bWorldItemFloorDrop = false;
}

ZWeaponItemKit::~ZWeaponItemKit() 
{

}

void ZWeaponItemKit::Create(RMesh* pMesh,rvector &pos, rvector &velocity,ZObject* pOwner)
{
	ZWeapon::Create(pMesh);

	m_Position=pos;
	rvector dir=velocity;
	Normalize(dir);
	m_Velocity=velocity;

	m_fStartTime=ZGetGame()->GetTime();

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	m_bInit = false;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		//_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage.Ref();

//	m_nSpItemID = m_nMySpItemID++;

	m_bSendMsg = false;
	m_bWorldItemFloorDrop = false;
}

rvector GetReflectionVector(rvector& v,rvector& n) {

	float dot = D3DXVec3Dot(&(-v),&n);

	return (2*dot)*n+v;
}

void ZWeaponItemKit::Render()
{
	if(m_bInit) {
		if(m_pVMesh->m_pMesh) {
		// 무기 파츠를 캐릭터에 붙어있을경우 조명을 받고 안붙어있을경우 조명을 안받는 2가지용도로사용하기때문		

		rmatrix mat;
		MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);
		m_pVMesh->SetWorldMatrix(mat);
//		m_pVMesh->m_pMesh->m_LitVertexModel = true;
		ZMovingWeapon::Render();
//		m_pVMesh->m_pMesh->m_LitVertexModel = false;

		}
	}
}

void ZWeaponItemKit::UpdateFirstPos()
{
	/////////////////////////////////////// 지금은 계산해서 보내준다...

	m_bInit = true;
	return;

	if(m_bInit==false) {

		ZCharacter* pC = ZGetCharacterManager()->Find(m_uidOwner);

		if(pC) {
			if(pC->m_pVMesh) {

				rvector vWeapon[1];

				vWeapon[0] = pC->m_pVMesh->GetCurrentWeaponPosition();

				// 벽뒤로 던지는 경우가 생기면
				rvector nPos = pC->m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
				rvector nDir = vWeapon[0] - nPos;

				Normalize(nDir);

				RBSPPICKINFO bpi;
				// 아래를 보면서 던지면 벽을 통과해서...
				if(ZGetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))
				{
					if(D3DXPlaneDotCoord(&(bpi.pInfo->plane),&vWeapon[0])<0) {
						vWeapon[0] = bpi.PickPos - nDir;
					}
				}

				m_Position = vWeapon[0];
			}
		}

		m_bInit = true;
	}
}

void ZWeaponItemKit::UpdatePost(DWORD dwPickPassFlag, float fDropDelayTime)
{
	if(m_uidOwner == ZGetGameClient()->GetPlayerUID()) {
		ZPostRequestSpawnWorldItem(ZGetGameClient()->GetPlayerUID(), m_nWorldItemID, m_Position, fDropDelayTime);
		m_PostPos = m_Position;
		m_bSendMsg = true;
	}
}

void ZWeaponItemKit::UpdateWorldItemFloorDrop(DWORD dwPickPassFlag)
{
	RBSPPICKINFO rpi;
	bool bPicked = ZGetWorld()->GetBsp()->Pick(m_Position,rvector(0,0,-1),&rpi, dwPickPassFlag );

	if(bPicked && fabsf(Magnitude(rpi.PickPos - m_Position)) < 5.0f ) {
		if(m_bWorldItemFloorDrop == false) {
			m_bWorldItemFloorDrop = true;
			m_fDeathTime = ZGetGame()->GetTime() + 2.0f;//네트웍 전송 시간고려 최대 2.0초뒤에 자동 소멸
		}
	}
}

void ZWeaponItemKit::UpdatePos(float fElapsedTime,DWORD dwPickPassFlag)
{
	rvector diff = m_Velocity * fElapsedTime;

	rvector dir = diff;
	Normalize(dir);

	float fDist = Magnitude(diff);

	rvector pickpos;
	rvector normal=rvector(0,0,1);

	ZPICKINFO zpi;

	bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);

	if(bPicked) {
		if(zpi.bBspPicked)	{
			pickpos=zpi.bpi.PickPos;
			rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
			normal=rvector(plane.a,plane.b,plane.c);
		}
		else if(zpi.pObject) {
			pickpos=zpi.info.vOut;
			if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
			{
				normal=pickpos-zpi.pObject->GetPosition();
				normal.z=0;
			}
			else
				normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
			Normalize(normal);
		}
	}

	if(bPicked && fabsf(Magnitude(pickpos-m_Position)) < fDist)
	{ // 언덕이나 난간위에서 땅에 떨어짐(기본 떨어지는 시간보다 많이 걸릴경우)
		m_Position = pickpos + normal;
		m_Velocity = GetReflectionVector(m_Velocity,normal);
		m_Velocity *= zpi.pObject ? 0.1f: 0.2f;	// 캐릭터에 충돌하면 속도가 많이 준다
		m_Velocity *= 0.2f;

		// 벽방향의 속도성분을 줄인다.
		Normalize(normal);
		float fAbsorb=DotProduct(normal,m_Velocity);
		m_Velocity -= 0.1*fAbsorb*normal;

		float fA=RANDOMFLOAT*2*pi;
		float fB=RANDOMFLOAT*2*pi;
		m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

	} else { // 떨어지고있음
		m_Position+=diff;
	}
}

// 땅에 닿으면 소멸~
bool ZWeaponItemKit::Update(float fElapsedTime)
{
	if(m_bDeath) {
		return false;
	}

	if(m_bWorldItemFloorDrop) // 월드 아이템이 땅에 떨어지면 처리하지 않는다.
		return true;

//	if(m_bSendMsg) { // 외부에서 bDeath 로만 지운다..
////		if( g_pGame->GetTime() > m_fDeathTime ) 
////			return false;
////		else 
//			return true;
//	}

	if(ZGetGame()->GetTime() - m_fStartTime < m_fDelayTime) {
		return true;
	}

	UpdateFirstPos();

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;
	
	{	// 월드아이템이 떨어질 시간과 위치를 미리 계산후 서버에 메세지를 미리 보내준다....20090213 by kammir
		rvector vTempPos = m_Position;
		rvector vTempVelocity = m_Velocity;

		if(m_bSendMsg == false)
		{ // 메세지를 안보냈을때에만 실행... 한번만 적용돼겠금한다.
			float fTick = 0.02f;
			for(int i=0; i<500; i++)
			{ // 10초 정도를 미리 예상하여 아이템이 떨어지는걸 확인한다.
				m_Velocity.z -= 500.f * fTick;
				UpdatePos( fTick , dwPickPassFlag );		// 아이템이 계속 떨어진다.
				UpdateWorldItemFloorDrop(dwPickPassFlag);	// 아이템이 땅에 떨어졌는지 확인

				if(m_bWorldItemFloorDrop == true)
				{ // 땅에 부딪혔으면 서버에 메세지를 날린다.
					UpdatePost( dwPickPassFlag, fTick*i);
					m_bWorldItemFloorDrop = false;
					break;
				}
			}
			// 만약 10초가 지나도 바닥에 떨어지지 않았으면 아이템을 그냥 죽여준다.
			m_bSendMsg = true;
		}

		m_Position = vTempPos;			// 메세지를 보내기위해 미리 계산한 변경된 값을 원래대로 돌려준다.
		m_Velocity = vTempVelocity;
	}

	// 월드아이템이 떨어지는걸 화면에 보여질수있게 계산해준다.
	m_Velocity.z -= 500.f * fElapsedTime;		// 시간에 따른 땅에 떨어지는 속도
	UpdatePos( fElapsedTime , dwPickPassFlag ); // 속도와 월드아이템의 높이를 시간당 계산함으로써 아래로 떨어진다.
	UpdateWorldItemFloorDrop(dwPickPassFlag);	// 아이템이 땅에 떨어졌는지 확인(땅바닥하고 월드아이템의 높이가 5미만이면 처리)

	rmatrix mat;
	rvector dir = m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetWorldMatrix(mat);

	return true;
}

void ZWeaponItemKit::Explosion()
{
	// 자신의 이펙트 그리고..
	// 주변 유닛들에게 데미지 주는 메시지 날리고..
	// ( 메시지 받은곳에서 주변 유닛들 체크하고.. 
	// 죽은 유닛이 있다면 발사유닛의 킬수를 올려주기)

//	rvector v = m_Position;
//	rvector dir = -RealSpace2::RCameraDirection;
//	ZGetEffectManager()->AddGrenadeEffect(v,dir);

	// 메시지를 날리거나~
	// 각각의 클라이언트의 무기 정보를 믿고 처리하거나~

//	g_pGame->OnExplosionGrenade(m_uidOwner,v,m_fDamage,400.f,m_nTeamID);

//	static RealSoundEffectSource* pSES= ZApplication::GetSoundEngine()->GetSES("we_grenade_explosion");
//	if( pSES != NULL ) {
//		ZApplication::GetSoundEngine()->PlaySE(pSES,v.x,v.y,v.z);
//	}

//	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}


////////////////////////// 수류탄

MImplementRTTI(ZWeaponGrenade,ZMovingWeapon);

void ZWeaponGrenade::Create(RMesh* pMesh,rvector &pos, rvector &velocity,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_Position=pos;
	rvector dir=velocity;
	Normalize(dir);
	m_Velocity=velocity;

	m_fStartTime=ZGetGame()->GetTime();

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		//_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage.Ref();

	m_nSoundCount = rand() % 2 + 2;	// 2~ 3번 소리를 낸다
}


#define GRENADE_LIFE			2.f		// 3초 뒤에 터짐

// 서로간에 동기화 되어야 한다.
bool ZWeaponGrenade::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;
	if(ZGetGame()->GetTime() - m_fStartTime > GRENADE_LIFE) {
		// 수류탄은 터지는 순간에 이펙트 추가.. 삭제...
		Explosion();
		if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );
		return false;
	}

	m_Velocity.z-=1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos,normal;

		ZPICKINFO zpi;
		bool bPicked=ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos=zpi.bpi.PickPos;
				rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal=rvector(plane.a,plane.b,plane.c);
			}
			else
			if(zpi.pObject)
			{
				pickpos=zpi.info.vOut;
				if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
				{
					normal=pickpos-zpi.pObject->GetPosition();
					normal.z=0;
				}else
					normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
				Normalize(normal);
			}
		}


		// 어딘가에 충돌했다
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		//*/

		/*
		RBSPPICKINFO bpi;
		bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position,dir,&bpi,dwPickPassFlag);
		if(bPicked)
		{
			pickpos=bpi.PickPos;
			rplane plane=bpi.pNode->pInfo[bpi.nIndex].plane;
			normal=rvector(plane.a,plane.b,plane.c);
		}

		if(bPicked && fabsf(Magnitude(bpi.PickPos-m_Position))<fDist)
		*/
		{
			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;		// 캐릭터에 충돌하면 속도가 많이 준다

			// 소리를 낸다 테스트
			if(zpi.bBspPicked && m_nSoundCount>0) {
				m_nSoundCount--;
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			// 벽방향의 속도성분을 줄인다.
			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA=RANDOMFLOAT*2*pi;
			float fB=RANDOMFLOAT*2*pi;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

		}else
			m_Position+=diff;
	}

	float fRotSpeed=Magnitude(m_Velocity)*0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponGrenade::Explosion()
{
	// 자신의 이펙트 그리고..
	// 주변 유닛들에게 데미지 주는 메시지 날리고..
	// ( 메시지 받은곳에서 주변 유닛들 체크하고.. 
	// 죽은 유닛이 있다면 발사유닛의 킬수를 올려주기)

	rvector v = m_Position;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	ZGetEffectManager()->AddGrenadeEffect(v,dir);

	// 메시지를 날리거나~
	// 각각의 클라이언트의 무기 정보를 믿고 처리하거나~

	ZGetGame()->OnExplosionGrenade(m_uidOwner,v,m_fDamage,400.f,.2f,1.f,m_nTeamID);

	ZGetSoundEngine()->PlaySound("we_grenade_explosion",v);

	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}

/////////////////////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponFlashBang,ZWeaponGrenade);

#define FLASHBANG_LIFE	3	//<<- TODO : 무기의 속성에서 받아오기
#define FLASHBANG_DISTANCE	2000		// 20미터 까지 영향을 줌

void ZWeaponFlashBang::Explosion()
{
	rvector dir		= RCameraPosition - m_Position;
	float dist		= Magnitude( dir );
	D3DXVec3Normalize( &dir, &dir );
	RBSPPICKINFO pick;

	if( dist > FLASHBANG_DISTANCE )	// 거리 밖에서는 영향을 주지 못함
	{
		return;
	}

	if( ZGetGame()->m_pMyCharacter->IsDie() )	// 옵저버 모드에서는 영향을 받지 않음
	{
		return;
	}

	if( !ZGetGame()->GetWorld()->GetBsp()->Pick( m_Position, dir, &pick ) )
	{
		mbIsLineOfSight	= true;
	}
	else
	{
		float distMap	= D3DXVec3LengthSq( &(pick.PickPos - m_Position) );
		rvector temp = ZGetGame()->m_pMyCharacter->GetPosition() - m_Position;
		float distChar	= D3DXVec3LengthSq( &(temp) );
		if( distMap > distChar )
		{
			mbIsLineOfSight	= true;
		}
		else
		{
			mbIsLineOfSight	= false;
		}
	}

	if( !mbIsExplosion && mbIsLineOfSight )
	{
		rvector pos		= ZGetGame()->m_pMyCharacter->GetPosition();
		rvector dir		= ZGetGame()->m_pMyCharacter->m_TargetDir;
		mbIsExplosion	= true;
		CreateFlashBangEffect( m_Position, pos, dir, 10 );
	}
	ZGetSoundEngine()->PlaySound("we_flashbang_explosion",m_Position);
}

bool	ZWeaponFlashBang::Update( float fElapsedTime )
{
	rvector oldPos = m_Position;

	float lap	= ZGetGame()->GetTime() - m_fStartTime;

	if( lap >= FLASHBANG_LIFE )
	{
		Explosion();
		return false;
	}

	m_Velocity.z	-= 1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff	= m_Velocity * fElapsedTime;
		rvector dir		= diff;
		Normalize( dir );

		float fDist		= Magnitude( diff );

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked	= ZGetGame()->Pick( ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag );

		if( bPicked )
		{
			if( zpi.bBspPicked )
			{
				pickpos			= zpi.bpi.PickPos;
				rplane plane	= zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal			= rvector( plane.a, plane.b, plane.c );
			}
			else if( zpi.pObject )
			{
				pickpos			= zpi.info.vOut;
				if( zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f )
				{
					normal		= pickpos-zpi.pObject->GetPosition();
					normal.z	= 0;
				}
				else
				{
					normal		= pickpos - (zpi.pObject->GetPosition()+rvector(0,0,90));
				}
				Normalize(normal);
			}

			pickpos		+= normal * BOUND_EPSILON;
		}

		if( bPicked && fabsf( Magnitude(pickpos-m_Position) ) < (fDist + BOUND_EPSILON) )
		{
			m_Position	= pickpos + normal;
			m_Velocity	= GetReflectionVector( m_Velocity, normal );
			m_Velocity	*= zpi.pObject ? 0.4f : 0.8f;

			Normalize( normal );
			float fAbsorb	= DotProduct( normal, m_Velocity );
			m_Velocity		-= 0.5 * fAbsorb * normal;

			float fA	= RANDOMFLOAT * 2 * pi;
			float fB	= RANDOMFLOAT * 2 * pi;
			m_RotAxis	= rvector( sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB) );

		}
		else
		{
			m_Position	+= diff;
		}
	}

	rmatrix Mat;

	if( !mbLand )
	{
		mRotVelocity	= min( Magnitude( m_Velocity ), MAX_ROT_VELOCITY );

		if( Magnitude(m_Velocity) < LANDING_VELOCITY )
		{
			mbLand	= true;
			rvector	right;
			m_Up	= rvector( 0, 1, 0 );
			D3DXVec3Cross( &right, &m_Dir, &m_Up );
			D3DXVec3Cross( &m_Dir, &right, &m_Up );
			D3DXMatrixIdentity( &mRotMatrix );
		}
		else
		{
			rmatrix	Temp;
			D3DXMatrixRotationAxis( &Temp, &m_RotAxis, mRotVelocity * 0.001 );
			mRotMatrix	= mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp;
		D3DXMatrixRotationX( &Temp, mRotVelocity * 0.001 );
		mRotMatrix	= mRotMatrix * Temp;
		mRotVelocity	*= 0.97;
	}

	MakeWorldMatrix( &Mat, m_Position, m_Dir, m_Up );
	Mat		= mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix( Mat );

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position	, ROCKET_SPEAR_EMBLEM_POWER );

	return true;
}

//////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponSmokeGrenade, ZWeaponGrenade);

#define SMOKE_GRENADE_LIFETIME 30
#define SMOKE_GRENADE_EXPLOSION	3

const float ZWeaponSmokeGrenade::mcfTrigerTimeList[NUM_SMOKE] =
{
	0, 0.5, 1, 1.7, 2.3, 2.5, 3
};

bool ZWeaponSmokeGrenade::Update( float fElapsedTime )
{
	rvector oldPos = m_Position;
	float lap	= ZGetGame()->GetTime() - m_fStartTime;

	if( lap >= SMOKE_GRENADE_LIFETIME )
	{
		return false;
	}
	
	if( miSmokeIndex < NUM_SMOKE && lap - SMOKE_GRENADE_EXPLOSION >= mcfTrigerTimeList[miSmokeIndex] )
	{
		Explosion();
		++miSmokeIndex;
	}

	m_Velocity.z	-= 1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff	= m_Velocity * fElapsedTime;
		rvector dir		= diff;
		Normalize( dir );

		float fDist		= Magnitude( diff );

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked	= ZGetGame()->Pick( ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag );
		
		if( bPicked )
		{
			if( zpi.bBspPicked )
			{
				pickpos			= zpi.bpi.PickPos;
				rplane plane	= zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal			= rvector( plane.a, plane.b, plane.c );
			}
			else if( zpi.pObject )
			{
				pickpos			= zpi.info.vOut;
				if( zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f )
				{
					normal		= pickpos-zpi.pObject->GetPosition();
					normal.z	= 0;
				}
				else
				{
					normal		= pickpos - (zpi.pObject->GetPosition()+rvector(0,0,90));
				}
				Normalize(normal);
			}

			pickpos		+= normal * BOUND_EPSILON;
		}

		if( bPicked && fabsf( Magnitude(pickpos-m_Position) ) < (fDist + BOUND_EPSILON) )
		{
			m_Position	= pickpos + normal;
			m_Velocity	= GetReflectionVector( m_Velocity, normal );
			m_Velocity	*= zpi.pObject ? 0.4f : 0.8f;

			Normalize( normal );
			float fAbsorb	= DotProduct( normal, m_Velocity );
			m_Velocity		-= 0.5 * fAbsorb * normal;

			float fA	= RANDOMFLOAT * 2 * pi;
			float fB	= RANDOMFLOAT * 2 * pi;
			m_RotAxis	= rvector( sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB) );

		}
		else
		{
			m_Position	+= diff;
		}
	}

	rmatrix Mat;

	if( !mbLand )
	{
 		mRotVelocity	= min( Magnitude( m_Velocity ), MAX_ROT_VELOCITY );

		if( Magnitude(m_Velocity) < LANDING_VELOCITY )
		{
			mbLand	= true;
			rvector	right;
			m_Up	= rvector( 0, 1, 0 );
			D3DXVec3Cross( &right, &m_Dir, &m_Up );
			D3DXVec3Cross( &m_Dir, &right, &m_Up );
			D3DXMatrixIdentity( &mRotMatrix );
		}
		else
		{
			rmatrix	Temp;
			D3DXMatrixRotationAxis( &Temp, &m_RotAxis, mRotVelocity * 0.001 );
			mRotMatrix	= mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp;
		D3DXMatrixRotationX( &Temp, mRotVelocity * 0.001 );
		mRotMatrix	= mRotMatrix * Temp;
 		mRotVelocity	*= 0.97;
	}
	
	MakeWorldMatrix( &Mat, m_Position, m_Dir, m_Up );
	Mat		= mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix( Mat );
	
	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponSmokeGrenade::Explosion()
{
	ZGetEffectManager()->AddSmokeGrenadeEffect( m_Position );
	mRotVelocity	*= 10;

	ZGetSoundEngine()->PlaySound("we_gasgrenade_explosion",m_Position);
}

/////////////////////////// 마법 미사일류..

MImplementRTTI(ZWeaponMagic, ZMovingWeapon);

void ZWeaponMagic::Create(RMesh* pMesh, ZSkill* pSkill, const rvector &pos, const rvector &dir, float fMagicScale,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_fMagicScale = fMagicScale;

	m_pVMesh->SetAnimation("play");

	m_pSkillDesc = pSkill->GetDesc();

	m_Position=pos;

	if (m_pSkillDesc) m_Velocity=dir * m_pSkillDesc->fVelocity;
	else m_Velocity=dir*ROCKET_VELOCITY;
	

	m_fStartTime = ZGetGame()->GetTime();
	m_fLastAddTime = ZGetGame()->GetTime();

	m_Dir=dir;
	m_Up=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	m_fDamage = pSkill->GetDesc()->nModDamage;

	m_uidTarget = pSkill->GetTarget();
	m_bGuide = pSkill->GetDesc()->bGuidable;

	m_bThroughNPC = pSkill->GetDesc()->bThroughNPC;
	m_bSurfaceMount = pSkill->GetDesc()->bSurfaceMount;
	
	if( Z_VIDEO_DYNAMICLIGHT && !pSkill->GetDesc()->bOffDLight) {
		//_ASSERT( m_SLSid == 0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}

	//ZGetEffectManager()->AddRocketSmokeEffect(m_Position,-RealSpace2::RCameraDirection);
}

#define MAGIC_WEAPON_LIFE			10.f		// 10초 뒤에 터짐


bool ZWeaponMagic::Update(float fElapsedTime)
{
	// 유도처리
	if(m_bGuide) {
		ZObject *pTarget = ZGetObjectManager()->GetObject(m_uidTarget);
		if (pTarget) {

			float fCurrentSpeed = Magnitude(m_Velocity);
			rvector currentDir = m_Velocity;
			Normalize(currentDir);

			rvector dir = (pTarget->GetPosition()+rvector(0,0,100)) - m_Position;

			if ( m_bSurfaceMount)
				dir.z = 0;

			Normalize(dir);

			float fCos = DotProduct(dir,currentDir);
			float fAngle = acos(fCos);
			if(fAngle>0.01f) {

		#define ANGULAR_VELOCITY	0.01f		// 각속도 : 초당 회전가능한 각의 크기. (단위:라디안)
				float fAngleDiff = min(1000.f*fElapsedTime*ANGULAR_VELOCITY,fAngle);

				rvector newDir = InterpolatedVector(m_Dir,dir,fAngleDiff/fAngle);
				m_Dir = newDir;

				m_Velocity = fCurrentSpeed * newDir;
				//_ASSERT(!_isnan(m_Velocity.x) &&!_isnan(m_Velocity.y) && !_isnan(m_Velocity.z) );
			}
		}
	}

	rvector oldPos = m_Position;

	if(ZGetGame()->GetTime() - m_fStartTime > MAGIC_WEAPON_LIFE ) {
		// 수류탄은 터지는 순간에 이펙트 추가.. 삭제...
		Explosion( WMET_MAP, NULL , rvector(0,1,0));

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}

		return false;
	}

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos;
		rvector pickdir;

		ZPICKINFO zpi;
		WeaponMagicExplosionType type = WMET_MAP;

		ZObject* pOwnerObject = ZGetObjectManager()->GetObject(m_uidOwner);
		ZObject* pPickObject = NULL;

		bool bPicked=ZGetGame()->Pick(pOwnerObject, m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked) {
				pickpos=zpi.bpi.PickPos;
				pickdir.x=zpi.bpi.pInfo->plane.a;
				pickdir.y=zpi.bpi.pInfo->plane.b;			
				pickdir.z=zpi.bpi.pInfo->plane.c;
				Normalize(pickdir);
			}	
			else
				if(zpi.pObject) {
					pPickObject = zpi.pObject;
					pickpos=zpi.info.vOut;
					type = WMET_OBJECT;
				}

		}


		if( bPicked && fabsf( Magnitude(pickpos-m_Position)) < fDist)
		{
			// 통과하는 종류라면...
			bool bCheck = true;
			if ( m_bThroughNPC && pPickObject && pPickObject->IsNPC())
				bCheck = false;

			if ( bCheck)
			{
				Explosion( type, pPickObject,pickdir);

				if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
				{
					ZGetStencilLight()->DeleteLightSource( m_SLSid );
					m_SLSid = 0;
					ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
				}

				return false;
			}
		}

//		else
		{
			rvector to = m_Position + diff;

			if ( ZGetGame()->ObjectColTest(pOwnerObject, m_Position, to, m_pSkillDesc->fColRadius, &pPickObject))
			{
				// 통과하는 종류라면...
				bool bCheck = true;
				if ( m_bThroughNPC && pPickObject && pPickObject->IsNPC())
					bCheck = false;

				if ( bCheck)
				{
					pickdir = to - m_Position;
					Normalize(pickdir);

					ExplosionThrow( WMET_OBJECT, pPickObject,pickdir);

					if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
					{
						ZGetStencilLight()->DeleteLightSource( m_SLSid );
						m_SLSid = 0;
						ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
					}
					
					return false;
				}
			}
//			else
			{
				m_Position+=diff;
			}
		}
	}

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

//	mat._12 *= m_fMagicScale;
//	mat._23 *= m_fMagicScale;
//	mat._31 *= m_fMagicScale;

	m_pVMesh->SetScale(rvector(m_fMagicScale,m_fMagicScale,m_fMagicScale));
	m_pVMesh->SetWorldMatrix(mat);

	float this_time = ZGetGame()->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

#define _ROCKET_RAND_CAP 10

		/*
		rvector add;

		add.x = rand()%_ROCKET_RAND_CAP;
		add.y = rand()%_ROCKET_RAND_CAP;
		add.z = rand()%_ROCKET_RAND_CAP;

		if(rand()%2) add.x=-add.x;
		if(rand()%2) add.y=-add.y;
		if(rand()%2) add.z=-add.z;

		rvector pos = m_Position+add;
		*/

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

//		case type

		ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

		nEffectType = m_pSkillDesc->nTrailEffectType;

		if (m_pSkillDesc->bDrawTrack)
		{
				if(nEffectType==ZSTE_FIRE)		ZGetEffectManager()->AddTrackFire(pos);
			else if(nEffectType==ZSTE_COLD)		ZGetEffectManager()->AddTrackCold(pos);
			else if(nEffectType==ZSTE_MAGIC)	ZGetEffectManager()->AddTrackMagic(pos);
		}

//		ZGetEffectManager()->AddRocketSmokeEffect(pos);
//		ZGetEffectManager()->AddSmokeEffect(NULL,pos,rvector(0,0,0),rvector(0,0,0),60.f,80.f,1.5f);
		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);

	return true;
}

void ZWeaponMagic::Render()
{
	ZWeapon::Render();


	// test 코드
#ifndef _PUBLISH
	if ((ZApplication::GetGameInterface()->GetScreenDebugger()->IsVisible()) && (ZIsLaunchDevelop()))
	{
		if (m_pSkillDesc)
		{
			RDrawSphere(m_Position, m_pSkillDesc->fColRadius, 10);
		}
	}
#endif

}



void ZWeaponMagic::Explosion(WeaponMagicExplosionType type, ZObject* pVictim,rvector& vDir)
{
	rvector v = m_Position-rvector(0,0,100.f);

#define MAGIC_MINIMUM_DAMAGE	0.2f
#define MAGIC_KNOCKBACK_CONST   .3f	

	if (!m_pSkillDesc->IsAreaTarget())
	{
		if ((pVictim) && (type == WMET_OBJECT))
		{
			ZGetGame()->OnExplosionMagicNonSplash(this, m_uidOwner, pVictim->GetUID(), v, m_pSkillDesc->fModKnockback);
		}
	}
	else
	{
		ZGetGame()->OnExplosionMagic(this,m_uidOwner,v,MAGIC_MINIMUM_DAMAGE,m_pSkillDesc->fModKnockback, m_nTeamID, true);
	}

	ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

	nEffectType = m_pSkillDesc->nTrailEffectType;

	rvector pos = m_Position;

	if(type == WMET_OBJECT) {// object 에 부딪히는 경우...
		float fScale = m_fMagicScale;

		// 이펙트는 스케일링하면 줄인다.
		/*		if(nEffectType==ZSTE_FIRE)			
		{
		if (fScale > 1.0f)
		{
		fScale = 1.0f + fScale*0.2f;
		pos -= rvector(0.0f, 0.0f, fScale * 100.0f);
		}

		ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_FIRE,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_COLD)		
		{
		ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_COLD,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_MAGIC)	
		{
		ZGetEffectManager()->AddMagicEffect(m_Position,0, fScale);
		}
		*/

		// 이펙트 사운드 추가
		if ( m_pSkillDesc->szExplosionSound[ 0])
		{
			rvector soundPos = pVictim->GetPosition();
			ZGetSoundEngine()->PlaySound( m_pSkillDesc->szExplosionSound, soundPos);
		}


		// 상대 타깃에 인첸트 이펙트 추가
		switch ( m_pSkillDesc->ResistType)
		{
		case ZSR_FIRE :
			{
				ZModule_FireDamage *pMod = (ZModule_FireDamage*)pVictim->GetModule( ZMID_FIREDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}

		case ZSR_COLD :
			{
				ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pVictim->GetModule( ZMID_COLDDAMAGE);
				if ( pMod)  pMod->BeginDamage( 1.0f, 0.9f);

				break;
			}

		case ZSR_LIGHTNING :
			{
				//				ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pVictim->GetModule( ZMID_LIGHTNINGDAMAGE);
				//				if ( pMod)  pMod->BeginDamage( MUID(0,0), 0, 0.9f);

				break;
			}

		case ZSR_POISON :
			{
				ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pVictim->GetModule( ZMID_POISONDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}
		}
	}


	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}



void ZWeaponMagic::ExplosionThrow(WeaponMagicExplosionType type, ZObject* pVictim,rvector& vDir)
{
	rvector v = m_Position-rvector(0,0,100.f);

#define MAGIC_MINIMUM_DAMAGE	0.2f
#define MAGIC_KNOCKBACK_CONST   .3f	

	if (!m_pSkillDesc->IsAreaTarget())
	{
		if ((pVictim) && (type == WMET_OBJECT))
		{
			ZGetGame()->OnExplosionMagicNonSplash(this, m_uidOwner, pVictim->GetUID(), v, m_pSkillDesc->fModKnockback);
		}
	}
	else
	{
		ZGetGame()->OnExplosionMagicThrow(this,m_uidOwner,v,MAGIC_MINIMUM_DAMAGE,m_pSkillDesc->fModKnockback, m_nTeamID, true, m_Position, vDir + m_Position);
	}

	ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

	nEffectType = m_pSkillDesc->nTrailEffectType;

	rvector pos = m_Position;

	if(type == WMET_OBJECT) {// object 에 부딪히는 경우...
		float fScale = m_fMagicScale;
		
		// 이펙트는 스케일링하면 줄인다.
/*		if(nEffectType==ZSTE_FIRE)			
		{
			if (fScale > 1.0f)
			{
				fScale = 1.0f + fScale*0.2f;
				pos -= rvector(0.0f, 0.0f, fScale * 100.0f);
			}

			ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_FIRE,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_COLD)		
		{
			ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_COLD,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_MAGIC)	
		{
			ZGetEffectManager()->AddMagicEffect(m_Position,0, fScale);
		}
*/

		// 이펙트 사운드 추가
		if ( m_pSkillDesc->szExplosionSound[ 0])
		{
			rvector soundPos = pVictim->GetPosition();
			ZGetSoundEngine()->PlaySound( m_pSkillDesc->szExplosionSound, soundPos);
		}


		// 상대 타깃에 인첸트 이펙트 추가
		switch ( m_pSkillDesc->ResistType)
		{
			case ZSR_FIRE :
			{
				ZModule_FireDamage *pMod = (ZModule_FireDamage*)pVictim->GetModule( ZMID_FIREDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}

			case ZSR_COLD :
			{
				ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pVictim->GetModule( ZMID_COLDDAMAGE);
				if ( pMod)  pMod->BeginDamage( 1.0f, 0.9f);

				break;
			}

			case ZSR_LIGHTNING :
			{
//				ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pVictim->GetModule( ZMID_LIGHTNINGDAMAGE);
//				if ( pMod)  pMod->BeginDamage( MUID(0,0), 0, 0.9f);

				break;
			}

			case ZSR_POISON :
			{
				ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pVictim->GetModule( ZMID_POISONDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}
		}
	}


	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}

////////////////////////// 다이나마이트

MImplementRTTI(ZWeaponDynamite, ZMovingWeapon);

void ZWeaponDynamite::Create(RMesh* pMesh, rvector &pos, rvector &velocity, ZObject* pOwner) 
{
	ZWeapon::Create(pMesh);
	
	rvector dir = velocity;
	Normalize(dir);

	m_Velocity = velocity;
	m_Position = pos;

	m_bCollided = false;
	m_fStartTime = 0.f;

	m_Dir		= rvector(1,0,0);
	m_Up		= rvector(0,0,1);
	m_RotAxis	= rvector(0,0,1);

	m_uidOwner	= pOwner->GetUID();
	m_nTeamID	= pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() ) {
		if( pOwner->GetItems()->GetSelectedWeapon() ) {
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();
		}
	}

	if (pDesc == NULL) {
		//_ASSERT(0);
		return;
	}

	m_fDamage = pDesc->m_nDamage.Ref();
	m_nSoundCount = (rand() % 2) + 2;
}

#define DYNAMITE_LIFE 0.7f	// 첫번째 충돌 후 0.7초후에 터짐

bool ZWeaponDynamite::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	if (m_Position.z < DIE_CRITICAL_LINE)
		return false;

	// 처음으로 부딪힌 때부터의 시간을 체크하여 폭발
	if( m_bCollided &&
		ZGetGame()->GetTime() - m_fStartTime > DYNAMITE_LIFE) {
		Explosion();
		if(Z_VIDEO_DYNAMICLIGHT)
			ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );
		return false;
	}

	m_Velocity.z = m_Velocity.z - (1000.f * fElapsedTime);

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff = m_Velocity * fElapsedTime;
		rvector dir	= diff;
		Normalize(dir);

		float fDist = Magnitude(diff);

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos = zpi.bpi.PickPos;
				rplane plane = zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal = rvector(plane.a, plane.b, plane.c);
			}
			else 
			{
				if(zpi.pObject)
				{
					pickpos = zpi.info.vOut;
					if(zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f)
					{
						normal = pickpos-zpi.pObject->GetPosition();
						normal.z = 0;
					}
					else 
					{
						normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
					}

					Normalize(normal);
				}
			}
		}

		// 어딘가에 충돌했다
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			// 처음으로 어딘가 부딪혔을 때부터 시간을 재기 시작
			if (!m_bCollided)
			{
				m_bCollided = true;
				m_fStartTime = ZGetGame()->GetTime();
			}

			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;		// 캐릭터에 충돌하면 속도가 많이 준다

			// 소리를 낸다 테스트
			if(zpi.bBspPicked && m_nSoundCount>0) {
				m_nSoundCount--;
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			// 벽방향의 속도성분을 줄인다.
			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA=RANDOMFLOAT*2*pi;
			float fB=RANDOMFLOAT*2*pi;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

		}
		else
			m_Position+=diff;
	}

	float fRotSpeed = Magnitude(m_Velocity) * 0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );
	return true;
}

void ZWeaponDynamite::Explosion()
{
	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;

	ZGetEffectManager()->AddDynamiteEffect(m_Position, dir);
	ZGetSoundEngine()->PlaySound("we_grenade_explosion", m_Position);

	ZGetGame()->OnExplosionDynamite(m_uidOwner, m_Position, m_fDamage, 400.f, 1.f, m_nTeamID);
	ZGetWorld()->GetFlags()->SetExplosion( m_Position, EXPLOSION_EMBLEM_POWER );
}


MImplementRTTI(ZWeaponTrap, ZMovingWeapon);

bool ZWeaponTrap::Init(RMesh* pMesh, rvector& pos, rvector& velocity, int nItemId, ZObject* pOwner)
{
	ZMovingWeapon::Create(pMesh);

	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (!pDesc) return false;

	m_fDamage=pDesc->m_nDamage.Ref();
	m_nItemId = nItemId;

	m_Position=pos;
	m_Velocity=velocity;

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	m_fThrowedTime = ZGetGame()->GetTime();

	m_bLanded = false;
	m_fLandedTime = 0;

	m_bActivated = false;
	m_fActivatedTime = 0;
	m_fNextEffectTime = 0;

	m_bFriendly = false;
	ZObject* pMyChar = ZGetCharacterManager()->Find(ZGetMyUID());
	if (!ZGetGame()->CanAttack(pOwner, pMyChar))
		m_bFriendly = true;
	else if (pOwner && pOwner->GetUID()==ZGetMyUID())
		m_bFriendly = true;

	m_nDamageType = pDesc->m_nDamageType.Ref();
	
	return true;
}

bool ZWeaponTrap::Create(RMesh* pMesh, rvector& pos, rvector& velocity, int nItemId, ZObject* pOwner)
{
	if (!Init(pMesh, pos, velocity, nItemId, pOwner)) return false;

	// 정말 해당 아이템을 장비하고 있는가
	if (pOwner->GetItems())
	{
		bool bEquiped = false;
		for (int i=MMCIP_CUSTOM1; i<=MMCIP_CUSTOM2; ++i)
		{
			ZItem* pItem = pOwner->GetItems()->GetItem((MMatchCharItemParts)i);
			if (pItem->GetDesc() && pItem->GetDesc()->m_nID == nItemId)
				bEquiped = true;
		}

		if (!bEquiped) return false;
	}

	// 서버에 트랩을 던진다는 것을 보낸다. 서버는 뭔가 던져졌다는 것만 기억해두었다가 
	// 이후에 트랩의 발동 통보를 다시 받을때 아 아까 던져진 애가 어디에서 발동됐구나 하고 체크하면 된다.
	// 어떤 트랩이 던져졌으나 아직 발동되지 않은 상태일 때 난입한 유저에게는 이 발동된 트랩에 대해서 서버가 별도로 메시지를 보내준다
	if (m_uidOwner == ZGetMyUID())
		ZPostNotifyThrowedTrap(m_nItemId);
	
	return true;
}

bool ZWeaponTrap::CreateActivated( RMesh* pMesh, rvector& pos, float fActivatedTime, int nItemId, ZObject* pOwner )
{
	if (!Init(pMesh, pos, rvector(0,0,0), nItemId, pOwner))
		return false;

	m_fActivatedTime = fActivatedTime;
	m_fNextEffectTime = m_fActivatedTime;
	m_bActivated = true;

	rmatrix mat;
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);
	m_pVMesh->SetWorldMatrix(mat);

	float fElapsedTime = ZGetGame()->GetTime() - m_fActivatedTime;
	AddEffectActivating(fElapsedTime * 1000, m_bFriendly);

	return true;
}

#define TRAP_LANDING_TIME 0.7f		// 땅에 닿은 후 이 시간동안 가만히 기다린다

bool ZWeaponTrap::Update( float fElapsedTime )
{
	if (m_bActivated)
		return UpdateOnActivated(fElapsedTime);
	
	if (m_bLanded)
	{
		if (ZGetGame()->GetTime() - m_fLandedTime > TRAP_LANDING_TIME)
		{
			Activate();
		}
		return true;
	}

	if (m_Position.z < DIE_CRITICAL_LINE)
		return false;

	if (ZGetGame()->GetTime() - m_fThrowedTime > MAX_TRAP_THROWING_LIFE)
		return false;

	rvector oldPos = m_Position;

	m_Velocity.z-=1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos,normal;

		ZPICKINFO zpi;
		bool bPicked=ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos=zpi.bpi.PickPos;
				rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal=rvector(plane.a,plane.b,plane.c);
			}
			else if(zpi.pObject)
			{
				pickpos=zpi.info.vOut;
				if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
				{
					normal=pickpos-zpi.pObject->GetPosition();
					normal.z=0;
				}
				else
					normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
				Normalize(normal);
			}
		}


		// 어딘가에 충돌했다
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;		// 캐릭터에 충돌하면 속도가 많이 준다

			if(zpi.bBspPicked) {
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			// 벽방향의 속도성분을 줄인다.
			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA=RANDOMFLOAT*2*pi;
			float fB=RANDOMFLOAT*2*pi;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

			// 지면에 닿았을 때 일정시간 후 폭발 (벽이나 천장 충돌은 패스)
			if(DotProduct(normal, rvector(0,0,1)) > 0.8f)
			{
				if (!m_bLanded)
				{
					m_bLanded = true;
					m_fLandedTime = ZGetGame()->GetTime();
					
					// 트랩이 설치된 위치를 알 수 있게 가이드를 조그맣게 표시하자
					ZGetEffectManager()->AddTrapGuideEffect(m_Position, rvector(0,1,0), int(TRAP_LANDING_TIME*1000), m_bFriendly, 0.1f, 12.f);
				}
				return true;
			}

		}else
			m_Position+=diff;
	}

	float fRotSpeed=Magnitude(m_Velocity)*0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponTrap::Activate()
{
	m_bActivated = true;
	m_fActivatedTime = ZGetGame()->GetTime();
	m_fNextEffectTime = m_fActivatedTime;
	
	AddEffectActivating(0, m_bFriendly);

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );

	// 서버에 발동된 트랩을 등록해서 이후 난입하는 유저가 월드에 설치된 트랩들을 알 수 있도록 하자
	if (m_uidOwner == ZGetMyUID())
		ZPostNotifyActivatedTrap(m_nItemId, m_Position);
}

void ZWeaponTrap::AddEffectActivating(int nElapsedIfActivated, bool bFriendly)
{
	// 발동이 지속될 동안 보여질 이펙트 세팅
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemId);
	if (!pItemDesc) return;
	int nLifeTime = pItemDesc->m_nLifeTime.Ref();

	ZGetEffectManager()->AddTrapGuideEffect(m_Position, rvector(0,1,0), nLifeTime - nElapsedIfActivated, bFriendly);
}

bool ZWeaponTrap::UpdateOnActivated( float fElapsedTime )
{
	if (m_bActivated)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemId);
		if (!pItemDesc) return false;

		float fCurrTime = ZGetGame()->GetTime();
		if (fCurrTime - m_fActivatedTime > pItemDesc->m_nLifeTime.Ref()*0.001f)
			return false;

		if (fCurrTime > m_fNextEffectTime)
		{
			m_fNextEffectTime += 1.f;	// 1초마다 이펙트를 출력한다
			switch (m_nDamageType)
			{
			case MMDT_FIRE:
				ZGetEffectManager()->AddTrapFireEffect(m_Position, rvector(0,1,0));
				break;
			case MMDT_COLD:
				ZGetEffectManager()->AddTrapColdEffect(m_Position, rvector(0,1,0));
				break;
			default:
				break;
			}
		}

		ZGetGame()->CheckZoneTrap(m_uidOwner,m_Position,pItemDesc,m_nTeamID);
		return true;
	}
	return true;
}