#pragma once
#include "UI.h"

class BasicUI
{
public:
	virtual void draw() = 0;
};

/*
*	Just sugar
*/
template <class UIType,
	class = class std::enable_if<std::is_base_of<BasicUI, UIType>::value>>
class BasicCreator;

class ObjectManager;
namespace Creator
{
	enum class CreatorMode
	{
		CREATE,
		DESTROY
	};
}

template <class UIType> 
class BasicCreator <UIType>
{
public:
	BasicCreator(ObjectManager* objManager);
	void setCreatorMode(Creator::CreatorMode mode);
	void setActive(bool active);

	void drawUI();

	virtual void update();
protected:
	virtual void setCreatorModeAction();
	virtual void setActiveAction();

	bool m_active = false;
	UIType m_ui = {};
	Creator::CreatorMode m_currentMode = {};

	ObjectManager* m_pObjectManager;
};

/*
template<class UIType>
void BasicCreator<UIType>::setCreatorMode(CreatorMode mode) const
{
	m_currentMode = mode;
}*/

template<class UIType>
inline BasicCreator<UIType>::BasicCreator(ObjectManager* objManager)
	:m_pObjectManager(objManager)
{
}

template<class UIType>
void BasicCreator<UIType>::setCreatorMode(Creator::CreatorMode mode)
{
	if (m_currentMode != mode)
	{
		m_currentMode = mode;

		setCreatorModeAction();
	}
}

template<class UIType>
void BasicCreator<UIType>::setActive(bool active)
{
	if (m_active != active)
	{
		m_active = active;

		setActiveAction();
	}
}

template<class UIType>
inline void BasicCreator<UIType>::setCreatorModeAction()
{
}

template<class UIType>
inline void BasicCreator<UIType>::setActiveAction()
{
}

template<class UIType>
void BasicCreator<UIType>::drawUI()
{
	m_ui.draw();
}

template<class UIType>
inline void BasicCreator<UIType>::update()
{
}
