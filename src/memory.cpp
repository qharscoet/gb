#include "memory.h"

inline uint32_t kilobytes(uint32_t n) { return n << 10; } // 1024 * n;}

Memory::Memory()
{
	mmap = new uint8_t[MEMSIZE];
	std::memset(mmap, 0, MEMSIZE);

	mbc = nullptr;
}

Memory::~Memory()
{
	delete[] mmap;
	delete mbc;
}

void Memory::set_apu(Sound* apu)
{
	this->apu = apu;
}

void Memory::reset()
{
	std::memset(mmap, 0, MEMSIZE);
	if(mbc != nullptr)
		mbc->reset();
}

void Memory::DMATransfer(uint8_t src)
{
	uint16_t addr  = src << 8;
	memcpy(mmap + 0xFE00, mmap + addr, 160);
	//TODO : check for timings and stuff
}

void Memory::load_content(std::istream &file)
{
	reset();
	file.read((char*)mmap, 0x8000);

	uint32_t romsize = ( kilobytes(32) )<< mmap[0x148];
	uint32_t ramsize = 0;

	switch(mmap[0x149])
	{
		case 0x01: ramsize = 2048; break;
		case 0x02: ramsize = 8192; break;
		case 0x03: ramsize = kilobytes(32); break;
		default:break;
	}

	if(mbc != nullptr)
		delete mbc;

	switch(mmap[0x147]) {
		case 0x00:
			mbc = new MBC(static_cast<MBC::mbc_type>(mmap[0x147]), romsize, ramsize, file);
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			mbc = new MBC1(static_cast<MBC::mbc_type>(mmap[0x147]), romsize, ramsize, file);
			break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			mbc = new MBC5(static_cast<MBC::mbc_type>(mmap[0x147]), romsize, ramsize, file);
			break;
		default:
			break;
	}
}

void Memory::load_content(const uint8_t* data, uint32_t size)
{
	memcpy(mmap, data, size);
}

uint8_t Memory::read_8bits(uint16_t addr) const
{
	uint8_t ret = 0;

	if (addr >= 0x4000 && addr < 0x8000)
	{
		ret = mbc->read_rom(addr - 0x4000);
	}
	else if (addr >= 0xA000 && addr < 0xC000)
	{
		ret = mbc->read_ram(addr - 0xA000);
	}
	else if (addr >= 0xFF10 && addr < 0xFF40)
	{
		ret = apu->read_reg(addr);
	}
	else
	{
		//echo ram
		if(addr >= 0xE000 && addr < 0xFE00)
			addr -= 0x2000;

		ret = mmap[addr];
	}

	return ret;
}

uint16_t Memory::read_16bits(uint16_t addr) const
{
	uint8_t lsb = read_8bits(addr++);
	uint8_t msb = read_8bits(addr);

	return (uint16_t)(msb << 8) | (uint16_t)(lsb);
}

void Memory::write_8bits(uint16_t addr, uint8_t value)
{
	//maybe forward to MBC
	if (addr < 0x8000)
	{
		mbc->write(addr, value);
	}
	else if (mbc->use_ram() && addr >= 0xA000 && addr < 0xC000)
	{
		mbc->write_ram(addr - 0xA000, value);
	}
	else if (addr >= 0xFF10 && addr < 0xFF40)
	{
		apu->write_reg(addr, value);
	} else {

		//echo ram
		if (addr >= 0xE000 && addr < 0xFE00)
			addr -= 0x2000;

		if(addr == 0xFF00)
		{
			uint8_t curr_value = mmap[addr];
			curr_value &= 0xC0;
			curr_value |= (value & 0x30);

			switch(value & 0x30)
			{
				case 0x20:
					curr_value |= ((joypad_keys >> 4) & 0x0F);
					break;
				case 0x10:
					curr_value |= (joypad_keys & 0x0F);
					break;
				case 0x30:
					curr_value |= 0x0F;
					break;
			}

			mmap[addr] = curr_value;
			// mmap[addr] ^= 0xFF; //Flip all bits
		}else
		{
			//if(addr == 0xFF40 && disabling && we are in vblank WE CANT IT MAY DAMAGE THE HARDWARE
			mmap[addr] = value;

			//If we write to FF46 we trigger DMA
			if(addr == 0xFF46)
				DMATransfer(value);
		}
	}
}

void Memory::write_16bits(uint16_t addr, uint16_t value)
{

	uint8_t msb = (value >> 8);
	uint8_t lsb = (value & 0xFF);

	write_8bits(addr, lsb);
	write_8bits(addr + 1, msb);

	// mmap[addr] = lsb;
	// mmap[addr + 1] = msb;
	//*((uint16_t *)(mmap + addr)) = value;
}

void Memory::request_interrupt(interrupt_id id)
{
	const uint16_t IF = 0xFF0F;
	uint8_t bit = static_cast<uint8_t>(id);
	uint8_t if_val = read_8bits(IF);

	if_val |= (1 << bit);

	write_8bits(IF, if_val);
}

void Memory::update_joypad(uint8_t keys)
{
	// In the gameboy, 0 means pressed
	joypad_keys = ~keys;
}

uint8_t *const Memory::get_data(uint16_t addr) const
{
	if (addr >= 0x4000 && addr < 0x8000)
		return mbc->get_rom_data(addr - 0x4000);
	else if (addr >= 0xA000 && addr < 0xC000)
		return mbc->get_ram_data(addr - 0xA000);
	// else if (addr >= 0xFF10 && addr < 0xFF40)
	// 	return apu->get_data(addr - 0xFF10);
	else
		return &mmap[addr];
}

void Memory::dump_ram(std::ostream &file) const
{
	mbc->dump_ram(file);
}

void Memory::load_ram(std::istream &file)
{
	if (mbc->use_ram())
		mbc->load_ram(file);
}

bool Memory::use_external_ram() const
{
	return use_mbc() && mbc->use_ram();
}

bool Memory::use_mbc() const
{
	return mmap[0x0147] != 0;
}