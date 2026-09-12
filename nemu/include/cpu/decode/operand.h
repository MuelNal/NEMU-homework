#ifndef __OPERAND_H__
#define __OPERAND_H__

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

#define OP_STR_SIZE 40

typedef struct {
	uint32_t type;				//操作码类型
	size_t size;				//操作码大小
	union {						//对应操作码类型的标识符
		uint32_t reg;			//寄存器编号
		swaddr_t addr;			//地址
		uint32_t imm;			//无符号立即数
		int32_t simm;			//有符号立即数
	};
	uint32_t val;				//所填的数据
	char str[OP_STR_SIZE];		//操作码转化为字符串
} Operand;

typedef struct {
	uint32_t opcode;			//操作码的编码
	bool is_operand_size_16;	//判断操作是否是16位的
	Operand src, dest, src2;	//源操作码，目的操作码
} Operands;

#endif
