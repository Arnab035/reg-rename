#ifndef __REGFILE_O3_HH__
#define __REGFILE_O3_HH__

#include <cstring>
#include <vector>

#include "regfile.hh"

#define panic(arg...) \
  do { printf("Panic: " arg); exit(1); } while (0)

namespace workflow
{
/**
 * Simple capability physical register file class.
 */
class PhysRegFile
{
  private:

    using PhysIds = std::vector<PhysRegId>;
  public:
    using IdRange = std::pair<PhysIds::iterator,
                              PhysIds::iterator>;
  private:
    /** capability register file. */
    RegFile capRegFile;
    std::vector<PhysRegId> capRegIds;

   /**
     * Number of physical general purpose registers
     */
    unsigned numPhysCapRegs;
  public:
    /**
     * Constructs a physical register file with the specified amount of
     * integer and floating point registers.
     */
    PhysRegFile(unsigned _numPhysicalCapRegs,
                const RegClass &classes);

    /**
     * Destructor to free resources
     */
    ~PhysRegFile() {}

    /* only working with capability registers */
    RegVal
    getReg(PhysRegIdPtr phys_reg) const
    {
        const RegClassType type = phys_reg->classValue();
        if (type != CapRegClass)
            panic("Only capability registers are supported!");
        const RegIndex idx = phys_reg->index();

        return capRegFile.reg(idx);
 
    }

    void
    getReg(PhysRegIdPtr phys_reg, void *val) const
    {
        const RegClassType type = phys_reg->classValue();
        if (type != CapRegClass)
            panic("Only capability registers are supported!");
        *(RegVal *)val = getReg(phys_reg);
    }

    void
    setReg(PhysRegIdPtr phys_reg, RegVal val)
    {
        const RegClassType type = phys_reg->classValue();
        if (type != CapRegClass)
            panic("Only capability registers are supported!");
        const RegIndex idx = phys_reg->index();
        capRegFile.reg(idx) = val;
    }

    void
    setReg(PhysRegIdPtr phys_reg, const void *val)
    {
        const RegClassType type = phys_reg->classValue();
        if (type != CapRegClass)
            panic("Only capability registers are supported!");
        const RegIndex idx = phys_reg->index();

        setReg(phys_reg, *(RegVal *)val);
    }

    /* only one class of registers */
    IdRange getCapRegIds();
};

}
#endif
