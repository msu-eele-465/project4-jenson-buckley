// Includes (if needed)
#include <msp430fr2355.h>

// Macros (if needed)

// Type definitions (if needed)
/*typedef struct {
    int field1;
    float field2;
} CommonStruct;*/

// Function declarations
void setupKeypad(char);
char readKeypad(void);
int checkCols(void);