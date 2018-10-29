/*
 * corruptor.cpp
 *
 * Implements type-specific file system corruption
 *
 * Kuei (Jack) Sun
 * kuei.sun@mail.utoronto.ca
 *
 * University of Toronto
 * 2014
 */

#include <libfs.h>
#include <getopt.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "corruptor.h"

using namespace std;

int CorruptSerializer::fill_random(char * buf, unsigned len)
{
    int randval;

    while (len >= 4) {
        *(int *)buf = rand();
        buf += 4;
        len -= 4;
    }
    
    if (len > 0) {
        randval = rand();
        memcpy(buf, &randval, len);
    }
    
    return 0;
}

int CorruptSerializer::try_set(char * buf, unsigned len)
{   
    if (this->value == 0) {
        memset(buf, 0, len);
        return 0;
    }
    
    switch (len) {
    case 8:
        *(long *)buf = this->value;
        break;
    case 4:
        *(int *)buf = (int)this->value;
        break;
    case 2:
        *(short *)buf = (short)this->value;
        break;
    case 1:
        *(char *)buf = (char)this->value;
        break;
    default:
        return -EINVAL;
    }
    
    return 0;
}

int CorruptSerializer::try_add(FS::Entity & ent, char * buf, unsigned len)
{
    FS::Field * field = ent.to_field();
    long curval;
    
    if (field == nullptr)
        return -EINVAL;
          
    curval = (long)field->to_integer();
    curval += this->value;
    
    switch (len) {
    case 8:
        *(long *)buf = curval;
        break;
    case 4:
        *(int *)buf = (int)curval;
        break;
    case 2:
        *(short *)buf = (short)curval;
        break;
    case 1:
        *(char *)buf = (char)curval;
        break;
    default:
        return -EINVAL;
    }
    
    return 0;
}

int CorruptSerializer::post_process(FS::Entity & ent, char * buf, unsigned len)
{
    int entsize = (int)ent.get_size();
    int size = (entsize < (int)len) ? entsize : (int)len;
    int ret;
    
    switch (this->type) {
    case CT_SET:
        ret = try_set(buf, size);
        break;
    case CT_RANDOM:
        ret = fill_random(buf, size);
        break;
    case CT_ADD:
        ret = try_add(ent, buf, size);
        break;
    default:
        ret = -EINVAL;
    }   
    
    return ret;
}


class PtrVisitor : public FS::Visitor
{
    Corruptor * corruptor;
    
public:    
    PtrVisitor(Corruptor * c) : corruptor(c) {}
    virtual int visit(FS::Entity & ent) override;
};

int Corruptor::visit_container(FS::Container * ctn)
{
    int ret;
    
    num_corrupted = 0;
    ret = ctn->accept_fields(*this);

    if (ret < 0) {
        cerr << "error while traversing " << ctn->get_type() << endl;
        return ret;   
    }
    
    /* at least one of the field is marked for corruption */
    if (num_corrupted > 0) {
        const FS::Location & loc = ctn->get_location();
        ret = ctn->save(FS::SO_NO_ALLOC);
        
        if (ret < 0) {
            cout << "error while saving ";
        }
        else {
            cout << ret << " bytes written to ";
            ret = 0;
        }
        
        cout << ctn->get_type() << " at " << fs.address_space_to_name(loc.aspc)
             << " address " << loc.addr << endl;
    }
    
    if (victim.size() > 0) {
        ret = ctn->accept_pointers(*ptr_visitor);
    }
        
    return ret;
}
    
int Corruptor::visit_field(FS::Field * field)
{
    std::vector<Victim>::iterator it;
    
    if ( field->is_aggregate() ) 
	{	    
		return field->accept_fields(*this);
	}
	
	for (it = victim.begin(); it != victim.end(); )
	{
	    if (!strcmp(field->get_name(), it->name))
	    {
	        if (it->skip-- <= 0) {
	            serializer.set_victim(*it);
	            field->post_process();
	            num_corrupted++;
	            if (--it->repeat <= 0) {
	                it = victim.erase(it);
	                continue;
	            }
	        }
	    }
	    
	    it++;
	}   
	
    return 0;
}

int Corruptor::visit(FS::Entity & ent)
{
    if (ent.to_container())
        return visit_container(ent.to_container());
    else if (ent.to_field())
        return visit_field(ent.to_field());
    /* object or entity */    
    return ent.accept_fields(*this);
}

int PtrVisitor::visit(FS::Entity & ent)
{
    FS::Pointer * ptr = ent.to_pointer();
    FS::Container * ctn;
    int ret = 0;
  
    if (corruptor->size() == 0) {
        /* no more fields to corupt */
        return 0;
    }
  
    if (ptr == nullptr) {
        cerr << "error visiting non-pointer type " << ent.get_type()
             << " during accept_pointers\n";
        return FS::ERR_CORRUPT;
    }
    else if (ptr->pointer_location().aspc > FS::NUM_ADDRSPACES) {
        /* TODO: skip non-block address space for now */
        return 0;
    }

    if ((ctn = ptr->fetch()) != nullptr) {
        ret = corruptor->visit(*ctn);
        ctn->destroy();
    }
    else if (ptr->to_integer() > 0) {
        cerr << "error while fetching from pointer type " 
             << ent.get_type() << endl;
        return FS::ERR_CORRUPT;
    }
    
    return ret;
}

