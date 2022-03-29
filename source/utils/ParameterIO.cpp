#include "ParameterIO.h"

#include <vector>
#include <algorithm>

#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#endif

void print_params( const ParameterList& parms, std::ostream& os )
{
    std::cout << parms.getName() << ":" << std::endl;
    for (auto p : parms.getParams())
    {
        std::cout << p->key() << " = " << p->str() << " (" << p->type() << ")" << std::endl;
    }
}

#ifdef PARAMETERBASE_JSON_SERIALIZATION
nlohmann::json getParameterListAsJson(const ParameterList& plist)
{
    using nlohmann::json;
    json j_array;
    for (auto p : plist.getParams())
    {
        json j;
        p->write(j);
        j_array.push_back(j);
    }

    json j;
    j["type"] = "ParameterList";
    j["name"] = plist.getName();
    j["parameters"] = j_array;
    return j;
}

void parseParameterListFromJson(ParameterList& plist, const nlohmann::json& j)
{
    if (!(j.contains("type") && j["type"] == "ParameterList" && j.contains("name")))
    {
        throw std::runtime_error("Invalid json");
    }

    if (j["name"] != plist.getName())
    {
        throw std::runtime_error("Mismatching json parameter list name");
    }

    if (j.contains("parameters"))
    {
        nlohmann::json j_parms_array = j["parameters"];
        if (!j_parms_array.is_array())
        {
            throw std::runtime_error("Invalid json");
        }
        for (auto& j_parm : j_parms_array)
        {
            ParameterBase* p = plist.getParam(j_parm["key"].get<std::string>());
            if (p)
            {
                p->read(j_parm);
            }
        }
    }
}

void appendParameterListToJsonCollection(const ParameterList& plist, nlohmann::json& j)
{
    j["type"] = "ParameterListCollection";
    j["data"].push_back(getParameterListAsJson(plist));
}

bool readParameterListFromJsonCollection(const nlohmann::json& j, ParameterList& plist)
{
    if (!(j.contains("type") && j["type"] == "ParameterListCollection" && j.contains("data") && j["data"].is_array()))
    {
        throw std::runtime_error("Invalid json");
    }

    bool found = false;
    for (auto& j_plist : j["data"])
    {
        if (j_plist["name"] == plist.getName())
        {
            parseParameterListFromJson(plist, j_plist);
            found = true;
            break;
        }
    }

    return found;
}

void writeParams(const ParameterList& plist, std::ostream& os)
{
    os << getParameterListAsJson(plist).dump(4);
}

void readParams(std::istream& is, ParameterList& plist)
{
    nlohmann::json j;
    is >> j;
    parseParameterListFromJson(plist, j);

}
#endif

#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
void save_params( const char* filename, const ParameterList& parms )
{
    using boost::property_tree::ptree;	
    ptree root;

    ParameterList::const_iterator it = parms.begin();
    for( ; it != parms.end(); it++ )
    {
        ptree pt;
        (*it)->write( pt );		
        root.add_child( "ParameterList.Parameter", pt );
    }

    write_xml( filename, root );
}

// Load w/o factory to known ParameterList
void load_params( const char* filename, ParameterList& parms )
{
    using namespace std;
    using boost::property_tree::ptree;
    ptree root;
    
    read_xml( filename, root );

    // ParameterList's on disk and in parms must match!
    // Iterate simultaneously over both.
    ParameterList::iterator it = parms.begin();	

#if 0
    // Debugging
    std::cout << "Read following parameter list from " << filename << ":" << std::endl;
    BOOST_FOREACH( ptree::value_type &v, root.get_child("ParameterList") )
    {
        std::cout << v.second.get<std::string>("key")
            << "(" << v.second.get<std::string>("type") << ")" << std::endl;
    }
#endif

    // Iterate through read property_tree
    ptree pt = root.get_child("ParameterList");
    boost::property_tree::ptree::iterator pit = pt.begin();	
#if 0
    // Assume exactly matching ParameterList and property_tree instances
    // (static solution)  OBSOLETE
    for( ; it != parms.end() && pit != pt.end(); it++, pit++ )
    {	
        // Read base fields (key,type) from file
        ParameterBase param("");
        param.read( pit->second );

        // Sanity check
        if( (*it)->type() == param.type() )
        {
            (*it)->read( pit->second );
        }
        else
        {
            // Type mismatch
            std::cerr << "Error: Type mismatch for variable " 
                << param.key() << std::endl;

            // Reset to default value
            (*it)->reset();
        }
    }	
#else
    // Match found variable names in property_tree to ParameterList
    // (dynamic solution)
    for( ; pit != pt.end(); pit ++ )
    {
        // Read base fields (key,type) from file
        ParameterBase param("");
        param.read( pit->second );

        ParameterBase* ptr = parms.get_param( param.key() );
        if( ptr )
        {
            // Sanity check type
            if( ptr->type() == param.type() )
            {
                // Found matching variable in given parameter list
                // Read again from file, this time with all fields for the
                // actual type.
                ptr->read( pit->second );
            }
            else
            {
                // Type mismatch
                std::cerr << "Error: Type mismatch for variable " 
                    << param.key() << std::endl;
            }
        }
    }
#endif
}
#endif
