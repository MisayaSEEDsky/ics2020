#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

paddr_t page_translate(vaddr_t vaddr, bool writing)
{
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



