// /17.0/nios2eds/bin/gnu/share/doc/gcc/Altera-Nios-II-Built_002din-Functions.html
// 17.0\nios2eds\bin\gnu\share\info\gcc.info

// 6.58.3 Altera Nios II Built-in Functions
// ----------------------------------------

// These built-in functions are available for the Altera Nios II family of
// processors.

// The following built-in functions are always available.  They all
// generate the machine instruction that is part of the name.

     int __builtin_ldbio (volatile const void *);
     int __builtin_ldbuio (volatile const void *);
     int __builtin_ldhio (volatile const void *);
     int __builtin_ldhuio (volatile const void *);
     int __builtin_ldwio (volatile const void *);
     void __builtin_stbio (volatile void *, int);
     void __builtin_sthio (volatile void *, int);
     void __builtin_stwio (volatile void *, int);
     void __builtin_sync (void);
     int __builtin_rdctl (int);
     int __builtin_rdprs (int, int);
     void __builtin_wrctl (int, int);
     void __builtin_flushd (volatile void *);
     void __builtin_flushda (volatile void *);
     int __builtin_wrpie (int);
     void __builtin_eni (int);
     int __builtin_ldex (volatile const void *);
     int __builtin_stex (volatile void *, int);
     int __builtin_ldsex (volatile const void *);
     int __builtin_stsex (volatile void *, int);
