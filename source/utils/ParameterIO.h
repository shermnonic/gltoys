#ifndef PARAMETERIO_H
#define PARAMETERIO_H

#include "ParameterBase.h"
#include <iostream>

//-----------------------------------------------------------------------------
// --- Parameter IO ---
//-----------------------------------------------------------------------------

/// Print parameters to an output stream.
void print_params( const ParameterList& parms, std::ostream& os=std::cout );

#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
/// Write parameters to disk.
void save_params( const char* filename, const ParameterList& parms );

/// Load parameters from disk to a given ParameterList, matched by key.
void load_params( const char* filename, ParameterList& parms );
#endif

#ifdef PARAMETERBASE_JSON_SERIALIZATION
void appendParameterListToJsonCollection(const ParameterList& plist, nlohmann::json& j);
bool readParameterListFromJsonCollection(const nlohmann::json& j, ParameterList& plist);

void writeParams(const ParameterList& plist, std::ostream& os);
void readParams(std::istream& is, ParameterList& plist);
#endif

#endif // PARAMETERIO_H
