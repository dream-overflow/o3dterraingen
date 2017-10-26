/*---------------------------------------------------------------------------------------
  Tools.h :created on 16/12/2003, 15h54
  -------------------------------------------------------------------------------------
  - Objective-3D is under copyright (c) 2006 Dream Overflow (see license.txt)
  - mailto:objective3d@free.fr
  - http://CLM.dreamoverflow.com
  -------------------------------------------------------------------------------------
  Brief: SmartPointer
---------------------------------------------------------------------------------------*/

#include <map>

/*---------------------------------------------------------------------------------------
  TEMPLATE class CLMSmartPointer
  -------------------------------------------------------------------------------------*/
template <class T>
class CLMSmartPointer
{
public:

	CLMSmartPointer() : m_pObject(NULL), m_pCounter(0) {}

	CLMSmartPointer(const CLMSmartPointer<T> & _which) :
		m_pObject(_which.m_pObject),
		m_pCounter(_which.m_pCounter)
	{
		if (m_pCounter != NULL) (*m_pCounter)++;
	}

	CLMSmartPointer(T * _object)
	{
		if (_object != NULL)
		{
			m_pObject = _object;
			m_pCounter = new O3D_UINT(1);
		}
		else
		{
			m_pObject = NULL;
			m_pCounter = NULL;
		}
	}

	~CLMSmartPointer()
	{
		Release();
	}

	CLMSmartPointer<T> & operator=(const CLMSmartPointer<T> & _which)
	{
		if ((this == &_which) && (this->m_pObject == _which.m_pObject))
			return *this;

		Release();

		m_pObject = _which.Get();
		m_pCounter = _which.GetCounter();

		if (m_pCounter != NULL) (*m_pCounter)++;

		return *this;
	}

	operator O3D_BOOL() const {	return Valid(); }

	O3D_BOOL Valid() const
	{
		O3D_ASSERT(((m_pCounter == NULL) && (m_pObject == NULL)) || ((m_pCounter != NULL) && (m_pObject != NULL)));

		return (m_pObject != NULL);
	}

	// Release the object.
	void Release()
	{
		if(m_pCounter != NULL)
		{
			(*m_pCounter)--;

			O3D_ASSERT(m_pCounter >= 0);

			if(*m_pCounter == 0)
			{
				O3D_DELETE(m_pCounter);
				O3D_DELETE(m_pObject);
			}
		}
		else
		{
			O3D_ASSERT(m_pObject == NULL);
		}

		m_pObject = NULL;
		m_pCounter = NULL;
	}

	T* Get() { return m_pObject; }
	T* Get() const { return m_pObject; }

	O3D_UINT * GetCounter() { return m_pCounter; }
	O3D_UINT * GetCounter() const { return m_pCounter; }

	O3D_INT GetRefCounter() const {return ((m_pCounter != NULL)?*m_pCounter:-1); }

	T& operator*() { return *m_pObject; }
	const T& operator*() const { return *m_pObject; }

	T* operator->() { return m_pObject; }
	const T* operator->() const { return m_pObject; }

	bool operator==(const CLMSmartPointer& _which) const { return (m_pObject == _which.m_pObject); }
	bool operator!=(const CLMSmartPointer& _which) const { return (m_pObject != _which.m_pObject); }

protected:

	T* m_pObject;
	O3D_UINT* m_pCounter;
};
