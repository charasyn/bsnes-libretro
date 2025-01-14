//immediate, 2-cycle opcodes with idle cycle will become bus read
//when an IRQ is to be triggered immediately after opcode completion.
//this affects the following opcodes:
//  clc, cld, cli, clv, sec, sed, sei,
//  tax, tay, txa, txy, tya, tyx,
//  tcd, tcs, tdc, tsc, tsx, txs,
//  inc, inx, iny, dec, dex, dey,
//  asl, lsr, rol, ror, nop, xce.
auto WDC65816::idleIRQ() -> void {
  if(interruptPending()) {
    //modify I/O cycle to bus read cycle, do not increment PC
    read(PC.d);
  } else {
    idle();
  }
}

auto WDC65816::idle2() -> void {
  if(D.l) idle();
}

auto WDC65816::idle4(uint16 x, uint16 y) -> void {
  if(!XF || x >> 8 != y >> 8) idle();
}

auto WDC65816::idle6(uint16 address) -> void {
  if(EF && PC.h != address >> 8) idle();
}

auto WDC65816::fetch() -> uint8 {
  return read(PC.b << 16 | PC.w++);
}
auto WDC65816::fetch_inst() -> uint8 {
  if (PC.b == 0x09 && PC.w == 0xa190) {
    brk_hit = true;
  }
  if (!hitIllegal) {
    if ((PC.b&0x80) >= 0x70 && (PC.b&0x80) <= 0x7d) {
      // executing from SRAM
      std::cout << "Executing from SRAM: " << std::hex << (int)PC.b << ":" << PC.w << std::endl;
      hitIllegal = true;
    } else if (((PC.b&0x80) >= 0x40 && (PC.b&0x80) <= 0x6f) || (
              ((PC.b&0x80) >= 0x00 && (PC.b&0x80) <= 0x3f) && 
              ((PC.w) >= 0 && (PC.w) <= 0x7fff))) {
      // executing from non-ROM and non-main-RAM
      std::cout << "Executing from unexpected area: " << std::hex << (int)PC.b << ":" << PC.w << std::endl;
      hitIllegal = true;
    }
  }
  return read(PC.b << 16 | PC.w++);
}

auto WDC65816::pull() -> uint8 {
  EF ? (void)S.l++ : (void)S.w++;
  return read(S.w);
}

auto WDC65816::push(uint8 data) -> void {
  write(S.w, data);
  EF ? (void)S.l-- : (void)S.w--;
}

auto WDC65816::pullN() -> uint8 {
  return read(++S.w);
}

auto WDC65816::pushN(uint8 data) -> void {
  write(S.w--, data);
}

auto WDC65816::readDirect(uint address) -> uint8 {
  if(EF && !D.l) return read(D.w | address & 0xff);
  return read(D.w + address & 0xffff);
}

auto WDC65816::writeDirect(uint address, uint8 data) -> void {
  if(EF && !D.l) return write(D.w | address & 0xff, data);
  write(D.w + address & 0xffff, data);
}

auto WDC65816::readDirectN(uint address) -> uint8 {
  return read(D.w + address & 0xffff);
}

auto WDC65816::readBank(uint address) -> uint8 {
  return read((B << 16) + address & 0xffffff);
}

auto WDC65816::writeBank(uint address, uint8 data) -> void {
  write((B << 16) + address & 0xffffff, data);
}

auto WDC65816::readLong(uint address) -> uint8 {
  return read(address & 0xffffff);
}

auto WDC65816::writeLong(uint address, uint8 data) -> void {
  write(address & 0xffffff, data);
}

auto WDC65816::readStack(uint address) -> uint8 {
  return read(S.w + address & 0xffff);
}

auto WDC65816::writeStack(uint address, uint8 data) -> void {
  write(S.w + address & 0xffff, data);
}
