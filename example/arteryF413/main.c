#include "main.h"
#include "gmsi.h"
//#include "at32f413_board.h"

gmsi_t tGmsi = {NULL};

int main(void)
{
    //at32_board_init();
    gmsi_Init(&tGmsi);

    for(;;)
    {
        
    }
    return 0;
}