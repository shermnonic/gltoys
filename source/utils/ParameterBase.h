#ifndef PARAMETERBASE_H
#define PARAMETERBASE_H

#include <string>
#include <sstream> // stringstream (for default value to string conversion)
#include <vector>
#include <cassert>
//#include <map> // for factory

#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
typedef boost::property_tree::ptree PTree;
#endif

#define PARAMETERBASE_JSON_SERIALIZATION

#ifdef PARAMETERBASE_JSON_SERIALIZATION
#include <nlohmann/json.hpp>
#endif

/**\verbatim

    Parameter system
    ================
    Max Hermann, September 2013

    Class hierarchy
    ---------------
    ParameterBase               // Abstract base class provides key(), type()
    +- ParameterBaseDefault<T>  // Provides value(), defaultValue()
       +- NumericParameter<T>   // Provides limits()
       |  |                     // Classes below are defined in ParameterTypes.h
       |  +- DoubleParameter
       |  +- IntParameter
       |     +- EnumParameter
       +- StringParameter

    Requirements / Features
    -----------------------
    (Marked with + are already implemented features)
    + Different data types
    + Default value and range (if applicable)
    + Description
    + Serialization (boost::property_tree)
    - Interpolation
    - Easy adaptor system to integrate complex types of different libraries
    - Automatic UI generation, extendable for custom types (Qt)
    - Compound parameter types?
    - Reference semantics (m_value as reference to member variable) and / or
      some kind of update mechanism (visitor, callback?).

    Serialization
    -------------
    Serialization is realized via boost::property_tree. Each class has to
    provide an implementation of \a read() and \a write() responsible to
    serialize its own members. Note that in this design the derived class is 
    responsible to invoke the serialization functions of its super class.

    Notes
    -----
    Further thoughts:
    - UI implementations for custom types could be realized via a factory 
      pattern.
    - Serialization could be realized via strategy pattern.
    - Adding value[To|From]String() functions to ParameterBase would enable
      canonic GUI treatment. Though, in the end we want to use some extendable
      factory solution for custom Qt delegates.
    - Hierarchy and parameter groups could be realized via naming convention,
      e.g. a slash ('/') could separate levels of an (acyclic) tree. So far I
      see no advantage in reflecting a parameter hierarchy in our storage class
      and would argue that the exisiting ParameterList concept is sufficient.	  

    Future extensions:
    - Support for change history?
      
    For inspiration see also:
    - Parameter management article and code by Qinghai Zhang:
      "C++ Design Patterns for Managing Parameters in Scientific Computing."
      http://www.math.utah.edu/~tsinghai/papers/parameterPatterns.pdf	  

\endverbatim
*/

//-----------------------------------------------------------------------------
// --- Parameter base classes ---
//-----------------------------------------------------------------------------

class ParameterBase;

typedef std::vector<ParameterBase*> ParameterVector;

/// Manage a named list of pointers to parameter instances.
class ParameterList
{
public:
    ParameterList(std::string name)
    {}

    ParameterList(std::string name, ParameterVector& params)
    : m_name(name),
      m_params(params)
    {}
    
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }

    /// Access parameter by key, returns nullptr if no matching parameter found
    ParameterBase* getParam( std::string key );

    ParameterVector& getParams() { return m_params; }
    const ParameterVector& getParams() const { return m_params; }

    void reset();

private:
    std::string m_name;
    ParameterVector m_params;
};

/// Equality operator based on key comparison.
/// Note that ParameterList has a custom function get_param(key).
bool operator == ( const ParameterBase& a, const ParameterBase& b );


//-----------------------------------------------------------------------------
//	ParameterBase
//-----------------------------------------------------------------------------
/// Abstract parameter base class, providing key and type semantics.
/// Value semantics is not included here but realized in a template child 
/// class. This serves a template free super class for all parameter types.
class ParameterBase // AbstractParameter ?
{
public:
    ParameterBase( const std::string& key );

    // Key and type semantics

    std::string         key()  const;
    virtual std::string type() const;

