#include "regfile_o3.hh"

namespace workflow
{

PhysRegFile::PhysRegFile(unsigned _numCapIntRegs,
		const RegClass &reg_class)
	: capRegFile(reg_class, _numCapIntRegs)
{
    RegIndex phys_reg;
    RegIndex flat_reg_idx = 0;

    // The initial batch of registers are the capability ones
    for (phys_reg = 0; phys_reg < _numCapIntRegs; phys_reg++) {
        capRegIds.emplace_back(reg_class,
                phys_reg, flat_reg_idx++);
    }
}

PhysRegFile::IdRange
PhysRegFile::getCapRegIds()
{
    return std::make_pair(capRegIds.begin(),
                          capRegIds.end());
}

}
