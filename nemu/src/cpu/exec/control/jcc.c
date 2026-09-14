#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "jcc-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "jcc-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "jcc-template.h"
#undef DATA_BYTE

 make_helper_v(jo_si);
make_helper_v(jno_si);
 make_helper_v(jb_si);
make_helper_v(jnb_si);
 make_helper_v(je_si);
make_helper_v(jne_si);
 make_helper_v(ja_si);
make_helper_v(jna_si);
 make_helper_v(js_si);
make_helper_v(jns_si);
 make_helper_v(jp_si);
make_helper_v(jnp_si);
 make_helper_v(jl_si);
make_helper_v(jnl_si);
 make_helper_v(jg_si);
make_helper_v(jng_si);
