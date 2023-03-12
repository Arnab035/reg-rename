#ifndef __RENAME__MAP__
#define __RENAME__MAP__

#include <vector>

#include "reg_class.hh"
#include "free_list.hh"

namespace workflow
{

class RenameMap
{
  private:
    using Arch2PhysMap = std::vector<PhysRegIdPtr>;
    Arch2PhysMap map;

  public:
    using iterator = Arch2PhysMap::iterator;
    using const_iterator = Arch2PhysMap::const_iterator;

  private:
    /* Pointer to the free list from which physical
     * registers will be allocated in rename()
     */
    SimpleFreeList *freeList;

  public:
    RenameMap(); // default constructor

    void init(const RegClass& reg_class, SimpleFreeList *_freeList);

    typedef std::pair<PhysRegIdPtr, PhysRegIdPtr> RenameInfo;
    /**
     * Tell rename map to get a new free physical register to remap
     * the specified architectural register.
     * @param arch_reg The architectural register to remap.
     * @return A RenameInfo pair indicating both the new and previous
     * physical registers.
     */
    RenameInfo rename(const RegId& arch_reg);

    /**
     * Look up the physical register mapped to an architectural register.
     * @param arch_reg The architectural register to look up.
     * @return The physical register it is currently mapped to.
     */
    PhysRegIdPtr
    lookup(const RegId& arch_reg) const
    {
        assert(arch_reg.index() <= map.size());
        return map[arch_reg.index()];
    }

    /**
     * Update rename map with a specific mapping.  Generally used to
     * roll back to old mappings on a squash.
     * @param arch_reg The architectural register to remap.
     * @param phys_reg The physical register to remap it to.
     */
    void
    setEntry(const RegId& arch_reg, PhysRegIdPtr phys_reg)
    {
        assert(arch_reg.index() <= map.size());
        map[arch_reg.index()] = phys_reg;
    }

    /** Return the number of free entries on the associated free list. */
    unsigned numFreeEntries() const { return freeList->numFreeRegs(); }

    size_t numArchRegs() const { return map.size(); }

    /** Forward begin/cbegin to the map. */
    /** @{ */
    iterator begin() { return map.begin(); }
    const_iterator begin() const { return map.begin(); }
    const_iterator cbegin() const { return map.cbegin(); }
    /** @} */

    /** Forward end/cend to the map. */
    /** @{ */
    iterator end() { return map.end(); }
    const_iterator end() const { return map.end(); }
    const_iterator cend() const { return map.cend(); }
    /** @} */
};
}

#endif
