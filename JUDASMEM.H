/*
 * Internal header file: memory locking
 */
int judas_memlock(void *start, unsigned size);
int judas_memunlock(void *start, unsigned size);
void *locked_malloc(int size);
void locked_free(void *address);
void *dos_malloc(int size);
void dos_free(void *address);
int DPMI_MapMemory (unsigned long *physaddress, unsigned long *linaddress, unsigned long size);
int DPMI_UnmapMemory (unsigned long *linaddress);
