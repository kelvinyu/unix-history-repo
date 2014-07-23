//===-- Type.h --------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_Type_h_
#define liblldb_Type_h_

#include "lldb/lldb-private.h"
#include "lldb/Core/ClangForward.h"
#include "lldb/Core/ConstString.h"
#include "lldb/Core/UserID.h"
#include "lldb/Symbol/ClangASTType.h"
#include "lldb/Symbol/Declaration.h"

#include <set>

namespace lldb_private {

class SymbolFileType :
    public std::enable_shared_from_this<SymbolFileType>,
    public UserID
    {
    public:
        SymbolFileType (SymbolFile &symbol_file, lldb::user_id_t uid) :
            UserID (uid),
            m_symbol_file (symbol_file)
        {
        }

        ~SymbolFileType ()
        {
        }

        Type *
        operator->()
        {
            return GetType ();
        }

        Type *
        GetType ();

    protected:
        SymbolFile &m_symbol_file;
        lldb::TypeSP m_type_sp;
    };
    
class Type :
    public std::enable_shared_from_this<Type>,
    public UserID
{
public:
    typedef enum EncodingDataTypeTag
    {
        eEncodingInvalid,
        eEncodingIsUID,                 ///< This type is the type whose UID is m_encoding_uid
        eEncodingIsConstUID,            ///< This type is the type whose UID is m_encoding_uid with the const qualifier added
        eEncodingIsRestrictUID,         ///< This type is the type whose UID is m_encoding_uid with the restrict qualifier added
        eEncodingIsVolatileUID,         ///< This type is the type whose UID is m_encoding_uid with the volatile qualifier added
        eEncodingIsTypedefUID,          ///< This type is pointer to a type whose UID is m_encoding_uid
        eEncodingIsPointerUID,          ///< This type is pointer to a type whose UID is m_encoding_uid
        eEncodingIsLValueReferenceUID,  ///< This type is L value reference to a type whose UID is m_encoding_uid
        eEncodingIsRValueReferenceUID,  ///< This type is R value reference to a type whose UID is m_encoding_uid
        eEncodingIsSyntheticUID
    } EncodingDataType;

    typedef enum ResolveStateTag
    {
        eResolveStateUnresolved = 0,
        eResolveStateForward    = 1,
        eResolveStateLayout     = 2,
        eResolveStateFull       = 3
    } ResolveState;

    Type (lldb::user_id_t uid,
          SymbolFile* symbol_file,
          const ConstString &name,
          uint64_t byte_size,
          SymbolContextScope *context,
          lldb::user_id_t encoding_uid,
          EncodingDataType encoding_uid_type,
          const Declaration& decl,
          const ClangASTType &clang_qual_type,
          ResolveState clang_type_resolve_state);
    
    // This makes an invalid type.  Used for functions that return a Type when they
    // get an error.
    Type();
    
    Type (const Type &rhs);

    const Type&
    operator= (const Type& rhs);

    void
    Dump(Stream *s, bool show_context);

    void
    DumpTypeName(Stream *s);


    void
    GetDescription (Stream *s, lldb::DescriptionLevel level, bool show_name);

    SymbolFile *
    GetSymbolFile()
    {
        return m_symbol_file;
    }
    const SymbolFile *
    GetSymbolFile() const
    {
        return m_symbol_file;
    }

    TypeList*
    GetTypeList();

    const ConstString&
    GetName();

    uint64_t
    GetByteSize();

    uint32_t
    GetNumChildren (bool omit_empty_base_classes);

    bool
    IsAggregateType ();

    bool
    IsValidType ()
    {
        return m_encoding_uid_type != eEncodingInvalid;
    }

    bool
    IsTypedef ()
    {
        return m_encoding_uid_type == eEncodingIsTypedefUID;
    }
    
    lldb::TypeSP
    GetTypedefType();

    const ConstString &
    GetName () const
    {
        return m_name;
    }

    ConstString
    GetQualifiedName ();

    void
    DumpValue(ExecutionContext *exe_ctx,
              Stream *s,
              const DataExtractor &data,
              uint32_t data_offset,
              bool show_type,
              bool show_summary,
              bool verbose,
              lldb::Format format = lldb::eFormatDefault);

    bool
    DumpValueInMemory(ExecutionContext *exe_ctx,
                      Stream *s,
                      lldb::addr_t address,
                      AddressType address_type,
                      bool show_types,
                      bool show_summary,
                      bool verbose);

    bool
    ReadFromMemory (ExecutionContext *exe_ctx,
                    lldb::addr_t address,
                    AddressType address_type,
                    DataExtractor &data);

    bool
    WriteToMemory (ExecutionContext *exe_ctx,
                   lldb::addr_t address,
                   AddressType address_type,
                   DataExtractor &data);

    bool
    GetIsDeclaration() const;

    void
    SetIsDeclaration(bool b);

    bool
    GetIsExternal() const;

    void
    SetIsExternal(bool b);

    lldb::Format
    GetFormat ();

    lldb::Encoding
    GetEncoding (uint64_t &count);

    SymbolContextScope *
    GetSymbolContextScope()
    {
        return m_context;
    }
    const SymbolContextScope *
    GetSymbolContextScope() const
    {
        return m_context;
    }
    void
    SetSymbolContextScope(SymbolContextScope *context)
    {
        m_context = context;
    }

    const lldb_private::Declaration &
    GetDeclaration () const;

    // Get the clang type, and resolve definitions for any 
    // class/struct/union/enum types completely.
    ClangASTType
    GetClangFullType ();

    // Get the clang type, and resolve definitions enough so that the type could
    // have layout performed. This allows ptrs and refs to class/struct/union/enum 
    // types remain forward declarations.
    ClangASTType
    GetClangLayoutType ();

    // Get the clang type and leave class/struct/union/enum types as forward
    // declarations if they haven't already been fully defined.
    ClangASTType 
    GetClangForwardType ();

    ClangASTContext &
    GetClangASTContext ();

    static int
    Compare(const Type &a, const Type &b);

    // From a fully qualified typename, split the type into the type basename
    // and the remaining type scope (namespaces/classes).
    static bool
    GetTypeScopeAndBasename (const char* &name_cstr,
                             std::string &scope,
                             std::string &basename,
                             lldb::TypeClass &type_class);
    void
    SetEncodingType (Type *encoding_type)
    {
        m_encoding_type = encoding_type;
    }

    uint32_t
    GetEncodingMask ();
    
    ClangASTType
    CreateClangTypedefType (Type *typedef_type, Type *base_type);

    bool
    IsRealObjCClass();
    
    bool
    IsCompleteObjCClass()
    {
        return m_flags.is_complete_objc_class;
    }
    
    void
    SetIsCompleteObjCClass(bool is_complete_objc_class)
    {
        m_flags.is_complete_objc_class = is_complete_objc_class;
    }

protected:
    ConstString m_name;
    SymbolFile *m_symbol_file;
    SymbolContextScope *m_context; // The symbol context in which this type is defined
    Type *m_encoding_type;
    lldb::user_id_t m_encoding_uid;
    EncodingDataType m_encoding_uid_type;
    uint64_t m_byte_size;
    Declaration m_decl;
    ClangASTType m_clang_type;
    
    struct Flags {
        ResolveState    clang_type_resolve_state : 2;
        bool            is_complete_objc_class   : 1;
    } m_flags;

    Type *
    GetEncodingType ();
    
    bool 
    ResolveClangType (ResolveState clang_type_resolve_state);
};

// these classes are used to back the SBType* objects

class TypePair {
private:
    ClangASTType clang_type;
    lldb::TypeSP type_sp;
    
public:
    TypePair () : clang_type(), type_sp() {}
    TypePair (ClangASTType type) :
    clang_type(type),
    type_sp()
    {
    }
    
