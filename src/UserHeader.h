#ifndef UserHeader_h
#define UserHeader_h


//#define THE_OMNET_SIMULATION
#define THE_WASPMOTE


#ifdef THE_WASPMOTE
// Avr we think...
// char 1 byte
// int 2 bytes
// short 2 bytes
// long 4 bytes
// long long 8 bytes
// float 4 bytes
// double 4 bytes

// tailored types
#define NEIGHBOUR_ADDR unsigned long
#define TIME_TYPE double

// Special address values
#define SELF_INTERFACE    0x00000001
#define UNKNOWN_INTERFACE 0x00000000

#undef MY_C_PLUSPLUS
#undef GRAD_FILES
#undef ROUTING_DEBUG
#undef DEBUG

// need to undef other debug IFs

#endif



#ifdef THE_OMNET_SIMULATION

// tailored types
#define NEIGHBOUR_ADDR uint64_t
#define TIME_TYPE double

// Special address values
#define SELF_INTERFACE 0x20000000001
#define UNKNOWN_INTERFACE 0x20000000000

#define MY_C_PLUSPLUS 1
#define GRAD_FILES 1

#endif

//do a list of commented out debug type defs

#endif
