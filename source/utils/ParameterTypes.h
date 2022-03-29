#ifndef PARAMETERTYPES_H
#define PARAMETERTYPES_H

#include "ParameterBase.h"
#include <string>
#include <vector>


//-----------------------------------------------------------------------------
// --- Concrete parameter types ---
//-----------------------------------------------------------------------------

#define PARAMETERBASE_NUMERIC_PARAM( NAME, TYPE, TYPENAME )   \
    public:                                                   \
    NAME( const std::string& key )                            \
        : NumericParameter( key )                             \
    {}                                                        \
    NAME( const std::string& key, TYPE value_ )               \
        : NumericParameter( key, value_ )                     \
    {}                                                        \
    NAME( const std::string& key,                             \
                      TYPE value_, TYPE min_, TYPE max_ )     \
        : NumericParameter( key, value_, min_, max_ )         \
    {}                                                        \
    NAME( const std::string& key,                             \
                      TYPE value_, TYPE min_, TYPE max_,      \
                      TYPE default_ )                         \
        : NumericParameter( key, value_, min_, max_, default_ ) \
    {}                                                        \
    virtual std::string type() const { return TYPENAME; }

class DoubleParameter: public NumericParameter<double>
{
    PARAMETERBASE_NUMERIC_PARAM( DoubleParameter, double, "double" )
};

class IntParameter: public NumericParameter<int>
{
    PARAMETERBASE_NUMERIC_PARAM( IntParameter, int, "int" )
};

class StringParameter: public ParameterBaseDefault<std::string>
{
    //PARAMETERBASE_DEFAULT_CTORS( StringParameter, std::string, "string" )
public:
    StringParameter( const std::string& key )
        : ParameterBaseDefault( key )
    {}

    StringParameter( const std::string& key, std::string value_and_default )
        : ParameterBaseDefault( key, value_and_default )
    {}

    StringParameter( const std::string& key, std::string value, std::string default_ ) 
        : ParameterBaseDefault( key, value, default_ )
    {}

    virtual std::string type() const { return "string"; }
};

/// A boolean parameter, realized via a two values integer range.
class BoolParameter: public IntParameter
{
public:
    BoolParameter( const std::string& key )
        : IntParameter( key, (int)true, 0,1 )
    {}

    BoolParameter( const std::string& key, bool value_and_default )
        : IntParameter( key, value_and_default, 0,1 )
    {}

    std::string type() const { return "bool"; };

protected:
    // Override limit functions and make them inaccessible from outside
    void setLimits( const int& min_, const int& max_ )
    {
        IntParameter::setLimits( min_, max_ );
    }
    void setLimits( const Range& limits )
    {
        IntParameter::setLimits( limits );
    }
};

/// An enum parameter, realized as a named limited integer range.
class EnumParameter: public IntParameter
{
public:
    EnumParameter( const std::string& key, std::vector<std::string> enumNames )
        : IntParameter( key, 0, 0, enumNames.size() ),
          m_enumNames( enumNames )
    {}

    EnumParameter( const std::string& key, int value_and_default, 
                   std::vector<std::string> enumNames )
        : IntParameter( key, value_and_default, 0, enumNames.size(), value_and_default),
          m_enumNames( enumNames )
    {}

    EnumParameter(const std::string& key, int value,
        std::vector<std::string> enumNames, int defaultValue)
        : IntParameter(key, value, 0, enumNames.size(), defaultValue),
        m_enumNames(enumNames)
    {}

    std::string type() const { return "enum"; };

    const std::vector<std::string>& enumNames() const { return m_enumNames; }

protected:
    // Override limit functions and make them inaccessible from outside
    void setLimits( const int& min_, const int& max_ )
    {
        IntParameter::setLimits( min_, max_ );
    }
    void setLimits( const Range& limits )
    {
        IntParameter::setLimits( limits );
    }

private:
    std::vector<std::string> m_enumNames;
};

#endif // PARAMETERTYPES_H
