#ifndef HAL_DEVICES_LM60_H_
#define HAL_DEVICES_LM60_H_

namespace hal {

namespace LM60 {
    static float temperature(float mV) {
        return (mV - 434)/6.25;
    }
}  // namespace LM60
}  // namespace hal

#endif  // HAL_DEVICES_LM60_H_