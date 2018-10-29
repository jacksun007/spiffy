/*
 * libfs.h
 *
 * Header for file-system agnostic interface - latest version for FAST18
 *
 * Kuei (Jack) Sun
 * kuei.sun@mail.utoronto.ca
 *
 * University of Toronto
 * 2017
 */
 
#ifndef LIBFS_H
#define LIBFS_H

#ifdef __KERNEL__
#include <usercompat.h>
#else /* USERSPACE */
#include <kerncompat.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <vector>
#endif

namespace FS
{
    enum TypeID
    {
        INVALID_TYPE_ID,    // this type is not valid
        UNKNOWN_TYPE_ID,    // the type is unknown (may or may not be valid)
        PRIORITY_TYPE_ID,   // prioritized data block type
        NUM_GENERIC_IDS     // actual metadata ids start from this value
    };
    
    enum Error
    {
        /* standard errno also be used here */
    
        /* all error values must be negative */
        
        ERR_UNIMP = -10000,      /* unimplemented feature */
        ERR_CORRUPT = -10001,    /* corrupted value */
        ERR_BUF2SM = -10002,     /* buffer too small */
        ERR_UNINIT = -10003,     /* object not initialized */
    }; 
    
    class Entity;
    class Field;
    class FileSystem;
    
    struct Visitor
    {
        Visitor() {}
        virtual ~Visitor() {}
        
        virtual int visit(Entity & ent) { 
            (void)ent;
            return ERR_UNIMP; 
        }
        
        virtual int diff(const Entity & p, const Field & a, const Field & b) {
            (void)p;
            (void)a;
            (void)b;
            return ERR_UNIMP;
        }
    };
    
    struct Serializer
    {
        Serializer() {}
        virtual int post_process(Entity & ent, char * buf, unsigned len)=0;
        virtual ~Serializer() {}
    };
    
    struct Location
    {
        int aspc;
        unsigned size;
        unsigned offset;
        unsigned short dynamic;
        unsigned short len;
        union {
            unsigned long addr;
            char * addrptr;
        };
        
        template<typename T>
        T get_address() const { return (T)this->addr; }
       
        template<typename T>
        void set_address(T val, unsigned len=sizeof(T)) {
            if (dynamic)
                delete this->addrptr;
        
            this->len = len;
            this->addr = (unsigned long)val;
            this->dynamic = 0;
        }
        
        Location() : aspc(0), size(0), offset(0), dynamic(0), len(0), 
            addr(0) {}
        Location(int as, unsigned s, unsigned o, unsigned long ad) :
            aspc(as), size(s), offset(o), dynamic(0), len(sizeof(char *)), 
            addr(ad) {}
        
        ~Location() {
            if (this->dynamic)
                delete this->addrptr;
        }
    };
    
    template<> const char * Location::get_address<const char *>() const;
    template<> void Location::set_address(const char * val, unsigned len);
    
    class Path
    {
        FileSystem * filsys;
    
    public:
        char *   buffer;
        unsigned length;
    
        Path(FileSystem * fs) : filsys(fs), buffer(nullptr), length(0) {}
        virtual ~Path() {}
        
        FileSystem * get_file_system() const { return filsys; }
    };
    
    enum TypeFlag
    {
        TF_NONE       = 0x00000,
        TF_INTEGRAL   = 0x00001,
        TF_POINTER    = 0x00002,
        TF_ARRAY      = 0x00004,
        TF_STRUCT     = 0x00008,
        TF_UNION      = 0x00010,
        TF_EXTENT     = 0x00020,   /* to differentiate from container */
        TF_FIELD      = 0x00040,   /* in case of embedded object */
        TF_CSTRING    = 0x00080,
        TF_BIGENDIAN  = 0x00100,
        TF_SIGNED     = 0x00200,
        TF_IMPLICIT   = 0x00400,
        TF_UUID       = 0x00800,
        TF_TIMESTAMP  = 0x01000,
        TF_ELEMENT    = 0x02000,   /* element of an array */
        TF_BITFIELD   = 0x04000,
        TF_ENUM       = 0x08000,
        TF_DATA       = 0x10000,
        TF_OFFSET     = 0x20000,   /* offset pointer field */
        TF_SUPER      = 0x40000,
        TF_POSTPROC   = 0x80000,   /* post-process this field */
    };
    
    class Container;
    class Object;
    class Pointer;
    
    class Nominal {
        const char * name;
        