    // Equality operator (via key comparison, defined globally)

    friend bool operator == ( const ParameterBase& a, const ParameterBase& b );

    // Serialization
#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
    virtual void write( PTree& pt ) const;
    virtual void read ( const PTree& pt );
#endif

#ifdef PARAMETERBASE_JSON_SERIALIZATION
    virtual void write(nlohmann::json& j) const;
    virtual void read(const nlohmann::json& j);
#endif

    /// Return string representation of current value
    virtual std::string str() const;

    /// Reset to default value (implemented in subclass ParameterBaseDefault)
    virtual void reset() { assert(false); /* should never be executed */ };

private:
    std::string m_key;
    std::string m_type;
};

//-----------------------------------------------------------------------------
//	ParameterBaseDefault< T >
//-----------------------------------------------------------------------------

//#define PARAMETERBASE_DEFAULT_CTORS( NAME, TYPE, TYPENAME ) \
//  NAME( const std::string& key )                            \
//      : ParameterBaseDefault( key )                         \
//  {}                                                        \
//                                                            \
//  NAME( const std::string& key, TYPE value_and_default )    \
//      : ParameterBaseDefault( key, value_and_default )      \
//  {}                                                        \
//                                                            \
//  NAME( const std::string& key, TYPE value, TYPE default_ ) \
//      : ParameterBaseDefault( key, value, default_ )        \
//  {}                                                        \
//  virtual std::string type() const { return TYPENAME; }

/// Templated base parameter class providing value and default value semantics.
template<class T> // ParameterBase
class ParameterBaseDefault: public ParameterBase
{
public:
    ParameterBaseDefault( const std::string& key )
        : ParameterBase( key ),
          m_value  ( T() ), // Initialize w/ default c'tor
          m_default( T() )
    {}

    ParameterBaseDefault( const std::string& key, T value_and_default_ )
        : ParameterBase( key ),
          m_value  ( value_and_default_ ),
          m_default( value_and_default_ )
    {}

    ParameterBaseDefault( const std::string& key, T value, T default_ )
        : ParameterBase( key ),
          m_value  ( value ),
          m_default( default_ )
    {}

    // Default value semantics

    void setValueAndDefault( const T& val )
    {
        setValue( val );
        m_default = val;
    }

    void setDefault( const T& val )
    {
        m_default = val;
    }

    T defaultValue() const  // Note, that `default` is a keyword since C++11
    { 
        return m_default; 
    }

    void reset()
    {
        m_value = m_default;
    }

    // Value semantics

    void setValue( const T& val )
    {
        m_value = val;
    }

    const T& value() const
    {
        return m_value;
    }

    T& valueRef() const
    {
        return m_value;
    }

    // Default serialization (FIXME: How to handle non-basic types?)
#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
    virtual void write( PTree& pt ) const
    {
        ParameterBase::write( pt ); // call super
        pt.put( "value"  ,value() );
        pt.put( "default",defaultValue() );
    }

    virtual void read( const PTree& pt )
    {
        ParameterBase::read( pt ); // call super
        m_value   = pt.get<T>( "value" );
        m_default = pt.get<T>( "default" );
    }
#endif

#ifdef PARAMETERBASE_JSON_SERIALIZATION
    virtual void write(nlohmann::json& j) const
    {
        ParameterBase::write(j);
        j["value"]   = value();
        j["default"] = defaultValue();
    }

    virtual void read(const nlohmann::json& j)
    {
        ParameterBase::read(j);
        if (!j.contains("value") || !j.contains("default"))
        {
            throw std::runtime_error("Invalid json");
        }
        m_value   = j["value"]  .get<T>();
        m_default = j["default"].get<T>();
    }
#endif

    // Default value to string conversion
    std::string str() const
    {
        std::stringstream ss;
        ss << m_value;
        return ss.str();
    }

private:
    T m_value;
    T m_default;
};

//-----------------------------------------------------------------------------
//	NumericParameter< T >
//-----------------------------------------------------------------------------

