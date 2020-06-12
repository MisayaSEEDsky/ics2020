#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
 // t0 = id_dest->val;
  rtl_push(&id_dest->val);
 /* if (id_dest->type == OP_TYPE_REG)
  {
	  rtl_sr(id_dest->reg,id_dest->width,&t0);
  }
  else if(id_dest->type == OP_TYPE_MEM)
  {
	  rtl_sm(&id_dest->addr,id_dest->width,&t0);
  }
  else
  {
	  assert(0);
  }
*/
  print_asm_template1(push);
}

make_EHelper(pop) {
 /* rtl_pop(&t0);
  if(id_dest->type == OP_TYPE_REG)
	  rtl_sr(id_dest->reg, id_dest->width, &t0);
  else if (id_dest->type == OP_TYPE_MEM)
	  rtl_sm(&id_dest->addr, id_dest->width, &t0);
  else 
	  assert(0);
*/
  rtl_pop(&id_src->val);
  operand_write(id_dest,&id_src->val);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  t2 = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t2);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t2);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {	//AX
    //TODO();
	rtl_lr_w(&t0,R_AX);
	if((int32_t)(int16_t)(uint16_t)t0 < 0)
	{
		reg_w(R_DX) = 0xffff;
	}
	else
	{
		reg_w(R_DX) = 0;
	}
  }
  else {	//EAX
	rtl_lr_l(&t0,R_EAX);
	if((int32_t)t0 < 0)
	{
		reg_l(R_EDX) = 0xffffffff;
	}
	else
	{
		reg_l(R_EDX) = 0;
	}
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {	//al
    rtl_lr_b(&t0,R_AL);
    rtl_sext(&t0,&t0,2);
    reg_w(R_AX) = t0;

  }
  else {	//ax
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);
    reg_l(R_EAX) = t0;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
