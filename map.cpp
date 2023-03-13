#include <iostream>
#include <map>
#include <vector>

#include "reg_class.hh"
#include "rename_map.hh"
#include "regfile_o3.hh"

using namespace std;

using namespace workflow;

class CAM
{
  private:
    const uint16_t _max_size = 512;

  public:
    uint16_t getMaxSize() {
      return _max_size;
    }
    // we change this to Addr and register number, 
    // whatever their formats are
    // keeping them to <int, RegIdPtr> now
    typedef std::map<int, RegIdPtr> cam_map_t;
    void add( int key, RegIdPtr value ) {
        cam_map_t::const_iterator it = m_table.find( key );
        if ( m_table.size() == _max_size ) { 
            // handle error, throw exception...
            cout << "CAM is full. Cannot add!" << endl;
        } else {
            m_table.insert( it, std::make_pair(key, value) );
        }
    }

    void loop() {
    	cam_map_t::const_iterator it;
	cout << "Displaying CAM contents now (KEY: VALUE)..." << endl;
	for (it = m_table.begin(); it != m_table.end(); it++) {
            cout << it->first
                 << ": "
                 << *(it->second)
                 << endl;
        }
    }

    // given the key, find value
    RegIdPtr find( int key ) {
        cam_map_t::const_iterator it = m_table.find( key );
        if (it == m_table.end()) {
            return nullptr; // handle at the caller
        } else {
            return it->second;
        }
    }
  private:
    cam_map_t m_table;
};

uint32_t getCacheLineNumber(uint32_t cap) {
    // for l1 cache, max no. of cache blocks = 2^9 = 512.
    return (((1 << 9) - 1) & (cap >> (9 - 1)));
}

/* i is the cache line number */
uint32_t constructCapability(int i) {
    // assuming non-secure class of service
    // and a first-level cache
    uint32_t access_rights = 0b00011111;
    uint32_t cache_level = 0b01;
    return cache_level << 17 | (i << 8 | access_rights);
}

int main() {

    CAM cam{};
    // RegIndex is uint16_t //
    uint16_t size = cam.getMaxSize();

    RegClass capRegClass(CapRegClass, "capability", size, debug::CapRegs);
    RegIndex i{0};
    RegIdPtr reg;
    for (i = 0; i < size; i++) {
        cam.add(i, new RegId(capRegClass, i));
    }
    // Check all values
    cam.loop();
    // define free list of cap registers
    // sets up capability physical register file
    PhysRegFile regFile{size, capRegClass};
    // sets up free list
    SimpleFreeList freeList{};

    PhysRegIdPtr physReg;

    PhysRegFile::IdRange capIdRange = regFile.getCapRegIds();

    freeList.addRegs(capIdRange.first, capIdRange.second);
    // do some renaming (map arch register to physical register)
    RenameMap rmap{};
    // associate free list of physical registers to rename map
    rmap.init(capRegClass, &freeList);
    // have an initial mapping of architectural regs to physical regs
    // we can rename later
    for (auto i = 0; i < size/4; i++) {
        rmap.setEntry(*cam.find(i), freeList.getReg());
    }

    // demonstrate rename now

    for (auto i = 0; i < size/4; i++) {
        rmap.rename(*cam.find(i));
    }
    // insert capability (load capability values into register)
    for (auto i = 0; i < size/4; i++) {
        physReg = rmap.lookup(*cam.find(i));
        uint32_t cap = constructCapability(i);
        regFile.setReg(physReg, cap);
    }

    // check capability (read capability values from register)

    for (auto i = 0; i < size/4; i++) {
        physReg = rmap.lookup(*cam.find(i));
        cout << "Capability value stored in physical register "
             << physReg->flatIndex()
             << " mapped to architectural register " << *cam.find(i)
             << " is " << regFile.getReg(physReg) << endl;
    }
    // get line number (get cache line number from register)
    for (auto i = 0; i < size/4; i++) {
        physReg = rmap.lookup(*cam.find(i));
        cout << "Cache line number mapped to capability value "
             << regFile.getReg(physReg) << " is " 
             << getCacheLineNumber(regFile.getReg(physReg)) << endl;
    }
    // free architectural registers in CAM
    for (auto i = 0; i < size; i++) {
        reg = cam.find(i);
        if (reg != nullptr)
	    delete reg;
    }
    // bye!
    cout << "\nBye!" << endl;
    return 0;
}
