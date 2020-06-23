#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

/*paddr_t page_translate(vaddr_t vaddr, bool writing)
{
	if(cpu.cr0.paging == 0 )	return vaddr;

        PDE pde;
        PTE pte;

        //Log("vaddr:%#x",vaddr);
       // Log("CR3:%#x",cpu.cr3.page_directory_base);

        uint32_t DIR  = (vaddr >> 22);
        //Log("DIR:%#x",DIR);
        uint32_t PDE_addr = (cpu.cr3.page_directory_base << 12) + (DIR << 2);
        pde.val = paddr_read(PDE_addr, 4);
      //  Log("PDE_addr:%#x",PDE_addr);
       // Log("PDE_val:%#x",pde.val);
        assert(pde.present);

        uint32_t PAGE = ((vaddr >> 12) & 0x3ff);
        uint32_t PTE_addr = (pde.val & 0xfffff000) + (PAGE << 2);
        pte.val = paddr_read(PTE_addr, 4);
       // Log("PTE_addr:%#x",PTE_addr);
       // Log("PTE_val:%#x",pte.val);
        assert(pte.present);

        uint32_t physical_addr = (pte.val & 0xfffff000) + (vaddr & 0xfff);
        //Log("Physical_addr:%#x",physical_addr);

        pde.accessed = 1;
        paddr_write(PDE_addr,4,pde.val);

        if(pte.accessed == 0 || (pte.dirty == 0  && writing))
        {
                pte.accessed = 1;
                pte.dirty = 1;
        }

        paddr_write(PTE_addr,4,pte.val);
        return physical_addr;
}
*/



paddr_t page_translate(vaddr_t addr, bool is_write) {

 if (!cpu.cr0.paging) return addr;
 // Log("page_translate: addr: 0x%x\n", addr);
  paddr_t pde_index  = (addr >> 22) & 0x3ff;
  paddr_t pte_index = (addr >> 12) & 0x3ff;
  paddr_t offset = addr & 0xfff;
  
  paddr_t PDT_base = cpu.cr3.page_directory_base;
  paddr_t pde = (PDT_base << 12) + (pde_index << 2 );
  //Log("page_translate: dir: 0x%x page: 0x%x offset: 0x%x PDT_base: 0x%x\n", dir, page, offset, PDT_base);
  
  PDE pde_obj;
  pde_obj.val = paddr_read(pde, 4);
  //if (!pde.present) {
    //Log("page_translate: addr: 0x%x\n", addr);
    //Log("page_translate: dir: 0x%x page: 0x%x offset: 0x%x PDT_base: 0x%x\n", dir, page, offset, PDT_base);
  //Log("cr3: %x:",cpu.cr3.page_directory_base); 
  assert(pde_obj.present);
  //}

  paddr_t pte = (pde_obj.val << 12) + (pte_index << 2);
  PTE pte_obj;
  // Log("page_translate: page_frame: 0x%x\n", pde.page_frame);
  pte_obj.val = paddr_read(pte, 4);
  //if (!pte.present) {
    //Log("page_translate: addr: 0x%x\n", addr);
    assert(pte_obj.present);
  //}
  
    if(!pde_obj.accessed)
    {
	    pde_obj.accessed = 1;
	    paddr_write(pde, 4, pde_obj.val);
    }

    if(!pte_obj.accessed || (!pte_obj.dirty == 0 && is_write))
    {
	    pte_obj.accessed = 1;
	    pte_obj.dirty = 1;
	    paddr_write(pte, 4, pte_obj.val);
    }
    
    paddr_t paddr = (pte_obj.page_frame << 12) | offset;
  //Log("page_translate: paddr: 0x%x\n", paddr);
  

  return paddr;
}

uint32_t paddr_read(paddr_t addr, int len) {
  	int mmio_id;
	mmio_id = is_mmio(addr);
	if(mmio_id != -1)	
		return mmio_read(addr,len,mmio_id);
	else
		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  	int mmio_id;
	mmio_id = is_mmio(addr);
	if(mmio_id != -1)	
		mmio_write(addr,len,data,mmio_id);
	else	
		memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging)
  {
	  if((((addr << 20) >> 20) + len) > 0x1000) //data cross the page boundary
	  {
		  //assert(0);
		  int fir_len , sec_len;
		  fir_len = 0x1000 - (addr & 0xfff);
		  sec_len = len - fir_len;

		  uint32_t fir_addr = page_translate(addr, false);
		  uint32_t fir_mem = paddr_read(fir_addr,fir_len);

		  uint32_t sec_addr = page_translate(addr + fir_len,false);
		  uint32_t sec_mem = paddr_read(sec_addr,sec_len);

		  return fir_mem + (sec_mem << (fir_len << 3));
	  }
	  else
	  {
		paddr_t paddr = page_translate(addr,false);
		return paddr_read(paddr, len);
	  }
  }
  else	return  paddr_read(addr,len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging)
  {
	  if((((addr << 20) >> 20) + len) > 0x1000)
	  {
		  //assert(0);
	  	int fir_len,sec_len;
		fir_len  = 0x1000 - (addr & 0xfff);
                sec_len = len - fir_len;

		uint32_t fir_addr = page_translate(addr,true);
		paddr_write(fir_addr,fir_len,data);

		uint32_t high_data = data >> (fir_len << 3);
		uint32_t sec_addr = page_translate(addr + fir_len, true);

		paddr_write(sec_addr,sec_len,high_data);
	  }
	  else
	  {
		  paddr_t paddr = page_translate(addr,true);
		  paddr_write(paddr,len,data);
	  }
  }
  else	paddr_write(addr, len, data);
}



