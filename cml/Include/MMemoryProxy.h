#pragma once

#include "MMath.h"
#include "MCRC32.h"

class MCrc32Container
{
	std::map<void*, DWORD> m_mapCrc32;
	typedef std::map<void*, DWORD>::iterator Iterator;

public:
	void Add(void* key, DWORD crc)
	{
		m_mapCrc32[key] = crc;
	}
	void Remove(void* key)
	{
		m_mapCrc32.erase(key);
	}
	DWORD Get(void* key)
	{
		Iterator it = m_mapCrc32.find(key);
		if (it!=m_mapCrc32.end())
			return it->second;
		return 0;
	}
};

extern void (*g_fpOnCrcFail)();	// crc체크 실패시(해킹시) 호출할 함수 포인터

MCrc32Container* GetCrcContainer();


#define PTR_OFFSET 0xD6
// 참고로 한국 핵유저들이 보통 검색해보는 힙 범위는 0x15000000 ~ 0x30000000 정도
// (↑RandMaskVal.exe에 의해서 빌드 전에 변경되는 값. define 명을 수정하면 파싱 에러가 납니다.)

template <typename T>
class MProtectValue
{
	// 값을 할당한 포인터 주소값에도 마스킹값을 얹어 주소자체를 숨긴다
	typedef unsigned char* MaskedPtr;
	MaskedPtr m_pValue;

private:
	MaskedPtr	ToMaskedPtr(T* ptr) const			{ return MaskedPtr(ptr) + (PtrToUlong(this)+PTR_OFFSET); }
	T*			ToNormalPtr(MaskedPtr mptr) const	{ return (T*)(mptr - (PtrToUlong(this)+PTR_OFFSET)); }

	DWORD BuildCrc()
	{
		BYTE* pData = (BYTE*)ToNormalPtr(m_pValue);
		return MCRC32::BuildCRC32(pData, sizeof(T));
	}

public:
	MProtectValue()		 { m_pValue = ToMaskedPtr(new T); }

	~MProtectValue()	 { 
		delete ToNormalPtr(m_pValue);
		::GetCrcContainer()->Remove(this);
	}

	// 값을 레퍼런스로 얻는다
			T& Ref()		{ return *ToNormalPtr(m_pValue); }
	const	T& Ref() const	{ return *ToNormalPtr(m_pValue); }

	// 값 배정
	void Set(const T& newVal)		{ Ref() = newVal; }

	// 값 배정하면서 crc검사
	bool Set_CheckCrc(const T& newVal) {
		CheckCrc();
		Set(newVal);
		MakeCrc();
		return true;
	}
	// 값 배정하면서 crc 생성 (최초 값 배정용)
	void Set_MakeCrc(const T& newVal) {
		Set(newVal);
		MakeCrc();
	}

	// Set_CheckCrc()와 같다. 다만 T가 구조체일 경우 crc를 체크하며 멤버를 갱신하기 쉽게 하는 매크로 함수
#define MEMBER_SET_CHECKCRC(PROTECTVALUE, MEMBER, NEWVAL) { \
		PROTECTVALUE.CheckCrc(); \
		PROTECTVALUE.Ref().MEMBER = (NEWVAL); \
		PROTECTVALUE.MakeCrc(); \
	}

	// 재할당->복사를 통해 힙위치를 변경한다
	void ShiftHeapPos()
	{
		T* p = ToNormalPtr(m_pValue);
		m_pValue = ToMaskedPtr(new T);
		Ref() = *p;
		delete p;
	}

	void ShiftHeapPos_CheckCrc() { CheckCrc(); ShiftHeapPos(); }

	void MakeCrc()
	{
		DWORD crc = BuildCrc();
		::GetCrcContainer()->Add(this, crc);
	}

	// crc를 검사한다
	void CheckCrc()
	{
		if (BuildCrc() != ::GetCrcContainer()->Get(this))
			g_fpOnCrcFail();
	}

private:
	// 연산자 오버로드를 쓰면 편하겠지만 실제로 무슨 일이 일어나는지 명시적으로 표현하도록 강제하는 편이 낫다고 생각함.

	MProtectValue(T val) { m_pValue = ToMaskedPtr(new T(val)); }	// 복사생성자..혼란스럽다 차라리 숨기자
	MProtectValue<T>& operator=(int) {}	// 복사대입 연산자를 숨김
};

