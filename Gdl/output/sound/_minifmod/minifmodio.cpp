
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "minifmod.h"

uintptr_t memopen(char *name) { return((uintptr_t)name); }

void memclose(uintptr_t handle){}

int memread(void *buffer, int size, uintptr_t handle)
{ MEMFILE *memfile = (MEMFILE *)handle;

  if (memfile->pos + size >= memfile->length)
    size = memfile->length - memfile->pos;

  memcpy(buffer, (char *)memfile->data+memfile->pos, size);
  memfile->pos += size;

  return(size);
}

void memseek(uintptr_t handle, int pos, signed char mode)
{ MEMFILE *memfile = (MEMFILE *)handle;

  if (mode == SEEK_SET)
    memfile->pos = pos;
  else if (mode == SEEK_CUR)
    memfile->pos += pos;
  else if (mode == SEEK_END)
    memfile->pos = memfile->length + pos;

  if (memfile->pos > memfile->length)
    memfile->pos = memfile->length;
}

int memtell(uintptr_t handle)
{ MEMFILE *memfile = (MEMFILE *)handle;
  return(memfile->pos);
}

void FMUSIC_MemInit(void)
{  FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell);
}