    TypePair (lldb::TypeSP type) :
    clang_type(),
    type_sp(type)
    {
        clang_type = type_sp->GetClangForwardType();
    }
    
    bool
    IsValid () const
    {
        return clang_type.IsValid() || (type_sp.get() != nullptr);
    }
    
    explicit operator bool () const
    {
        return IsValid();
    }
    
    bool
    operator == (const TypePair& rhs) const
    {
        return clang_type == rhs.clang_type &&
        type_sp.get() == rhs.type_sp.get();
    }
    
    bool
    operator != (const TypePair& rhs) const
    {
        return clang_type != rhs.clang_type ||
        type_sp.get() != rhs.type_sp.get();
    }
    
    void
    Clear ()
    {
        clang_type.Clear();
        type_sp.reset();
    }
    
    ConstString
    GetName () const
    {
        if (type_sp)
            return type_sp->GetName();
        if (clang_type)
            return clang_type.GetTypeName();
        return ConstString ();
    }
    
    void
    SetType (ClangASTType type)
    {
        type_sp.reset();
        clang_type = type;
    }
    
    void
    SetType (lldb::TypeSP type)
    {
        type_sp = type;
        clang_type = type_sp->GetClangForwardType();
    }
    
    lldb::TypeSP
    GetTypeSP () const
    {
        return type_sp;
    }
    
