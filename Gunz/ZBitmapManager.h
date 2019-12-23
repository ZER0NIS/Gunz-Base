#pragma once

#include "Mint4R2.h"

// #비트맵 매니저
// MBitmapManager가 있긴 하지만 전역객체 형태로 사용되고 있습니다.
// 이 매니저는 별도의 비트맵 그룹을 다루고 싶을 때 인스턴싱해서 사용합니다.
// 아이템 썸네일 표시 업데이트 작업 중에,
// 필요한 썸네일 아이콘을 그때그때 로딩하고, 필요없어지면(ex;상점/장비창에서 나갈때) 전부 언로드하기 쉽도록
// 별도의 비전역적 매니저가 필요해져서 만들었습니다.

template <typename KEYTYPE>
class ZBitmapManager
{
//	typedef map<KEYTYPE, MBitmap*>	MapBitmap;
//	typedef MapBitmap::iterator		ItorBitmap;

	map<KEYTYPE, MBitmap*> m_mapBitmap;

public:
	ZBitmapManager() {}
	~ZBitmapManager()
	{
		Clear();
	}


	void Clear()
	{
		for (map<KEYTYPE, MBitmap*>::iterator it=m_mapBitmap.begin(); it!=m_mapBitmap.end(); ++it)
			delete it->second;
		m_mapBitmap.clear();
	}

	// 비트맵의 텍스처를 임시로 메모리에서 모두 내린다.
	// 비트맵 용량이 많은데, 항상 메모리에 로드 상태일 필요는 없는 것들을 가끔 정리해주는 용도
	void UnloadTextureTemporarily()
	{
		for (map<KEYTYPE, MBitmap*>::iterator it=m_mapBitmap.begin(); it!=m_mapBitmap.end(); ++it)
			((MBitmapR2*)(it->second))->UnloadTextureTemporarily();
	}

	bool Add(KEYTYPE key, MBitmap* pBitmap)
	{
		map<KEYTYPE, MBitmap*>::iterator it = m_mapBitmap.find(key);
		if (it == m_mapBitmap.end()) {
			m_mapBitmap[key] = pBitmap;
			return true;
		}
		return false;
	}

	MBitmap* Get(KEYTYPE key)
	{
		map<KEYTYPE, MBitmap*>::iterator it = m_mapBitmap.find(key);
		if (it != m_mapBitmap.end())
			return it->second;
		return NULL;
	}

	void Delete(KEYTYPE key)
	{
		map<KEYTYPE, MBitmap*>::iterator it = m_mapBitmap.find(key);
		if (it != m_mapBitmap.end()) {
			delete it->second;
			m_mapBitmap.erase(it);
		}
	}
};