//PLL #defines


//PLLCFG
#define MSEL 0
#define NSEL 16


//PLLCON (These define PLL mode (off/active-no connect/active-connected)
#define PLLE 0	
#define PLLC 1

//PLLSTAT
#define PLLSTAT_PLLE 	24
#define PLLSTAT_PLLC 	25
#define PLLSTAT_PLOCK 26

//PLL mult = 12 = 0x0C.  Reg holds M-1 = 0x0B
//#define PLL_MULT 0x23
// PLL mult of 35 gives fcco=(2 x (M+1) x Fin)/N = 288000000 = (2*46*4000000)/1
//#define PLL_MULT 0x23
// CPU div = 4, reg holds N-1 = 3
// 288000000/3 = 96000000
// 288000000/4 = 72000000
//#define CPU_DIV  0x02			// for 96000000
//#define CPU_DIV  0x04			// for 57600000
//#define CPU_DIV  0x03				// for 72000000

// (PLL_MULT (M) = 35 = 0x23) (PLL_DIV (N) = 0x03)  gives fcco=(2 x (M+1) x Fin)/N = (2*36*4000000)/3 = 96000000 
#define PLL_MULT 0x23
// PLL div N = 4, reg holds N-1 = 3
// 288000000/3 = 96000000
// 288000000/4 = 72000000
#define PLL_DIV_72MHZ  0x03	 // for 72000000
#define CCLKSEL_36MHZ  0x01  // This divides the PLL Output Signal in Half
                             // In this case 72000000/(CCLKSEL + 1) = 36000000

void PLL_CONFIG(void);
