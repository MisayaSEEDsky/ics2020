#include "common.h"
#include "fs.h"
//#include "mm.c"
#define DEFAULT_ENTRY ((void *)0x8048000)

void* new_page(void);
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
  	int fd = fs_open(filename, 0 ,0);
	size_t lens = fs_filesz(fd);
	//fs_read(fd,DEFAULT_ENTRY,lens);
	void *va = DEFAULT_ENTRY;
	void *pa;
	int page_count = lens  /  PGSIZE + 1;

	for(int i = 0; i < page_count; i++)
	{
		pa = new_page();
		Log("Map va to pa: 0x%08x to 0x%08x",va,pa);
		_map(as,va,pa);
		fs_read(fd,pa,PGSIZE);
		va += PGSIZE;
	}
	
	
	fs_close(fd);

	return (uintptr_t)DEFAULT_ENTRY;
}
