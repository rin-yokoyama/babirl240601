vme_dma_handle_t *dma_p;
char *dmaptr, *dmamapptr;
size_t ps = PAGE_SIZE;
unsigned long dmapaddr, doff, dmapsize;

void *mmp(unsigned long paddr, size_t size){
  doff = paddr % ps;
  dmapaddr = paddr - doff;
  dmapsize = size + doff;
  dmapsize += (dmapsize % ps) ? ps - (dmapsize % ps) : 0;
  
  dmamapptr = ioremap(dmapaddr, dmapsize);
  
  return dmamapptr + doff;
}



int dma_window(unsigned long size){
  int status;
  dma.size = size;
  dma.flags = 0;
  dma.paddr = NULL;
  
  dma_p = (vme_dma_handle_t *) & dma.id;
  status = vme_dma_buffer_create(bus, dma_p, dma.size, dma.flags, dma.paddr);
  dma.paddr = vme_dma_buffer_phys_addr(bus, dma.id);
  dmaptr = mmp((unsigned long)dma.paddr, dma.size);
  
  return 0;
}


void univ_init_dma(unsigned long vaddr, unsigned long size){
  dma_window(size);
  
  dma.vaddr = vaddr;
  dma.offset = 0;
  dma.am = 0x0b;
  
}

void univ_end_dma(void){
  vfree(dmamapptr);
  vme_dma_buffer_release(bus, *dma_p);
}


int univ_dma_segdata(int size){
  vme_dma_read2(bus, *dma_p, dma.offset, dma.vaddr, dma.am, size, dma.flags);
  memcpy((char *)(data+mp), dmaptr, size);
  segmentsize += size/2;
  mp += size/2;
  
  return segmentsize;
}

