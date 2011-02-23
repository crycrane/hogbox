#include <hogboxStage/EntityManager.h>

using namespace hogboxStage;


osg::ref_ptr<EntitytManager> s_entityManagerInstance = NULL;

EntitytManager* EntitytManager::Instance(bool erase)
{
	if(s_entityManagerInstance.get()==NULL)
	{
		s_entityManagerInstance = new EntitytManager();
		//also register the singleton instance with the hogboxDBRegistry
		static hogboxDB::XmlNodeManagerRegistryProxy g_proxy_EntityManager(s_entityManagerInstance.get() , "Entity" );
	}		
	if(erase)
	{
		s_entityManagerInstance->destruct();
		s_entityManagerInstance = 0;
	}
    return s_entityManagerInstance.get();
}

EntitytManager::EntitytManager(void)
	: hogboxDB::XmlClassManager()
{
	SupportsClassType("Entity", "Xml definition of Entity and basic derived types");
}

EntitytManager::~EntitytManager(void)
{

}

hogboxDB::XmlClassWrapperPtr EntitytManager::ReadObjectFromXmlNodeImplementation(osgDB::XmlNode* xmlNode)
{
	EntityXmlWrapperPtr entityWrapper;

	//create our object and it's xml wrapper.
	entityWrapper = new EntityXmlWrapper(xmlNode);
	
	if(!entityWrapper.get()){return NULL;}
	//did the wrapper alocate an object
	if(!entityWrapper->getWrappedObject()){return NULL;}

	//if the wrapper was created properly then use it 
	//to deserialize our xmlNode into it's wrapped object
	if(!entityWrapper->deserialize(xmlNode))
	{
		//an error occured deserializing the xml node
		return NULL;
	}

	return entityWrapper;
}

//
//Add an existing entity to our list
//
void EntitytManager::AddEntity(Entity* entity)
{
	if(!entity){return;}
	//add the entity to out wrapper
	EntityXmlWrapperPtr newObject = new EntityXmlWrapper(entity);
	//check it loaded
	if(newObject.get())
	{
		//add to our list of loaded nodes
		XmlNodeToObjectPair newObjectEntry(new osgDB::XmlNode(), newObject);
		m_objectList.insert(newObjectEntry);
		return;// newObject->getWrappedObject();
	}
}