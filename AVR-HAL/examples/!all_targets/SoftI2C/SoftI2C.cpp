#include <util/delay.h>
#include "SoftI2C.h"

int main() {
    using SoftI2C = hal::SoftI2C<8, 9>;
    SoftI2C::init();
    constexpr hal::I2C_Device<SoftI2C> dev{0x1E};

    while (true) {
        dev.write(0x12);
        _delay_ms(10);
    }
}