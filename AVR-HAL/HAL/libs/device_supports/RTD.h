#ifndef HAL_DEVICES_RTD_H_
#define HAL_DEVICES_RTD_H_

namespace hal {

class RTD {
 public:
    constexpr RTD(const float R0, const float a = 0.00390802, const float b = -0.000000580195) : R0(R0), a(a), b(b) {
    }

    float temperature(float resistance) {
        return (-R0 * a + sqrt(R0*R0*a*a - 4 * R0 * b * (R0-resistance)))/(2*R0*b);
    }

 private:
    const float R0, a, b;
};

}  // namespace hal

#endif  // HAL_DEVICES_RTD_H_