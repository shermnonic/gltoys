#include "ParameterBase.h"

void ParameterList::reset() 
{ 
    for (ParameterBase* p : m_params) p->reset(); 
}

ParameterBase::ParameterBase( const std::string& key )
    : m_key(key)
{
    // Internally assign type of derived class
    m_type = type();
}

std::string ParameterBase::key()  const { return m_key; }
std::string ParameterBase::type() const { return m_type; } //=0;

#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
void ParameterBase::write( PTree& pt ) const
{
    pt.put( "key" , key() );
    pt.put( "type", type() );
}

void ParameterBase::read ( const PTree& pt )
{
    m_key  = pt.get<std::string>( "key" );
    m_type = pt.get<std::string>( "type" );
}
#endif

#ifdef PARAMETERBASE_JSON_SERIALIZATION
void ParameterBase::write(nlohmann::json& j) const
{
    j["key"] = key();
    j["type"] = type();
}

void ParameterBase::read(const nlohmann::json& j)
{
    if (!j.contains("key") || !j.contains("type"))
    {
        throw std::runtime_error("Invalid json");
    }
    m_key = j["key"].get<std::string>();
    m_type = j["type"].get<std::string>();
}
#endif


std::string ParameterBase::str() const
{
    return std::string("(No string conversion available.)");
}

ParameterBase* ParameterList::getParam( std::string key )
{
    ParameterVector::iterator it = m_params.begin();
    for( ; it != m_params.end(); it++ )
    {
        if( (*it)->key() == key )
            return *it;
    }
    
    return nullptr;
}

// Equality operator (via key comparison, defined globally)
bool operator == ( const ParameterBase& a, const ParameterBase& b )
{
    return a.key() == b.key();
};

