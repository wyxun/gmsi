#include "main.h"
#include "gmsi.h"
//#include "at32f413_board.h"

gmsi_t tGmsi = {NULL};

void main(void)
{
    //at32_board_init();
    gmsi_Init(&tGmsi);

    for(;;)
    {
        
    }
}