    protected:
        void set_name(const char * name) { this->name = name; } 
     
    public:
        Nominal(const char * n) : name(n) {}
        const char * get_name() const { return this->name; }   
    };
    
    static const int INVALID_ENTITY = -1;
    
    class Entity : public Nominal
    {  
        const char * type;
        int index;
        int flags;
        
    protected:
        void set_type(const char * type) { this->type = type; }
        void set_index(int idx) { this->index = idx; }
        
        void set_flags(int flags) { this->flags |= flags; }
        void clear_flags(int flags) { this->flags &= ~flags; }
        
    public:
        Entity(const char * t, int f, const char * n="unknown", int i=INVALID_ENTITY) 
            : Nominal(n), type(t), index(i), flags(f) {}
        Entity() : Nominal("unknown"), type("null"), index(INVALID_ENTITY), 
            flags(0) {}
        virtual ~Entity() {}
        
        const char * get_type() const { return this->type; }
        int get_index() const { return this->index; }
        
        /* use these to perform type-safe casting */
        virtual Field * to_field() { return nullptr; }
        virtual Container * to_container() { return nullptr; }
        virtual Object * to_object() { return nullptr; }
        virtual Pointer * to_pointer() { return nullptr; }
        
        bool is_valid() const { return index >= 0; }
        bool is_integral() const { return flags & TF_INTEGRAL; }
        bool is_pointer() const { return flags & TF_POINTER; }
        bool is_array() const { return flags & TF_ARRAY; }
        bool is_struct() const { return flags & TF_STRUCT; }
        bool is_union() const { return flags & TF_UNION; }
        bool is_extent() const { return flags & TF_EXTENT; }
        bool is_field() const { return flags & TF_FIELD; }
        bool is_cstring() const { return flags & TF_CSTRING; }
        bool is_big_endian() const { return flags & TF_BIGENDIAN; }
        bool is_signed() const { return flags & TF_SIGNED; }
        bool is_implicit() const { return flags & TF_IMPLICIT; }
        bool is_uuid() const { return flags & TF_UUID; }
        bool is_timestamp() const { return flags & TF_TIMESTAMP; }
        bool is_element() const { return flags & TF_ELEMENT; }
        bool is_bitfield() const { return flags & TF_BITFIELD; }
        bool is_enum() const { return flags & TF_ENUM; }
        bool is_offset() const { return flags & TF_OFFSET; }
        bool is_super() const { return flags & TF_SUPER; }
        bool is_aggregate() const { 
            return flags & ( TF_BITFIELD | TF_ARRAY | TF_STRUCT | TF_OFFSET | TF_UNION ); 
        }
        bool can_post_process() const { return flags & TF_POSTPROC; }

        void set_element() { flags |= TF_ELEMENT; }
        void post_process() { flags |= TF_POSTPROC; }

        virtual unsigned get_size() const=0;
        
        virtual int parse(const char * buf, unsigned len) {
            (void)buf; (void)len;
            return ERR_UNIMP; 
        }
        
        virtual int accept_fields(Visitor & visitor) {
            (void)visitor;
            return 0; 
        }
        
        virtual int accept_pointers(Visitor & visitor) {
            (void)visitor;
            return 0; 
        }
        
        virtual int accept_by_type(int type, Visitor & visitor) { 
            (void)visitor;
            return 0; 
        }
        
        virtual Entity * get_field_by_name(const char * name) { 
            (void)name;
            return nullptr;
        }
        
        virtual void resolve(void) {}
    };
    
    enum SaveOptions
    {
        SO_NO_ALLOC = 0x0001,
    };
    
    class Container : public Entity
    {
    protected:
        Location location;
        int type_id = INVALID_TYPE_ID;
        
    private:
        mutable int refcount;
        Path * path;
           
    protected:
        Container(const Location & loc, Path * p, const char * t, int f, 
            const char * n="", int i=0)
            : Entity(t, f, n, i), location(loc), refcount(1), path(p) {}         
        void set_size(unsigned sz) { location.size = sz; }
            
    public:
        Container(Container & rhs) = delete;
        Container(Path * p, const char * t="null", int f=0) 
            : Entity(t, f), refcount(1), path(p) {}   
        virtual ~Container() override {}
    
        Container & get_parent() { return *this; }
        Path * get_path() { return path; }
        const Location & get_location() const { return location; }
        