    ClangASTType
    GetClangASTType () const
    {
        return clang_type;
    }
    
    ClangASTType
    GetPointerType () const
    {
        if (type_sp)
            return type_sp->GetClangLayoutType().GetPointerType();
        return clang_type.GetPointerType();
    }
    
    ClangASTType
    GetPointeeType () const
    {
        if (type_sp)
            return type_sp->GetClangFullType().GetPointeeType();
        return clang_type.GetPointeeType();
    }
    
    ClangASTType
    GetReferenceType () const
    {
        if (type_sp)
            return type_sp->GetClangLayoutType().GetLValueReferenceType();
        return clang_type.GetLValueReferenceType();
    }

    ClangASTType
    GetTypedefedType () const
    {
        if (type_sp)
            return type_sp->GetClangFullType().GetTypedefedType();
        return clang_type.GetTypedefedType();
    }

    ClangASTType
    GetDereferencedType () const
    {
        if (type_sp)
            return type_sp->GetClangFullType().GetNonReferenceType();
        return clang_type.GetNonReferenceType();
    }
    
    ClangASTType
    GetUnqualifiedType () const
    {
        if (type_sp)
            return type_sp->GetClangLayoutType().GetFullyUnqualifiedType();
        return clang_type.GetFullyUnqualifiedType();
    }
    
    ClangASTType
    GetCanonicalType () const
    {
        if (type_sp)
            return type_sp->GetClangFullType().GetCanonicalType();
        return clang_type.GetCanonicalType();
    }
    
    clang::ASTContext *
    GetClangASTContext () const
    {
        return clang_type.GetASTContext();
    }
};
    
class TypeImpl
{
public:
    
    TypeImpl();
    
    ~TypeImpl () {}
    
    TypeImpl(const TypeImpl& rhs);
    
    TypeImpl (lldb::TypeSP type_sp);
    
    TypeImpl (ClangASTType clang_type);
    
    TypeImpl (lldb::TypeSP type_sp, ClangASTType dynamic);
    
    TypeImpl (ClangASTType clang_type, ClangASTType dynamic);
    
    TypeImpl (TypePair pair, ClangASTType dynamic);

    void
    SetType (lldb::TypeSP type_sp);
    
    void
    SetType (ClangASTType clang_type);
    
    void
    SetType (lldb::TypeSP type_sp, ClangASTType dynamic);
    
    void
    SetType (ClangASTType clang_type, ClangASTType dynamic);
    
    void
    SetType (TypePair pair, ClangASTType dynamic);
    
    TypeImpl&
    operator = (const TypeImpl& rhs);
    
    bool
    operator == (const TypeImpl& rhs) const;
    
    bool
    operator != (const TypeImpl& rhs) const;
    
    bool
    IsValid() const;
    
    explicit operator bool () const;
    
    void Clear();
    
    ConstString
    GetName ()  const;
    
    TypeImpl
    GetPointerType () const;
    
    TypeImpl
    GetPointeeType () const;
    
    TypeImpl
    GetReferenceType () const;
    
    TypeImpl
    GetTypedefedType () const;

    TypeImpl
    GetDereferencedType () const;
    
    TypeImpl
    GetUnqualifiedType() const;
    
    TypeImpl
    GetCanonicalType() const;
    
    ClangASTType
    GetClangASTType (bool prefer_dynamic);
    
    clang::ASTContext *
    GetClangASTContext (bool prefer_dynamic);
    
    bool
    GetDescription (lldb_private::Stream &strm, 
                    lldb::DescriptionLevel description_level);
    
private:
    TypePair m_static_type;
    ClangASTType m_dynamic_type;
};

class TypeListImpl
{
public:
    TypeListImpl() :
        m_content() 
    {
    }
    
    void
    Append (const lldb::TypeImplSP& type)
    {
        m_content.push_back(type);
    }
    
    class AppendVisitor
    {
    public:
        AppendVisitor(TypeListImpl &type_list) :
            m_type_list(type_list)
        {
        }
        
        void
        operator() (const lldb::TypeImplSP& type)
        {
            m_type_list.Append(type);
        }
        
    private:
        TypeListImpl &m_type_list;
    };
    
