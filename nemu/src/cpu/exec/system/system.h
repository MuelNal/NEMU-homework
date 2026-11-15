#ifndef __SYSTEM_H__
#define __SYSTEM_H__

make_helper(lgdt);
make_helper(mov_cr2r);
make_helper(mov_r2cr);
make_helper(mov_rm2sreg);
make_helper(ljmp);

void load_sreg(uint8_t sreg, uint16_t selector);

#endif
