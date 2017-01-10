#ifndef HAL_DEVICES_SUNS_BH1730FVC_H_
#define HAL_DEVICES_SUNS_BH1730FVC_H_

#include "array.h"
#include "DigitalIO.h"
#include <util/delay.h>


namespace hal {

template<DigitalIO::Pin scl, DigitalIO::Pin SDA_A, DigitalIO::Pin SDA_B, DigitalIO::Pin SDA_C, DigitalIO::Pin SDA_D>
class softI2Cmulti {
 public:
    enum Action {
        START_WRITE = 0,
        START_READ = 1
    };

    enum Status {
        ACK_ERROR = 0,
        OK = 1,
        SCL_ERROR = 2
    };

    softI2Cmulti() {
    }

    void init() const {
        this->SCL.pinmode(DigitalIO::INPUT);
        this->SCL.reset();

        for (uint8_t i = 0; i < this->len; i++) {
            SDAs[i].pinmode(DigitalIO::INPUT);
            SDAs[i].read();
        }
    }


    libs::array<Status, 5> start(uint8_t address, const Action start_action) {
        this->SCL.pinmode(DigitalIO::INPUT);
        _delay_loop_1(this->hDelay);
        this->SDAs_LOW();
        _delay_loop_1(this->hDelay);

        return this->write((address << 1) | start_action);
    }

    void stop() const {
        this->SDAs_LOW();
        _delay_loop_1(this->hDelay);
        this->SCL.pinmode(DigitalIO::INPUT);
        _delay_loop_1(this->qDelay);
        this->SDAs_HIGH();
        _delay_loop_1(this->hDelay);
    }

    libs::array<Status, 5> write(uint8_t data) {
        libs::array<Status, 5> return_status = {OK, OK, OK, OK, OK};

        for (uint8_t i = 0; i < 8; i++) {
            this->SCL.pinmode(DigitalIO::OUTPUT);
            _delay_loop_1(this->qDelay);

            if (data & 0x80) {
                this->SDAs_HIGH();
            } else {
                this->SDAs_LOW();
            }

            _delay_loop_1(this->hDelay);
            this->SCL.pinmode(DigitalIO::INPUT);
            _delay_loop_1(this->hDelay);

            volatile uint16_t timeout = 0;
            while (0 == this->SCL.read()) {
                timeout++;
                if (timeout > 5000) {
                    return_status[4] = SCL_ERROR;
                    break;
                }
            }

            data = data << 1;
        }

        this->SCL.pinmode(DigitalIO::OUTPUT);
        _delay_loop_1(this->qDelay);

        this->SDAs_HIGH();
        _delay_loop_1(this->hDelay);

        this->SCL.pinmode(DigitalIO::INPUT);
        _delay_loop_1(this->hDelay);


        for (uint8_t i = 0; i < 4; i++) {
            if (0 != SDAs[i].read()) {
                return_status[i] = ACK_ERROR;
            }
        }

        this->SCL.pinmode(DigitalIO::OUTPUT);
        _delay_loop_1(this->hDelay);

        return return_status;
    }

    libs::array<uint8_t, 4> read(bool ACK) {
        libs::array<uint8_t, 4> SDA_read_data;

        for (uint8_t i = 0; i < 8; i++) {
            this->SCL.pinmode(DigitalIO::OUTPUT);
            _delay_loop_1(this->hDelay);
            this->SCL.pinmode(DigitalIO::INPUT);
            _delay_loop_1(this->hDelay);

            volatile uint16_t timeout = 0;
            while (0 == this->SCL.read()) {
                timeout++;
                if (timeout > 5000) {
                    break;
                }
            }


            for (uint8_t j = 0; j < 4; j++) {
                if (SDAs[j].read()) {
                    SDA_read_data[j] |=(0x80 >> i);
                } else {
                    SDA_read_data[j] &=~(0x80 >> i);
                }
            }
        }

        this->SCL.pinmode(DigitalIO::OUTPUT);
        _delay_loop_1(this->qDelay);

        if (ACK) {
            this->SDAs_LOW();
        } else {
            this->SDAs_HIGH();
        }

        _delay_loop_1(this->hDelay);

        this->SCL.pinmode(DigitalIO::INPUT);
        _delay_loop_1(this->hDelay);

        this->SCL.pinmode(DigitalIO::OUTPUT);

        this->SDAs_HIGH();
        _delay_loop_1(this->hDelay);

        return SDA_read_data;
    }

