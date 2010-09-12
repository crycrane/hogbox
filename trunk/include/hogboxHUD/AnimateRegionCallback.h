#pragma once

#include <hogbox/AnimateValue.h>
#include <deque>


namespace hogboxHUD 
{

class HudRegion;

//
//AnimateRegionCallback
//Provides a Animate value per attribute of the region
//KeyFrames can be added and will animate through them 
//until the keyframe queue is empty
//
class AnimateRegionCallback : public osg::NodeCallback
{
public:
	//contuct, passing the region to update
	AnimateRegionCallback(HudRegion* region) : osg::NodeCallback(),
												m_updateRegion(region),
												m_rotate(0.0f),m_isRotating(false),
												m_size(osg::Vec2(1.0f,1.0f)),m_isTranslating(false),
												m_position(osg::Vec2(0.0f,0.0f)),m_isSizing(false),
												m_color(osg::Vec3(0.0f,0.0f,0.0f)),m_isColoring(false),
												m_alpha(1.0f),m_isFading(false),
												m_prevTick(0.0f),
												m_disabled(false)
	{
		if(m_updateRegion)
		{
			m_rotate.SetValue(m_updateRegion->GetRotation());
			m_size.SetValue(m_updateRegion->GetSize());
			m_position.SetValue(m_updateRegion->GetPosition());
			m_color.SetValue(m_updateRegion->GetColor());
			m_alpha.SetValue(m_updateRegion->GetAlpha());
		}
	}
	virtual ~AnimateRegionCallback(void){}

	//
	//Update operator
	//Here we smooth to targets if value is not equal
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (m_updateRegion &&
		nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR && 
		nv->getFrameStamp())
		{
			//get the time passed since last update
			double time = nv->getFrameStamp()->getReferenceTime();
			if(m_prevTick==0.0f){m_prevTick = time;}
			float timePassed = time - m_prevTick;
			m_prevTick = time;

			if(!m_disabled)
			{
				//update all our smooth values
				if(m_isRotating = m_rotate.Update(timePassed))
				{m_updateRegion->SetRotation(m_rotate.GetValue());}

				if(m_isTranslating = m_position.Update(timePassed))
				{m_updateRegion->SetPosition(m_position.GetValue());}

				if(m_isSizing = m_size.Update(timePassed))
				{m_updateRegion->SetSize(m_size.GetValue());}

				if(m_isColoring = m_color.Update(timePassed))
				{m_updateRegion->SetColor(m_color.GetValue());}

				if(m_isFading = m_alpha.Update(timePassed))
				{m_updateRegion->SetAlpha(true, m_alpha.GetValue());}
			}
		}
		osg::NodeCallback::traverse(node,nv);
	}

	//Rotation channel
	template <typename M>
	void AddRotationKey(float degrees, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_rotate.GetNumKeys() == 0){m_rotate.SetValue(m_updateRegion->GetRotation());}
		m_rotate.AddKey<M>(degrees, duration);
	}
	hogbox::AnimateValue<float>::KeyFrame* GetRotationKey(unsigned int index){return m_rotate.GetKey(index);}
	bool RemoveRotationKey(unsigned int index){return m_rotate.RemoveKey(index);}
	unsigned int GetNumRotationKeys(){return m_rotate.GetNumKeys();}

	//Position channel
	template <class M>
	void AddPositionKey(osg::Vec2 pos, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_position.GetNumKeys() == 0){m_position.SetValue(m_updateRegion->GetPosition());}
		m_position.AddKey<M>(pos, duration);
	}
	hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetPositionKey(unsigned int index){return m_position.GetKey(index);}
	bool RemovePositionKey(unsigned int index){return m_position.RemoveKey(index);}
	unsigned int GetNumPositionKeys(){return m_position.GetNumKeys();}

	//Size channel
	template <typename M>
	void AddSizeKey(osg::Vec2 size, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_size.GetNumKeys() == 0){m_size.SetValue(m_updateRegion->GetSize());}
		m_size.AddKey<M>(size, duration);
	}
	hogbox::AnimateValue<osg::Vec2>::KeyFrame* GetSizeKey(unsigned int index){return m_size.GetKey(index);}
	bool RemoveSizeKey(unsigned int index){return m_size.RemoveKey(index);}
	unsigned int GetNumSizeKeys(){return m_size.GetNumKeys();}

	//Color channel
	template <typename M>
	void AddColorKey(osg::Vec3 color, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_color.GetNumKeys() == 0){m_color.SetValue(m_updateRegion->GetColor());}
		m_color.AddKey<M>(color, duration);
	}
	hogbox::AnimateValue<osg::Vec3>::KeyFrame* GetColorKey(unsigned int index){return m_color.GetKey(index);}
	bool RemoveColorKey(unsigned int index){return m_color.RemoveKey(index);}
	unsigned int GetNumColorKeys(){return m_color.GetNumKeys();}

	//Alpha channel
	template <typename M>
	void AddAlphaKey(float alpha, float duration)
	{
		//if its going to be the first key ensure value is the regions current
		if(m_alpha.GetNumKeys() == 0){m_alpha.SetValue(m_updateRegion->GetAlpha());}
		m_alpha.AddKey<M>(alpha, duration);
	}
	hogbox::AnimateValue<float>::KeyFrame* GetAlphaKey(unsigned int index){return m_alpha.GetKey(index);}
	bool RemoveAlphaKey(unsigned int index){return m_alpha.RemoveKey(index);}
	unsigned int GetNumAlphaKeys(){return m_alpha.GetNumKeys();}

	//are any of our channels still animating
	bool IsAnimating()
	{
		unsigned int totalKeys = GetNumPositionKeys() + GetNumSizeKeys() + GetNumColorKeys() + GetNumAlphaKeys();
		if(totalKeys > 0)
		{return true;}
		return false;
	}

	void SetDisabled(bool disable){m_disabled = disable;}

private:

	//the region we want to update
	HudRegion* m_updateRegion;
	
	hogbox::AnimateValue<float> m_rotate; bool m_isRotating;
	hogbox::AnimateValue<osg::Vec2> m_position; bool m_isTranslating;
	hogbox::AnimateValue<osg::Vec2> m_size; bool m_isSizing;
	hogbox::AnimateValue<osg::Vec3> m_color; bool m_isColoring;
	hogbox::AnimateValue<float> m_alpha; bool m_isFading;

	//previous framestamp time to calc time elapsed
	float m_prevTick;

	//used to stop any animation
	bool m_disabled;
};
	
};
