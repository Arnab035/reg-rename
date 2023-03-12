#include "rename_map.hh"

namespace workflow {

RenameMap::RenameMap() : freeList(NULL)
{
}

void RenameMap::init(const RegClass &reg_class, SimpleFreeList *_freeList) {
    assert(freeList == NULL);
    assert(map.empty());

    map.resize(reg_class.numRegs());
    freeList = _freeList;
}

RenameMap::RenameInfo
RenameMap::rename(const RegId& arch_reg)
{
    PhysRegIdPtr renamed_reg;
    // Record the current physical register that is renamed to the
    // requested architected register.
    PhysRegIdPtr prev_reg = map[arch_reg.index()];

    if (arch_reg.is(InvalidRegClass)) {
        assert(prev_reg->is(InvalidRegClass));
        renamed_reg = prev_reg;
    } else if (prev_reg->getNumPinnedWrites() > 0) {
        // Do not rename if the register is pinned
        assert(arch_reg.getNumPinnedWrites() == 0);  // Prevent pinning the
                                                     // same register twice
        renamed_reg = prev_reg;
        renamed_reg->decrNumPinnedWrites();
    } else {
        renamed_reg = freeList->getReg();
        map[arch_reg.index()] = renamed_reg;
        renamed_reg->setNumPinnedWrites(arch_reg.getNumPinnedWrites());
    }
    std::cout << "Renamed reg " << arch_reg << " to physical reg " << renamed_reg->flatIndex()  << 
	    " old mapping was " << prev_reg->flatIndex() << std::endl;
    return RenameInfo(renamed_reg, prev_reg);
}

}