 private:
    const DigitalIO SCL{scl};
    libs::array<const DigitalIO, 4> SDAs = {{DigitalIO(SDA_A), DigitalIO(SDA_B), DigitalIO(SDA_C), DigitalIO(SDA_D)}};

    uint8_t qDelay{3};
    uint8_t hDelay{5};
    uint8_t len{4};

    void SDAs_LOW() const {
        for (uint8_t i = 0; i < this->len; i++) {
            SDAs[i].pinmode(DigitalIO::OUTPUT);
        }
    }
    void SDAs_HIGH() const {
        for (uint8_t i = 0; i < this->len; i++) {
            SDAs[i].pinmode(DigitalIO::INPUT);
        }
    }
};



template<DigitalIO::Pin scl, DigitalIO::Pin SDA_A, DigitalIO::Pin SDA_B, DigitalIO::Pin SDA_C, DigitalIO::Pin SDA_D>
class SunS_BH1730FVC {
 public:
    enum Gain {
        GAIN_1 = 0b00,
        GAIN_2 = 0b01,
        GAIN_64 = 0b10,
        GAIN_128 = 0b11,
    };

    enum RegisterAddr {
        CONTROL = 0x00,
        TIMING = 0x01,
        INTERRUPT = 0x02,
        THLLOW = 0x03,
        THLHIGH = 0x04,
        THHLOW = 0x05,
        THHHIGH = 0x06,
        GAIN = 0x07,
        ID = 0x12,
        DATA0LOW = 0x14,
        DATA0HIGH = 0x15,
        DATA1LOW = 0x16,
        DATA1HIGH = 0x17,
    };

    enum MeasurementType {
        CONTINUOUS = 0b00000011,
        ONE_SHOT = 0b00001011,
    };

    enum DataSel {
        VL_IR = 0b00000000,
        VL_ONLY = 0b00000100,
    };

     SunS_BH1730FVC() {}

     void init() const {
         this->i2c.init();
     }

    libs::array<typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Status, 5> setIntegrationTime(uint8_t cycle) {
        libs::array<Status, 5> comm_status = {Status::OK, Status::OK, Status::OK, Status::OK, Status::OK};

        libs::array<Status, 5> return_status = this->i2c.start(this->address, I2C_Action::START_WRITE);

        this->processStatus(comm_status, return_status);

        return_status = this->i2c.write(0b10000000 | TIMING);

        this->processStatus(comm_status, return_status);

        // ITIME
        return_status = this->i2c.write(256-cycle);

        this->processStatus(comm_status, return_status);

        this->i2c.stop();

        this->current_time = (256-cycle);

        return comm_status;
    }


    libs::array<typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Status, 5> setGain(Gain gain) {
        libs::array<Status, 5> comm_status = {Status::OK, Status::OK, Status::OK, Status::OK, Status::OK};

        libs::array<Status, 5> return_status = this->i2c.start(this->address, I2C_Action::START_WRITE);

        this->processStatus(comm_status, return_status);

        return_status = this->i2c.write(0b10000000 | GAIN);

        this->processStatus(comm_status, return_status);

        // GAIN
        return_status = this->i2c.write(gain);

        this->processStatus(comm_status, return_status);

        this->i2c.stop();

        switch (gain) {
            case GAIN_1:
                this->current_gain = 1;
            break;
            case GAIN_2:
                this->current_gain = 2;
            break;
            case GAIN_64:
                this->current_gain = 64;
            break;
            case GAIN_128:
                this->current_gain = 128;
            break;
        }

        return comm_status;
    }

    libs::array<uint8_t, 4> readPartID() {
        this->i2c.start(this->address, I2C_Action::START_WRITE);


        this->i2c.write(0b10000000 | ID);


        this->i2c.stop();

        this->i2c.start(this->address, I2C_Action::START_READ);

        libs::array<uint8_t, 4> ids =  this->i2c.read(false);

        this->i2c.stop();

        return ids;
    }