        virtual unsigned get_size() const override { return location.size; }
        virtual Container * to_container() final override { return this; }
      
        void incref() const;
        void decref() const;
        
        void destroy() { decref(); }
        int save(int options=0);
                    
        virtual int serialize(char * buf, unsigned len, int options=0) {
            (void)buf; (void)len; (void)options;
            return ERR_UNIMP;
        }
    };
    
    class Field : public Entity
    {
    public:
        Field(const char * t, int f, const char * n="", int i=0) :
            Entity(t, f, n, i) {} 
        virtual ~Field() {}
        
        virtual Field * to_field() final override { return this; }
        virtual const char * to_string(char * buf=nullptr, unsigned len=0) const 
            { return ""; }
        virtual unsigned long to_integer() const { return 0; }
    };
    
    class Object : public Entity
    {
        static Container nullpt;
        unsigned size = 0;

    private:
        Path * path;
    
    protected:
        Container & parent;

        Object(Container & pt, Path * p, const char * type, int flags,
            const char * name="", int index=0)
            : Entity(type, flags, name, index), path(p), parent(pt) {}
        void set_size(unsigned sz) { this->size = sz; }
            
    public:
        Object(Path * p, const char * t, int f) : Entity(t, f), path(p), parent(nullpt) {}
         
        virtual ~Object() override {}   
        
        virtual unsigned get_size() const final override { return this->size; }
        virtual Object * to_object() final override { return this; }
        
        Container & get_parent() { return parent; }
        Path * get_path() { return path; }
        const Location & get_location() const { return parent.get_location(); }
        
        void incref() const { parent.incref(); }
        void decref() const { parent.decref(); }
    };
    
    class Bit : public Field
    {
    protected:
        unsigned char bit;

    public:
        Bit() : Field("bit", TF_INTEGRAL), bit(0) {}
        Bit(bool bit, int idx) : Field("bit", TF_INTEGRAL, "", idx),
            bit((bit) ? 1 : 0) {}
        Bit(bool bit, const char * name) : Field("bit", TF_INTEGRAL, name),
            bit((bit) ? 1 : 0) {}
            
        Bit & operator=(Bit && rhs);
        operator bool() const { return bit ? true : false; }
        
        virtual unsigned get_size() const final override { return 1; }
        virtual unsigned long to_integer() const final override { return bit; }
        virtual const char * to_string(char * buf=nullptr, unsigned len=0) const 
            final override;

    };
    
    template<typename P> 
    class Bitmap : public P
    {
    protected:
        char * buf = nullptr;
        virtual ~Bitmap()
        {
            if (buf)
                delete [] buf;
        }
        
    public:
        using P::P;
        using P::get_size;
        
        const char * get_buffer() const { return buf; }
        
        bool operator[] (int idx) const {
            int max_idx = get_size() * 8;
            int byte = idx / 8;
            int bit = idx % 8;
            assert(idx >= 0 && idx <= max_idx);
            assert(buf);
            return (bool)(buf[byte] & (1 << bit));
        }
        
        /* if P = Field, you need to override get_size */
        virtual int parse(const char * ibf, unsigned len) override
        {
            unsigned size;

            if (buf) {
                delete [] buf;
                buf = nullptr;
            }

            size = get_size();
            if (len >= size) {
                buf = new char[size];
                memcpy(buf, ibf, size);
            }
            else {
                return ERR_BUF2SM;
            }
            
            return size;
        }

        int serialize(FileSystem * fs, char * obf, unsigned len, int options=0)
        {
            unsigned size = get_size();

            if (len < size)
                return ERR_BUF2SM;
                
            (void)fs;
            memcpy(obf, buf, size);
            return size;
        }
    
        virtual int accept_fields(Visitor & visitor) override
        {
            unsigned i, j, size = get_size();
            int ret = 0;
            
            for (i = 0; i < size; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    const unsigned n = i * 8 + j;
                    Bit bit(buf[i] & (1 << j), n);
                    if ((ret = visitor.visit(bit)) != 0)
                        return ret;
                }
            }
            
            return ret;
        }

