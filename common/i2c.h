// Includes (if needed)
#include <string.h>
#include <msp430fr2355.h>

// Macros (if needed)

// Type definitions (if needed)
/*typedef struct {
    int field1;
    float field2;
} CommonStruct;*/

// Function declarations
void setupMasterI2C(void);
void Tx(int slave_addr, char message[], char tx_buff[]);
void setupSlaveI2C(int slave_addr, int bytes);