    libs::array<typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Status, 5> ambientLightRAW(libs::array<uint16_t, 4>& VL, libs::array<uint16_t, 4>& IR) {
        libs::array<uint8_t, 4> DATA0MSB;
        libs::array<uint8_t, 4> DATA0LSB;
        libs::array<uint8_t, 4> DATA1MSB;
        libs::array<uint8_t, 4> DATA1LSB;

        libs::array<Status, 5> comm_status = {Status::OK, Status::OK, Status::OK, Status::OK, Status::OK};

        libs::array<Status, 5> return_status = this->i2c.start(this->address, I2C_Action::START_WRITE);

        this->processStatus(comm_status, return_status);

        return_status = this->i2c.write(0b10010100);

        this->processStatus(comm_status, return_status);

        this->i2c.stop();

        return_status = this->i2c.start(this->address, I2C_Action::START_READ);

        this->processStatus(comm_status, return_status);

        // LSB first, then MSB!
        DATA0LSB = this->i2c.read(true);
        DATA0MSB = this->i2c.read(true);
        DATA1LSB = this->i2c.read(true);
        DATA1MSB = this->i2c.read(false);

        this->i2c.stop();

        for (uint8_t i = 0; i < 4; i++) {
            VL[i] = (static_cast<uint16_t>(DATA0MSB[i]) << 8) | DATA0LSB[i];
            IR[i] = (static_cast<uint16_t>(DATA1MSB[i]) << 8) | DATA1LSB[i];
        }

        return comm_status;
    }


    libs::array<typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Status, 5> ambientLightRAW(libs::array<uint16_t, 4>& VL) {
        libs::array<uint8_t, 4> DATA0MSB;
        libs::array<uint8_t, 4> DATA0LSB;

        libs::array<Status, 5> comm_status = {Status::OK, Status::OK, Status::OK, Status::OK, Status::OK};

        libs::array<Status, 5> return_status = this->i2c.start(this->address, I2C_Action::START_WRITE);
        this->processStatus(comm_status, return_status);

        return_status = this->i2c.write(0b10010100);
        this->processStatus(comm_status, return_status);

        this->i2c.stop();

        return_status = this->i2c.start(this->address, I2C_Action::START_READ);
        this->processStatus(comm_status, return_status);

        // LSB first, then MSB!
        DATA0LSB = this->i2c.read(true);
        DATA0MSB = this->i2c.read(false);

        this->i2c.stop();

        for (uint8_t i = 0; i < 4; i++) {
            VL[i] = (static_cast<uint16_t>(DATA0MSB[i]) << 8) | DATA0LSB[i];
        }

        return comm_status;
    }

    libs::array<typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Status, 5> setMeasurement(MeasurementType type, DataSel data_sel) {
        libs::array<Status, 5> comm_status = {Status::OK, Status::OK, Status::OK, Status::OK, Status::OK};

        libs::array<Status, 5> return_status = this->i2c.start(this->address, I2C_Action::START_WRITE);
        this->processStatus(comm_status, return_status);

        return_status = this->i2c.write(0b10000000 | CONTROL);
        this->processStatus(comm_status, return_status);

        // CONTROL
        return_status = this->i2c.write(type | data_sel);
        this->processStatus(comm_status, return_status);

        this->i2c.stop();
        return comm_status;
    }

 private:
    softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D> i2c;
    const uint8_t address = 0b0101001;

    uint8_t current_gain = 1;
    uint8_t current_time = 218;

    using Status = typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Status;
    using I2C_Action = typename softI2Cmulti<scl, SDA_A, SDA_B, SDA_C, SDA_D>::Action;

    void processStatus(libs::array<Status, 5>& status_return,  libs::array<Status, 5>& current_status) {
        for (uint8_t i = 0; i < 4; i++) {
            if (Status::ACK_ERROR == current_status[i]) {
                status_return[i] = Status::ACK_ERROR;
            }
        }

        if (Status::SCL_ERROR == current_status[4]) {
            status_return[4] = Status::SCL_ERROR;
        }
    }
};

}  // namespace hal

#endif  // HAL_DEVICES_SUNS_BH1730FVC_H_