        int compare(const Bitmap& other, Visitor& v) const {

            unsigned i, j, size = get_size();

            assert(size != other.get_size());

            const char* buf_other = other.get_buffer();

            bool bit_this;
            bool bit_other;

            int ret = 0;

            for (i = 0; i < size; i++) {

                for (j = 0; j < 8; j++) {

                    const unsigned n = i * 8 + j;

                    bit_this = buf[i] & (1 << j);
                    bit_other = buf_other[i] & (1 << j);

                    if (bit_this != bit_other) {

                        Bit bit_This(buf[i] & (1 << j), n);
                        Bit bit_Other(buf_other[i] & (1 << j), n);

                        ret = v.diff(*this, bit_This, bit_Other);

                        if (ret != 0)
                            return ret;

                    }

                }

            }

            return 0;

        }

    };
    
    class Buffer : public Field
    {
    protected:
        char * buf = nullptr;
        unsigned size = 0;
    
    public:
        Buffer(const char * t, int f, const char * n="", int i=0) :
            Field(t, f, n, i) {}
        Buffer(Buffer && rhs);    
        Buffer(Buffer & rhs) = delete;
        virtual ~Buffer();
            
        virtual unsigned get_size() const final override { return size; }    
        virtual const char * to_string(char * ib=nullptr, unsigned len=0) 
            const final override { return buf; }
        virtual unsigned long to_integer() const final override;

        int compare(const Buffer& other, const Entity& p, Visitor& v) const {

            if (size != other.get_size()) {
                return v.diff(p, *this, other);
            }

            if (memcmp(buf, other.buf, size)) {
                return v.diff(p, *this, other);
            }

            return 0;

        }
        
        operator const char *() { return buf; }
    };

    class IO : public Nominal
    {
    public:
        using Nominal::Nominal;
        IO() : Nominal("null") {}
        
        virtual int read(const Location & loc, char * & buf) {
            return ERR_UNIMP;
        }
        
        virtual int write(const Location & loc, const char * buf) {
            return ERR_UNIMP;
        }
        
        virtual int alloc(Location & loc, int type) {
            return ERR_UNIMP;
        }
            
        virtual ~IO() {}
    };
    
    class FileSystem : public Nominal
	{
	    static IO nio;
	    Serializer * serializer;
	
	public:
	    IO & io;
	    
	    FileSystem(const char * n) : Nominal(n), serializer(nullptr), 
	        io(nio) {}
		FileSystem(const char * n, IO & io, Serializer * s=nullptr) 
		    : Nominal(n), serializer(s), io(io) {}
        virtual ~FileSystem() {}
        
        virtual Container * fetch_super() const = 0;
        virtual Container * parse_super(unsigned long start, const char * buf, 
                                        unsigned len) const = 0;  
        virtual Container * parse_by_type(int type, Location & loc, Path * path, 
            const char * buf, unsigned len) const = 0;
        virtual int super_type_id() const = 0;     
        virtual Container * create_container(int type, Path * path) const = 0;
        
        virtual const char * type_to_name(unsigned type) const;
        virtual const char * address_space_to_name(int aspc) const;
        
        void set_serializer(Serializer * s) { serializer = s; }
        int post_process(Entity & ent, char * buf, unsigned len);
	};
    
    struct ByteSwap {
        static u64 byteswap(u64 v) { return __builtin_bswap64(v); }
        static u32 byteswap(u32 v) { return __builtin_bswap32(v); }
        static u16 byteswap(u16 v) { return __builtin_bswap16(v); }
    };
    
    template<typename S, TypeFlag F=TF_NONE>
    class Integer : public Field
    {
        S value;
    
        template<typename ST, TypeFlag TF>
        struct Caster {
            static ST cast(ST v) { return v; }
        };
        
        template<typename ST>
        struct Caster<ST, TF_BIGENDIAN> : public ByteSwap {
            static ST cast(ST v) { return byteswap(v); }  
        };
    
    public:
        Integer(const char * type, int flags=0, const char * name="", 
            int index=0) : 
            Field(type, flags | TF_INTEGRAL | F, name, index), value(0) {}
        
        virtual unsigned get_size() const override { return sizeof(S); }
        
        int compare(const Integer<S, F> & other, const Entity & p, 
            Visitor & v) const {
            if (value != other.value) {
                return v.diff(p, *this, other);
            }            
            return 0;
        }
        
        virtual int parse(const char * buf, unsigned len) override {
            if (len >= sizeof(S)) {
                this->value = Caster<S, F>::cast(*(S *)buf);
                return sizeof(S);
            }
            
            return ERR_BUF2SM;
        }
        
        int serialize(FileSystem * filsys, char * buf, unsigned len, int options=0) {
            if (len < sizeof(S))
                return ERR_BUF2SM;      
            *(S *)buf = Caster<S, F>::cast(this->value);
            if (can_post_process() && filsys) {
                int ret;
                if ((ret = filsys->post_process(*this, buf, sizeof(S))) < 0)
                    return ret;
            }
            return sizeof(S);
        }
        
        operator S() const { return this->value; }

        S & set_value(S v) { this->value = v; return this->value; }
        
        virtual unsigned long to_integer() const override {
            return (unsigned long)value;
        }
        
        static const char * 
        to_string(S v, char * buf=nullptr, unsigned len=0) {
            static char st_buf[32];
            
            if (buf == nullptr) {
                buf = st_buf;
                len = sizeof(st_buf);
            }
            
            assert(len > 0);
            snprintf(buf, len, "%lu", (unsigned long)v);
            buf[len-1] = '\0';
            return buf;
        }
        
        virtual const char * 
        to_string(char * buf=nullptr, unsigned len=0) const override { 
            return to_string(this->value, buf, len);
        }
    };
    
    template<typename S, TypeFlag F=TF_NONE>
    class Bitfield : public Integer<S, F>
    {
    public:
        using Integer<S, F>::Integer;
    
        bool operator[](int idx)
        {
            assert(idx >= 0 && idx < (int)(sizeof(S) * 8));
            return ((S)(*this) & (1 << idx)) ? true : false;
        } 
    };
    
    enum AddrSpace
    {
        AS_NONE,
        AS_BYTE,
        
        NUM_ADDRSPACES,
    };
    
    class Pointer : public Field
    {
    protected:
        typedef Container * (Pointer::*fetch_f)() const;
        
        Location location;
        fetch_f  fetch_func = nullptr;
        int      ptr_type   = INVALID_TYPE_ID;
        bool     resolved   = false;

    public:
        Pointer(const char * type, int flags, const char * name="", 
            int index=0) 
            : Field(type, flags | TF_POINTER, name, index) {}

      	Container * fetch() const;
      	const Location & pointer_location() const { return location; }
      	unsigned pointer_type() const { return ptr_type; }
      	
      	virtual Pointer * to_pointer() final override { return this; }
      	virtual unsigned get_size() const override { return location.len; }
      	
      	virtual int parse(const char * buf, unsigned len) override=0;
        virtual void resolve(void) override=0;
    };

    template<typename S, TypeFlag F=TF_NONE>
    class IntPtr : public Pointer
    {
        template<typename ST, TypeFlag TF>
        struct Caster {
            static ST cast(ST v) { return v; }
        };
        
        template<typename ST>
        struct Caster<ST, TF_BIGENDIAN> : public ByteSwap {
            static ST cast(ST v) { return byteswap(v); }  
        };
    
    public:
        IntPtr(const char * type, int flags=0, const char * name="", 
            int index=0) : 
            Pointer(type, flags | TF_INTEGRAL | F, name, index) {
            location.len = sizeof(S);
        }
  
        virtual int parse(const char * buf, unsigned len) override {
            if (len >= sizeof(S)) {
                location.set_address(Caster<S, F>::cast(*(S *)buf));
                return sizeof(S);
            }
            
            return ERR_BUF2SM;
        }
        
        int serialize(FileSystem * filsys, char * buf, unsigned len, int options=0) {
            if (len < sizeof(S))
                return ERR_BUF2SM;
            *(S *)buf = Caster<S, F>::cast(location.get_address<S>());
            if (can_post_process() && filsys) {
                int ret;
                if ((ret = filsys->post_process(*this, buf, sizeof(S))) < 0)
                    return ret;
            }
            return sizeof(S);
        }
        
        int compare(const IntPtr<S, F> & other, const Entity & p, 
            Visitor & v) const {
            if (location.addr != other.location.addr) {
                return v.diff(p, *this, other);
            }            
            return 0;
        }
        
        operator S() const { 
            return location.get_address<S>(); 
        }
        
        bool operator>(S v) const {
            return (location.get_address<S>() > v);
        }
        
        unsigned long & set_address(const S v) { 
            location.set_address(v);
            return location.addr;
        }
        
        virtual unsigned long to_integer() const override {
            return location.addr;
        }
        
        virtual const char * 
        to_string(char * buf=nullptr, unsigned len=0) const override { 
            static char st_buf[32];
            
            if (buf == nullptr) {
                buf = st_buf;
                len = sizeof(st_buf);
            }
            
            assert(len > 0);
            snprintf(buf, len, "%lu", location.addr);
            buf[len-1] = '\0';
            return buf;
        }
    };
    
    template<typename S, TypeFlag F=TF_NONE>
    class Offset : public Integer<S, F>
    {
    protected: 
        typedef int (Offset::*create_f)();
       
        Path * path;
        Object * target;
           
    public:
        Offset(Path * p, const char * type, int flags=0, 
            const char * name="", int index=0) : 
            Integer<S, F>(type, flags | TF_OFFSET, name, index),
            path(p), target(nullptr) {}
            
        virtual ~Offset() override {
            if (target != nullptr)
                delete target;
        }
        
        Object * get() { return target; }
        
        Object & operator*() {
            assert(target);
            return *target;
        }
            
        virtual int create_target() = 0;
        virtual int save_target(FileSystem * filsys, int options) = 0;
        
        int seek_to_position(long ofs, const char * & buf, long & len) {
            if (path == nullptr)
                return -EINVAL;
            
            if ((buf = path->buffer) == nullptr)
                return -EINVAL;
                
            len = (long)path->length - ofs;
            buf += ofs;
            
            return 0;
        }
        
        virtual int parse(const char * buf, unsigned len) override {
            int ret = Integer<S, F>::parse(buf, len);   
            if (ret < 0) return ret;
            if ((ret = create_target()) <= 0)
                return ret;
            /* on success, return size of the field */
            return Integer<S, F>::get_size();
        }
        
        int serialize(FileSystem * filsys, char * buf, unsigned len, int options=0) {
            int ret = Integer<S, F>::serialize(filsys, buf, len, options);   
            if (ret < 0) return ret;
            if ((ret = save_target(filsys, options)) <= 0)
                return ret;
            return Integer<S, F>::get_size();
        }
        
        virtual int accept_fields(Visitor & visitor) override {
            int ret = 0;
        
            if (target != nullptr)
                ret = visitor.visit(*target);
                
            return ret;    
        }
        
        virtual int accept_pointers(Visitor & visitor) override {
            int ret = 0;
        
            if (target != nullptr)
                ret = target->accept_pointers(visitor);
                
            return ret;    
        }
        
        virtual void resolve() override { if (target != nullptr) target->resolve(); }
    };
    
    class Data : public Container
    {
        const char * buffer;
    
    public:
        Data() : Container(nullptr, "data", TF_DATA), buffer(nullptr) {}
        Data(const Location & lc, const Path * p, int idx=0, const char * name="") 
            : Container(lc, nullptr, "data", TF_DATA, name, idx)
            , buffer(nullptr) { (void)p; }
        
        const char * get_buffer() const { return buffer; }
            
        virtual int parse(const char * buf, unsigned len) {
            if (len < get_size())
                return ERR_BUF2SM;
        
            this->buffer = buf; 
            return get_size();
        }

        /* (spatel): support for comparing Data */
    };

    template<typename T, typename U>
    class Vector : public Container
    {
        std::vector<T *> element;

    public:
        using Container::Container;
        virtual ~Vector() override {
            typename std::vector<T *>::iterator it = element.begin();
            for( ; it != element.end(); ++it)
                delete (*it);
        }
 
        unsigned get_count() const { return (unsigned)element.size(); }
    
        T & operator[](int idx) {
            T * ptr;  
            assert(idx >= 0 && idx < (int)element.size());
            ptr = element[idx];  
            return *ptr;
        }
        
        T * get(int idx) {
            if (idx >= 0 && idx < (int)element.size()) {
                return element[idx];
            }
            return nullptr;
        }
    
        virtual int parse(const char * buf, unsigned len) override 
        {
            int idx = 0;
            /* (jsun): must be signed to prevent integer underflow */
            int remaining = (int)len;
            int minimum = (int)sizeof(U);
            int total = 0;
            Path * path = Container::get_path();
            
            if (path) {
                path->buffer = const_cast<char *>(buf);
                path->length = len;
            }
            
            while (remaining >= minimum)
            {
                int ret;
                T * tmp = new T(*this, path, idx);
                
                if (tmp == nullptr) 
                    return -ENOMEM;     
                else if ((ret = tmp->parse(buf, remaining)) < 0) {
                    delete tmp;
                    return ret;
                }
                else if (ret == 0) {
                    delete tmp;
                    return ERR_CORRUPT;
                }
                
                tmp->set_element();
                element.push_back(tmp);
                idx       += 1;
                remaining -= tmp->get_size();
                buf       += tmp->get_size();
                total     += tmp->get_size();
                if (is_sentinel(*tmp))
                    break;
            }
            
            resolve();
            if (path) {
                path->buffer = nullptr;
                path->length = 0;
            }
            
            /* 
             * (jsun): the size of a vector type is always as much as the
             * input size, even if it causes internal fragmentation
             */
            set_size(len);
            return len;           
        }
        
        virtual void resolve() override
        {
            typename std::vector<T *>::iterator it = element.begin();
            for( ; it != element.end(); ++it)
                (*it)->resolve(); 
        }
        
        virtual int serialize(char * buf, unsigned len, int options=0) override
        {
            int total_bytes = 0;
            FileSystem * fs = get_path()->get_file_system();
            typename std::vector<T *>::iterator it = element.begin();
            
            for( ; it != element.end(); ++it) {
                int bytes_written = (*it)->serialize(fs, buf, len, options);
                if (bytes_written <= 0)
                    return bytes_written;
                    
                total_bytes += bytes_written;
                buf += bytes_written;
                len -= bytes_written;
            }
            
            return total_bytes;
        }
  
        virtual int accept_fields(FS::Visitor & visitor) override
        {
            typename std::vector<T *>::iterator it = element.begin();
            int ret = 0;
            
            for( ; it != element.end(); ++it) {
                T * ptr = *it;
                if ((ret = visitor.visit(*ptr)) != 0)
                    return ret;
            }

            return ret;
        }

        virtual int accept_pointers(FS::Visitor & visitor) override
        {
            typename std::vector<T *>::iterator it = element.begin();
            int ret = 0;
            
            for( ; it != element.end(); ++it) {
                T * ptr = *it;
                if ((ret = ptr->accept_pointers(visitor)) != 0)
                    return ret;
            }

            return ret;
        }

        virtual bool is_sentinel(T & self) const 
        { 
            (void)self;
            return false; 
        }

        int compare(const Vector& other, Visitor& v) const {

            typename std::vector<T *>::iterator it_this = element.begin();
            typename std::vector<T *>::iterator it_other = other.element.begin();

            int ret = 0;

            for ( ; (it_this != element.end() && it_other != other.element.end()); ++it_this, ++it_other ) {

                if ((ret = ((**it_this).compare(**it_other, v))) != 0)
                    break;

            }

            return ret;

        }

    };

    template<typename T, typename U>
    class Array : public Entity
    {
    protected:
        std::vector<T *> element;
        unsigned size = 0;

        virtual int get_count() const { return (unsigned)(-1); }

    public:
        Array(const char * t, int f, const char * n="", int i=0) :
            Entity(t, f, n, i) {} 
        virtual ~Array() override {
            typename std::vector<T *>::iterator it = element.begin();
            for( ; it != element.end(); ++it)
                delete (*it);
        }
 
        virtual unsigned get_size() const override { return size; }
        
        T & operator[](int idx) {
            assert(idx >= 0 && idx < (int)element.size());
            return *element[idx];
        }
        
        virtual T * create_element(int idx, const char * name)=0;
    
        virtual int parse(const char * buf, unsigned len) override 
        {
            int idx = 0;
            /* (jsun): must be signed to prevent integer underflow */
            int remaining = (int)len;
            int minimum = (int)sizeof(U);
            int max_count = get_count();
 
            size = 0;
            while (remaining >= minimum && idx < max_count)
            {
                int ret;
                T * tmp = create_element(idx, this->get_name()); 
                if (tmp == nullptr) return -ENOMEM;
                
                if ((ret = tmp->parse(buf, remaining)) < 0)
                    return ret;  
                else if (ret == 0) return ERR_CORRUPT;
                
                tmp->set_element();
                element.push_back(tmp);    
                idx       += 1;
                remaining -= tmp->get_size();
                buf       += tmp->get_size();
                size      += tmp->get_size();
                
                if (is_sentinel(*tmp))
                    break;
            }
            
            resolve();  
            return size;
        }
        
        virtual void resolve() override
        {
            typename std::vector<T *>::iterator it = element.begin();
            for( ; it != element.end(); ++it)
                (*it)->resolve(); 
        }
        
        int serialize(FileSystem * fs, char * buf, unsigned len, int options=0)
        {
            int total_bytes = 0;
            typename std::vector<T *>::iterator it = element.begin();
            
            for( ; it != element.end(); ++it) {
                int bytes_written = (*it)->serialize(fs, buf, len, options);
                
                if (bytes_written < 0) return bytes_written;
                if (bytes_written == 0) return ERR_CORRUPT;
                    
                total_bytes += bytes_written;
                buf += bytes_written;
                len -= bytes_written;
            }
            
            return total_bytes;
        }
  
        virtual int accept_fields(FS::Visitor & visitor) override
        {
            typename std::vector<T *>::iterator it = element.begin();
            int ret = 0;
            
            for( ; it != element.end(); ++it) {
                T * ptr = *it;
                if ((ret = visitor.visit(*ptr)) != 0)
                    return ret;
            }

            return ret;
        }

        virtual int accept_pointers(FS::Visitor & visitor) override
        {
            typename std::vector<T *>::iterator it = element.begin();
            int ret = 0;
            
            for( ; it != element.end(); ++it) {
                T * ptr = *it;
                if ((ret = ptr->accept_pointers(visitor)) != 0)
                    return ret;
            }

            return ret;
        }

        virtual bool is_sentinel(T & self) const 
        { 
            (void)self;
            return false; 
        }

        int compare(const Array<T,U>& other, Visitor& v) const {

            typename std::vector<T *>::const_iterator it_this = element.begin();
            typename std::vector<T *>::const_iterator it_other = other.element.begin();

            int ret = 0;

            for ( ; (it_this != element.end() && it_other != other.element.end()); ++it_this, ++it_other ) {

                if ((ret = ((**it_this).compare(**it_other, v))) != 0)
                    break;

            }

            return ret;

        }
    };

    template<typename T>
    class Extent : public Container
    {
    protected:
        std::vector<T *> element;
  
        T * get_or_create(int idx) {
            assert(idx >= 0 && idx < (int)element.size());
            
            T * ret = this->element[idx];
            if (ret == nullptr)
                ret = create_element(idx);

            /* (jsun): handle this properly */
            if (ret != nullptr) {
                this->element[idx] = ret;
                ret->set_element();
            }
            
            return ret;
        }
  
        virtual ~Extent() override {
            typename std::vector<T *>::iterator it = element.begin();
            
            for ( ; it != element.end(); ++it) {
                T * & tmp = *it;
                if (tmp != nullptr)
                    tmp->destroy();
            }     
        }
        
    public:
        Extent(const Location & lc, Path * p, const char * t, int f, 
            const char * n="", int idx=0) : 
            Container(lc, p, t, f | TF_EXTENT, n, idx) {}   
        
        /*
         * need an int initialize(void) function for auto-generated
         * factory function
         */  
               
        virtual T * create_element(int idx) const=0;
        unsigned get_count() const { return (unsigned)element.size(); }
        
        T & operator[](int idx) {
            T * ctn = this->get_or_create(idx);
            assert(ctn);
            return *ctn;
        }
        
        /* use this interface to avoid crashing on parsing error */
        T * get(int idx) {
            return this->get_or_create(idx);
        }
        
        virtual int accept_fields(FS::Visitor & visitor) override
        {
            int ret = 0;
        
            for (unsigned i = 0; i < this->get_count(); ++i) {
                T * tmp = get_or_create(i);
                if (tmp != nullptr) {
                    if ((ret = visitor.visit(*tmp)) != 0)
                        return ret;
                }
                else return ERR_CORRUPT;
            }
            
            return ret;
        }

        virtual int accept_pointers(FS::Visitor & visitor) override
        {
            int ret = 0;
        
            for (unsigned i = 0; i < this->get_count(); ++i) {
                T * tmp = get_or_create(i);
                if (tmp != nullptr) {
                    if ((ret = tmp->accept_pointers(visitor)) != 0)
                        return ret;
                }
                else return ERR_CORRUPT;
            }
            
            return ret;
        }

        int compare(const Extent& other, Visitor& v) const {

            typename std::vector<T *>::iterator it_this = element.begin();
            typename std::vector<T *>::iterator it_other = other.element.begin();

            int ret = 0;

            for ( ; (it_this != element.end() && it_other != other.element.end()); ++it_this, ++it_other ) {

                if ((ret = ((**it_this).compare(**it_other, v))) != 0)
                    break;

            }

            return ret;

        }

    };
} /* namespace FS */

#endif