// USAGE : 해킹을 막고자 하는 값을 이 템플릿으로 감싼다. 
// 이 템플릿은 두가지 수단으로 메모리핵을 방해할 수 있다.
// 주기적으로 힙위치를 옮겨서 변수를 특정값으로 freeze하는 것을 막아볼 수 있다.
// 그러나 이 템플릿 안에 있는 실제값 주소를 추적할 수도 있으므로 2차적으로 crc32검사를 할 수 있다.
// crc32를 사용하려면 값을 갱신하기 전에 기존값crc체크를 해야하고, 값을 넣은뒤엔 crc를 새로 빌드해야만 한다.
//
// 이 템플릿클래스로 감싸더라도 필요한 경우에만 핵방어 기능을 동작시킬 수 있다. 예를들어 ZObject의 멤버를 이걸로 감싸더라도
// 직접 핵방어 기능을 하는 함수를 호출하지 않으면 핵방어 효과는 없다. ZObject의 경우 ZMyCharacter와 NPC인 ZActor가 상속받는데
// NPC는 해킹대상이 아니므로 굳이 핵방어에 따르는 부하를 감수할 필요없이 ZMyCharacter만 핵방어용 함수를 호출하면 된다.


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 이하의 템플릿들은 과거에 핵방어로 사용하던 것임
// MMemoryBlock은 뚫렸다고 함..
// MMemoryFugitive은 힙위치를 잘 옮기지만 각 경우에 따른 선택적 적용과 체크섬을 병행하기 곤란하다
/*


#define WRAPING_TIME 1000	//메모리 블럭을 바꿔주는 시간...1초에 한번씩...
#define NUM_OF_MEMORY 20	//하나의 메모리 블럭에 잡히는 메모리 개수..

template <typename _data>
struct MMemoryBlock
{
	_data * m_pMemory[NUM_OF_MEMORY];
};

template <typename _data>
class MMemoryProxy
{
	MMemoryBlock<_data> * MB1;
	MMemoryBlock<_data> * MB2;
	MMemoryBlock<_data> * MB3;

	MMemoryBlock<_data> * MB;

	_data m_initial;

	DWORD m_dwCurrentTime;
	int m_nCorrectDataIndex;
	int m_nIndex;
	
public:
	MMemoryProxy()
	{
		Init();
	}

	MMemoryProxy(_data init)
	{
		m_initial = init;
		m_nIndex = 0;
		m_nCorrectDataIndex = 0;
		Init();
	}

	~MMemoryProxy()
	{
		Destroy();
	}

	void Init()
	{
		MB1 = new MMemoryBlock<_data>();
		MB2 = new MMemoryBlock<_data>();
		MB3 = new MMemoryBlock<_data>();
		
		for(int i=0; i<NUM_OF_MEMORY; i++)
		{
			MB1->m_pMemory[i] = new _data;
			*(MB1->m_pMemory[i]) = m_initial;

			MB2->m_pMemory[i] = new _data;
			*(MB2->m_pMemory[i]) = m_initial;

			MB3->m_pMemory[i] = new _data;
			*(MB3->m_pMemory[i]) = m_initial;
		}
		
		m_nIndex = 1;
		MB = MB1;
		m_dwCurrentTime = GetTickCount();

		SetRandomData( m_initial);
	}
	void Destroy()
	{
		for(int i=0; i<NUM_OF_MEMORY; i++)
		{
			delete MB1->m_pMemory[i];
			delete MB2->m_pMemory[i];
			delete MB3->m_pMemory[i];
		}

		delete MB1;
		delete MB2;
		delete MB3;
	}

	void SetData(const _data & data)
	{
		SetRandomData(data);
	}

	_data & GetData()
	{
		return * MB->m_pMemory[m_nCorrectDataIndex];
	}

	void SetRandomData(const _data & data)
	{
		m_nCorrectDataIndex = RandomNumber(0, NUM_OF_MEMORY-1);
		*(MB->m_pMemory[m_nCorrectDataIndex]) = data;
	}
	void SetWarpingAdd(DWORD tick)
	{
		if( tick - m_dwCurrentTime > WRAPING_TIME)
		{
			int i = RandomNumber(0, NUM_OF_MEMORY-1);
			if(m_nIndex == 1)
			{
				*(MB2->m_pMemory[i]) = *(MB1->m_pMemory[m_nCorrectDataIndex]);
				MB = MB2;
				*(MB1->m_pMemory[m_nCorrectDataIndex]) = m_initial;	//흔적 안남기기
				m_nCorrectDataIndex = i;
			}
			else if(m_nIndex == 2)
			{
				*(MB3->m_pMemory[i]) = *(MB2->m_pMemory[m_nCorrectDataIndex]);
				MB = MB3;
				*(MB2->m_pMemory[m_nCorrectDataIndex]) = m_initial;
				m_nCorrectDataIndex = i;
			}
			else
			{
				*(MB1->m_pMemory[i]) = *(MB3->m_pMemory[m_nCorrectDataIndex]);
				MB = MB1;
				*(MB3->m_pMemory[m_nCorrectDataIndex]) = m_initial;
				m_nCorrectDataIndex = i;
			}
			m_nIndex++;
			if(m_nIndex>3)
				m_nIndex = 1;

			m_dwCurrentTime = tick;
		}	
	}
};




template <typename _data>
class MMemoryFugitive
{
	_data* m_value;

public:
	MMemoryFugitive()
	{
		m_value = NULL;
	}

	MMemoryFugitive(_data data)
	{
		m_value = new _data;
		*m_value = data;
	}

	~MMemoryFugitive()
	{
		Destroy();
	}

	void Destroy()
	{
		delete m_value;
	}

	MMemoryFugitive& operator=(const _data& other)	
	{
		SetData(other);
		return *this;
	}

	MMemoryFugitive& operator+=(const _data &other)
	{
		_data sum = *m_value + other;
		SetData(sum);
		return *this;
	}
	MMemoryFugitive& operator-=(const _data &other)
	{
		_data sum = *m_value - other;
		SetData(sum);
		return *this;
	}
	MMemoryFugitive& operator*=(const _data &other)
	{
		_data sum = *m_value * other;
		SetData(sum);
		return *this;
	}

	void SetData(const _data & data)
	{
		_data* pTemp = NULL;

		if (m_value != NULL)
		{
			pTemp = m_value;
		}

		m_value = new _data;
		*m_value = data;

		if (pTemp != NULL)
		{
			delete pTemp;
		}
	}

	_data & GetData()
	{
		return *m_value ;
	}

	const _data & GetData() const
	{
		return *m_value ;
	}


	void ShiftManually()
	{
		if (m_value != NULL)
		{
			_data val_copy = GetData();
			SetData(val_copy);
		}
	}
};

*/