Corruptor::Corruptor(FS::FileSystem & fs) : 
        ptr_visitor(new PtrVisitor(this)), fs(fs), num_corrupted(0)
{
    fs.set_serializer(&this->serializer);
}
        
Corruptor::~Corruptor() 
{
    if (ptr_visitor)
        delete ptr_visitor;
}

Victim * Corruptor::add_victim(const char * n)
{
    victim.emplace_back(n);
    return &victim.back();
}

#define eprintf(fmt, args...) fprintf (stderr, fmt, ##args)

int Corruptor::run()
{
    FS::Container * super;
    int ret = 0;

    if (ptr_visitor == nullptr)
        return -ENOMEM;
    
    /* initialize random */
    srand(time(NULL));
    if ((super = fs.fetch_super()) != nullptr) {
        ret = this->visit(*super);
        super->destroy();
    }
    
    if (ret < 0) {
        eprintf("error while attempting type-specific corruption\n");
    }
    else if (victim.size() > 0) {
        eprintf("could not corrupt all specified fields\n");
        ret = EXIT_FAILURE;
    }
    
    return ret;
}

#define errx(fmt, args...) fprintf (stderr, "%s: " fmt, argv[0], ##args)

static void _print_usage(char * argv[]) 
{
    eprintf("usage: %s -n NAME [-t TYPE=set][-v VAL=0][-s NUM=0][-r NUM=1]"
            "[-h] DEVICE\n", argv[0]);
    eprintf("\t-n NAME: corrupt field with NAME\n");
    eprintf("\t-t TYPE: set, add, or random\n");
    eprintf("\t-v VAL:  corrupt field with value (set or add only)\n");
    eprintf("\t-s NUM:  skip NUM number of matches\n");
    eprintf("\t-r NUM:  repeat the corruption NUM times\n");
    eprintf("\t-h:      print this help message\n");
    eprintf("\tDEVICE:  device to corrupt (e.g. /dev/sdb1)\n");
    eprintf("Spiffy's type-specific file system corruption tool (v0.1)\n");
    exit(EXIT_FAILURE);
}

#define print_usage() _print_usage(argv)
#define error_no_victim(c) do { \
    errx("must specify name before the -%c option\n", c); \
    print_usage(); \
} while(false)

int Corruptor::process_arguments(int argc, char * argv[])
{
    Victim * victim = nullptr;
    char * endptr;    
    int c;

    opterr = 0;
    while ((c = getopt(argc, argv, "n:t:v:s:r:h")) != -1)
    switch (c)
    {
        case 'n':
            if (victim != nullptr) {
                errx("corrupting multiple fields is currently not supported\n");
                print_usage();
            }
            victim = add_victim(optarg);
            break;
        case 't':
            if (victim == nullptr)
                error_no_victim(c);
            if (!strcmp(optarg, "random"))
                victim->type = CT_RANDOM;
            else if (!strcmp(optarg, "set"))
                victim->type = CT_SET;
            else if (!strcmp(optarg, "add"))
                victim->type = CT_ADD;
            else {
                errx("unknown corruption type '%s'.\n", optarg);
                print_usage();
            }
            break;
        case 'v':
            if (victim == nullptr)
                error_no_victim(c);
            victim->value = strtol(optarg, &endptr, 10);
            if (endptr != nullptr && *endptr != '\0') {
                errx("corrupt value must be an integer (got '%s').\n",
                    optarg);
                print_usage();
            }
            break;
        case 's':
            if (victim == nullptr)
                error_no_victim(c);
            victim->skip = strtol(optarg, &endptr, 10);
            if ((endptr != nullptr && *endptr != '\0') || victim->skip < 0) {
                errx("skip must be a non-negative integer (got '%s').\n",
                    optarg);
                print_usage();
            }
            break;   
        case 'r':
            if (victim == nullptr)
                error_no_victim(c);
            victim->repeat = strtol(optarg, &endptr, 10);
            if ((endptr != nullptr && *endptr != '\0') || victim->repeat <= 0) {
                errx("repeat must be a positive integer (got '%s').\n",
                    optarg);
                print_usage();
            }
            break;                     
        case '?':
            if (strchr("ntvsr", optopt) != nullptr)
                errx("option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                errx("unknown option '-%c'.\n", optopt);
            else
                errx("unknown option character '\\x%x'.\n",
                   optopt);
        default:
            print_usage();
    }

    if (argc - optind != 1) {
        errx("missing device name\n");
        print_usage();
    }
    
    return optind;
}

