/* Written by Thomas Hogarth, (C) 2011
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#pragma once

#include <hogboxDB/XmlAttribute.h>

//
//Xml Attribute for enum types
//
namespace hogboxDB {

	
    class EnumLookUp{
    public:
        EnumLookUp(){
        }
        virtual ~EnumLookUp(){}
        
        void addLookUp(const std::string& name, int value){
            if(_lookup.count(name) > 0){
                OSG_WARN << "EnumLookUp::addLookUp: WARNING: Lookup name '" << name << "' is already in lookup with value '" << _lookup[name] << "'." << std::endl;
                return;
            }
            _lookup[name] = value;
        }
        
        //
        //return int for name
        bool getValue(const std::string& name, int& value){
            if(_lookup.count(name) == 0){
                OSG_WARN << "EnumLookUp::addLookUp: ERROR: Lookup name '" << name << "' does not exist." << std::endl;
                return false;
            }
            value = _lookup[name];
            return true;
        }
        
        //
        //return name for int
        bool getName(const int& value, std::string& name){
            std::map<std::string, int>::iterator itr = _lookup.begin();
            for( ; itr!=_lookup.end(); itr++){
                if((*itr).second == value){
                    name = (*itr).first;
                    return true;
                }
            }
            return false;
        }
        
    protected:
        std::map<std::string, int> _lookup;
    };
    
    class EnumXmlAttribute : public XmlAttribute
    {
    public:
        EnumXmlAttribute() : XmlAttribute() {
        }
        
        //
        //Add an enum value and reference name 
        void addEnum(const std::string& name, int value){
            _lookup.addLookUp(name, value);
        }
        
    protected:
        virtual ~EnumXmlAttribute(){}
    protected:
        //our lookup to convert 
        EnumLookUp _lookup;
    };
    
	template <typename T> class EnumTypedXmlAttribute: public EnumXmlAttribute 
	{
	public:
        
		EnumTypedXmlAttribute(T* value = 0) 
        : EnumXmlAttribute(),			
        _value(value) 
		{
		}
		
		virtual const T& get() const { return *_value; }
		
		virtual void set(const T& value) {			
			*_value = value;
		}
		//write to nodes contents
		virtual bool serialize(osgDB::XmlNode* out) {
			//std::stringstream ss;
			//ss << *_value;
			//out->contents = ss.str();
			//out << *_value;
			return true;
		}
        
		virtual bool deserialize(osgDB::XmlNode* in) 
		{
			//
			if(!in){return false;}
            
            //read as string when convert using lookup
            std::string readValue;
			if(hogboxDB::getXmlContents(in, readValue))
			{
                int enumValue;
                if(_lookup.getValue(readValue, enumValue)){
                    set(static_cast<T>(enumValue));return true;
                }
            }
            
			return false;
		}
		
		void (EnumTypedXmlAttribute<T>::*f_sethandler)(T&, const T&);

        
	protected:
        
		EnumTypedXmlAttribute<T>& operator = (const T& value) {
			*_value = value;		
			return *this;
		}
        
		void operator = (const EnumTypedXmlAttribute<T>& value) {
			*_value = value.get();		
		}
		
		//the pointer to the member this
		//attribute represents
		T* _value;
    
	};

    
    //
    //
    //
	template <class C, typename T> class EnumCallbackXmlAttribute : public EnumTypedXmlAttribute<T> 
	{
	public:	
        
		typedef const T& (C::* GetHandler)() const;  //get value function definition
		typedef void (C::* SetHandler)(const T&); //set value function definition
        
		EnumCallbackXmlAttribute(C *object, 
                             GetHandler ghandler = 0,
                             SetHandler shandler = 0) : 
        f_gethandler(ghandler),			
        f_sethandler(shandler),
        mp_object(object)
		{
		}
		
		virtual const T& get() const {
			return (mp_object->*f_gethandler)();
		}
		
		virtual void set(const T& value) 
		{
			if (f_sethandler) 
			{
				(mp_object->*f_sethandler)(value);			
			} else 
			{
				OSG_WARN << "XML ERROR: No set method provided for XmlAttribute. Objects value will not be set." << std::endl;
			}
		}
		
	protected:	
        
		EnumCallbackXmlAttribute<T,C>& operator = (const T& value) {
			set(value);		
			return *this;
		}
        
		void operator = (const EnumCallbackXmlAttribute<T,C>& value) {
			set(value.get());		
		}
        
		//function pointers to the get/set 
		//methods of mp_object
		GetHandler f_gethandler;
		SetHandler f_sethandler;
		
		//the pointer to the class instance
		//we are calling our get set methods on
		C* mp_object;				
	};

 };//end hogboxDB namespace
 