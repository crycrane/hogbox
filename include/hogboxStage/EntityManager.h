//

#include <hogboxStage/Entity.h>
#include <hogboxStage/EntityXmlWrapper.h>
#include <hogboxStage/ComponentXmlWrapper.h>

#include <hogboxDB/HogBoxRegistry.h>
#include <hogboxDB/XmlClassManager.h>
#include <hogboxDB/HogBoxManager.h>




namespace hogboxStage 
{

//
//EntitytManager
//Will handle creation and updating etc of all entities on the stage
//the entity manager is also a hogboxDB xmlClassManager to allow us to
//use the xml system for saving and loading the entities
//All our entities our stored in the xml managers m_objectList
//
//Also responsible for loading entity components
//
class HOGBOXSTAGE_EXPORT EntitytManager : public hogboxDB::XmlClassManager
{
public:

	enum EntityType {
		PLAYER = 0
	};

    static EntitytManager* Instance(bool erase = false);

    virtual const char* className() const { return "EntityManager"; } 
	virtual const char* libraryName() const { return "hogboxStage"; } 
	static const std::string xmlClassName(){ osg::ref_ptr<EntitytManager> classType;return classType->className();}

	//Add an existing entity to our list
	void AddEntity(Entity* entity);
	

protected:
	
	EntitytManager(void);
	virtual ~EntitytManager(void);

	virtual void destruct(){
	}

	//implement xml manager stuff
	virtual hogboxDB::XmlClassWrapperPtr ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode);

protected:


};

};