#include "cpu/exec.h"

make_EHelper(test) {
  //TODO();
	rtl_and(&t0,&id_dest->val,&id_src->val);
	t1 = 0;
	rtl_set_OF(&t1);
	rtl_set_CF(&t1);

	rtl_update_ZFSF(&t0,id_dest->width);	
  print_asm_template2(test);
}

make_EHelper(and) {
  rtlreg_t a1,a0;
  rtl_and(&a1, &id_dest->val, &id_src->val);	//and
  operand_write(id_dest, &a1);	//

  rtl_update_ZFSF(&a1, id_dest->width);	//update ZF SF
  a0 = 0;
  rtl_set_OF(&a0);	//OF = 0
  rtl_set_CF(&a0);	//CF = 0;
  
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  if(id_dest->type == OP_TYPE_REG)
  {
	  rtl_sr(id_dest->reg, id_dest->width, &t0);
  }
  else if(id_dest->type == OP_TYPE_MEM)
  {
	  rtl_sm(&id_dest->addr, id_dest->width, &t0);
  }
  else assert(0);
  t1 = 0;
  rtl_set_OF(&t2);
  rtl_set_CF(&t2);
  rtl_update_ZFSF(&id_dest->val,id_dest->width);


  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
	rtl_or(&t0,&id_dest->val,&id_src->val);
	operand_write(id_dest, &t0);

	t1 = 0;
	rtl_set_OF(&t1);
	rtl_set_CF(&t1);
	rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template2(or);
}

make_EHelper(sar) {
  
  rtl_sar(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);

  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);

  rtl_update_ZFSF(&t0,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);

  rtl_update_ZFSF(&t0,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_li(&t0,id_dest->val);
  rtl_not(&t0);
  operand_write(id_dest,&t0);

  print_asm_template1(not);
}

make_EHelper(rol) {
  //TODO();
  for(t0 = 0;t0 < id_src->val ; t0++)
  {
	  rtl_shri(&t1, &id_dest->val, id_dest->width * 8 - 1);
	  rtl_shli(&t1, &id_dest->val, 1);
	  rtl_xori(&t2,&id_dest->val,t1);
  }
  rtl_set_CF(&t1);
  operand_write(id_dest,&t2);
  
  print_asm_template2(rol);
}
