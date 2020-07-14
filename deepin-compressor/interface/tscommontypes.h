#ifndef TSCOMMONTYPES_H
#define TSCOMMONTYPES_H

#include <memory>

////////////////////////////////////////////////////////////////////////////////
///<summary>transfer one smart ptr to another smart ptr</summary>
///
/// <remarks>han,2020/05/09.</remarks>
///
/// <param name="from">source smart pointer</param>
/// <param name="to">target smart pointer </param>
///////////////////////////////////////////////////////////////////////////////
#define TS_STATIC_CAST(from , to)  std::static_pointer_cast<to::element_type>(from)
#define TS_CAST(from , to)  std::dynamic_pointer_cast<to::element_type>(from)

////////////////////////////////////////////////////////////////////////////////
///<summary>transfer this to smart ptr</summary>
///
/// <remarks>han,2020/05/09.</remarks>
///
///////////////////////////////////////////////////////////////////////////////
#define TS_THISPTR std::static_pointer_cast<ThisType>(shared_from_this())

#define CLASS_PTR_DECLARE(a) typedef std::shared_ptr<class a> a##Ptr;\
    typedef std::weak_ptr<class a> a##WPtr

#define STRUCT_PTR_DECLARE(a) typedef std::shared_ptr<struct a> a##Ptr;\
    typedef std::weak_ptr<struct a> a##WPtr

#endif // TSCOMMONTYPES_H