    void
    Append (const lldb_private::TypeList &type_list);

    lldb::TypeImplSP
    GetTypeAtIndex(size_t idx)
    {
        lldb::TypeImplSP type_sp;
        if (idx < GetSize())
            type_sp = m_content[idx];
        return type_sp;
    }
    
    size_t
    GetSize()
    {
        return m_content.size();
    }
    
private:
    std::vector<lldb::TypeImplSP> m_content;
};
    
class TypeMemberImpl
{
public:
    TypeMemberImpl () :
        m_type_impl_sp (),
        m_bit_offset (0),
        m_name (),
        m_bitfield_bit_size (0),
        m_is_bitfield (false)

    {
    }

    TypeMemberImpl (const lldb::TypeImplSP &type_impl_sp, 
                    uint64_t bit_offset,
                    const ConstString &name,
                    uint32_t bitfield_bit_size = 0,
                    bool is_bitfield = false) :
        m_type_impl_sp (type_impl_sp),
        m_bit_offset (bit_offset),
        m_name (name),
        m_bitfield_bit_size (bitfield_bit_size),
        m_is_bitfield (is_bitfield)
    {
    }
    
    TypeMemberImpl (const lldb::TypeImplSP &type_impl_sp, 
                    uint64_t bit_offset):
        m_type_impl_sp (type_impl_sp),
        m_bit_offset (bit_offset),
        m_name (),
        m_bitfield_bit_size (0),
        m_is_bitfield (false)
    {
        if (m_type_impl_sp)
            m_name = m_type_impl_sp->GetName();
    }

    const lldb::TypeImplSP &
    GetTypeImpl ()
    {
        return m_type_impl_sp;
    }

    const ConstString &
    GetName () const
    {
        return m_name;
    }
    
    uint64_t
    GetBitOffset () const
    {
        return m_bit_offset;
    }

    uint32_t
    GetBitfieldBitSize () const
    {
        return m_bitfield_bit_size;
    }

    void
    SetBitfieldBitSize (uint32_t bitfield_bit_size)
    {
        m_bitfield_bit_size = bitfield_bit_size;
    }

    bool
    GetIsBitfield () const
    {
        return m_is_bitfield;
    }
    
    void
    SetIsBitfield (bool is_bitfield)
    {
        m_is_bitfield = is_bitfield;
    }

protected:
    lldb::TypeImplSP m_type_impl_sp;
    uint64_t m_bit_offset;
    ConstString m_name;
    uint32_t m_bitfield_bit_size; // Bit size for bitfield members only
    bool m_is_bitfield;
};

    
///
/// Sometimes you can find the name of the type corresponding to an object, but we don't have debug
/// information for it.  If that is the case, you can return one of these objects, and then if it
/// has a full type, you can use that, but if not at least you can print the name for informational
/// purposes.
///

class TypeAndOrName
{
public:
    TypeAndOrName ();
    TypeAndOrName (lldb::TypeSP &type_sp);
    TypeAndOrName (const ClangASTType &clang_type);
    TypeAndOrName (const char *type_str);
    TypeAndOrName (const TypeAndOrName &rhs);
    TypeAndOrName (ConstString &type_const_string);
    
    TypeAndOrName &
    operator= (const TypeAndOrName &rhs);
    
    bool
    operator==(const TypeAndOrName &other) const;
    
    bool
    operator!=(const TypeAndOrName &other) const;
    
    ConstString GetName () const;
    
    lldb::TypeSP
    GetTypeSP () const
    {
        return m_type_pair.GetTypeSP();
    }
    
    ClangASTType
    GetClangASTType () const
    {
        return m_type_pair.GetClangASTType();
    }
    
    void
    SetName (const ConstString &type_name);
    
    void
    SetName (const char *type_name_cstr);
    
    void
    SetTypeSP (lldb::TypeSP type_sp);
    
    void
    SetClangASTType (ClangASTType clang_type);
    
    bool
    IsEmpty () const;
    
    bool
    HasName () const;
    
    bool
    HasTypeSP () const;
    
    bool
    HasClangASTType () const;
    
    bool
    HasType () const
    {
        return HasTypeSP() || HasClangASTType();
    }
    
    void
    Clear ();
    
    explicit operator bool ()
    {
        return !IsEmpty();
    }
    
private:
    TypePair m_type_pair;
    ConstString m_type_name;
};
    
} // namespace lldb_private

#endif  // liblldb_Type_h_