/// Base class for all numeric parameters, providing min/max range semantic.
template<class T>
class NumericParameter: public ParameterBaseDefault<T>
{
public:
    typedef ParameterBaseDefault<T> Super;

    NumericParameter( const std::string& key )
        : Super( key )
    {}

    NumericParameter( const std::string& key, T value_and_default )
        : Super( key, value_and_default, value_and_default )
    {}

    NumericParameter( const std::string& key, T value_and_default, T min_, T max_ )
        : Super( key, value_and_default, value_and_default ),
          m_limits( min_, max_ )
    {}

    NumericParameter( const std::string& key, T value_, T min_, T max_,
                      T default_ )
        : Super( key, value_, default_ ),
          m_limits( min_, max_ )
    {}

    struct Range { 
        Range()
            : min_(std::numeric_limits<T>::lowest()),
              max_(std::numeric_limits<T>::max()),
              active(false)
        {}
        
        Range( const T& min__, const T& max__ )
            : min_(min__),
              max_(max__),
              active(true)
        {}

        T min_, max_; 
        bool active;

        T clamp( const T& val ) const 
        { 
            return (val > max_) ? max_ : ((val < min_) ? min_ : val); 
        }
    };

    // Value range limits semantics

    void setLimits( const T& min_, const T& max_ )
    {
        m_limits = Range( min_, max_ );
    }

    void setLimits( const Range& limits )
    {
        m_limits = limits;
    }

    Range limits() const
    {
        return m_limits;
    }

    // Serialization
#ifdef PARAMETERBASE_BOOST_PTREE_SERIALIZATION
    virtual void write( PTree& pt ) const
    {
        Super::write( pt ); // call super
        // Do not store limits if they are not used (i.e. initialized)
        if( m_limits.active ) {
            pt.put( "limit_min", limits().min_ );
            pt.put( "limit_max", limits().max_ );
        }
    }

    virtual void read( const PTree& pt )
    {
        Super::read( pt ); // call super
        boost::optional<T> min_ = pt.get_optional<T>( "limits_min" );
        boost::optional<T> max_ = pt.get_optional<T>( "limits_max" );
        // Only set limits if *both* min and max are provided
        if( min_ && max_ )
            setLimits( Range(min_.get(),max_.get()) );		
    }
#endif

#ifdef PARAMETERBASE_JSON_SERIALIZATION
    virtual void write(nlohmann::json& j) const
    {
        Super::write(j);
        if (m_limits.active)
        {
            j["rangeFrom"] = limits().min_;
            j["rangeTo"]   = limits().max_;
        }
    }

    virtual void read(const nlohmann::json& j)
    {
        Super::read(j);
        if (j.contains("rangeFrom") && j.contains("rangeTo"))
        {
            setLimits(j["rangeFrom"].get<T>(), j["rangeTo"].get<T>());
        }
        else
        {
            m_limits = Range();
        }
    }
#endif

private:
    Range m_limits;
};


//-----------------------------------------------------------------------------
// --- Parameter factory ---  (NOT YET!)
//-----------------------------------------------------------------------------
/*
class ParameterFactory
{
public:
    /// Return singleton instance
    static ParameterFactory& ref()
    {
        static ParameterFactory singleton;
        return singleton;
    }

    typedef ParameterBase* (*CreateParameterCallback)();

    /// Returns parameter instance for given type string (e.g. "double")
    /// Returns nullptr if type string is not supported.
    ParameterBase* create_Parameter( const boost::property_tree::ptree& pt );

    /// Register new parameter class for specific type string
    /// Returns true if registration was succesful
    bool register_format( std::string type, CreateParameterCallback cb );

private:

    typedef std::map<std::string, ParameterFactory> CallbackMap;
    CallbackMap m_callbacks;

    // make c'tors private for singleton
    ParameterFactory() {}
    ~ParameterFactory() {}
    ParameterFactory( const ParameterFactory& ) {}
    ParameterFactory& operator = ( const ParameterFactory& ) { return *this; }
};
*/

#endif // PARAMETERBASE_H
