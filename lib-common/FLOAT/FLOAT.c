#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	return (FLOAT)(((long long)a * b) >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */
	int q, r;
	asm volatile ("idivl %4" : "=a"(q), "=d"(r)
	              : "a"(a << 16), "d"(a >> 16), "r"(b) : "cc");
	return q;
}

FLOAT f2F(float a) {
	/* You should figure out how to convert `a' into FLOAT without
	 * introducing x87 floating point instructions. Else you can
	 * not run this code in NEMU before implementing x87 floating
	 * point instructions, which is contrary to our expectation.
	 *
	 * Hint: The bit representation of `a' is already on the
	 * stack. How do you retrieve it to another variable without
	 * performing arithmetic operations on it directly?
	 */
	int f;
	memcpy(&f, &a, sizeof(float));
	if ((f & 0x7fffffff) == 0) return 0;
	int f_s = f >> 31;
	int f_exp = (f & 0x7fffffff) >> 23;
	int f_frac = f & 0x007fffff;
	if (f_exp == 0xff) { nemu_assert(0); return 0; }
	if (f_exp < 110 || f_exp > 141) { nemu_assert(0); return 0; }
	f_frac |= 0x00800000;
	int mag = (f_exp > 134) ? (f_frac << (f_exp - 134)) : (f_frac >> (134 - f_exp));
	return f_s ? -mag : mag;
	
}

FLOAT Fabs(FLOAT a) {
	return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

