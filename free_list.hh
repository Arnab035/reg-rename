#ifndef __CPU_O3_FREE_LIST_HH__
#define __CPU_O3_FREE_LIST_HH__

#include <queue>

#include "regfile.hh"


namespace workflow
{
/**
 * Free list for a single class of registers (e.g., integer
 * or floating point).  Because the register class is implicitly
 * determined by the rename map instance being accessed, all
 * architectural register index parameters and values in this class
 * are relative (e.g., %fp2 is just index 2).
 */
class SimpleFreeList
{
  private:

    /** The actual free list */
    std::queue<PhysRegIdPtr> freeRegs;

  public:

    SimpleFreeList() {};

    /** Add a physical register to the free list */
    void addReg(PhysRegIdPtr reg) { freeRegs.push(reg); }

    /** Add physical registers to the free list */
    template<class InputIt>
    void
    addRegs(InputIt first, InputIt last) {
        std::for_each(first, last, [this](typename InputIt::value_type& reg) {
            freeRegs.push(&reg);
        });
    }

    /** Get the next available register from the free list */
    PhysRegIdPtr getReg()
    {
        assert(!freeRegs.empty());
        PhysRegIdPtr free_reg = freeRegs.front();
        freeRegs.pop();
        return free_reg;
    }

    /** Return the number of free registers on the list. */
    unsigned numFreeRegs() const { return freeRegs.size(); }

    /** True iff there are free registers on the list. */
    bool hasFreeRegs() const { return !freeRegs.empty(); }
};
}
#endif
