#ifndef _SAIGONTECH_MPU6050_H_
#define _SAIGONTECH_MPU6050_H_

#include "Wire.h"

// Hằng số đổi radian sang độ.
#define RAD2DEG 57.295779

// Lớp đọc MPU6050 và tính góc bằng bộ lọc bổ sung.
class SMPU6050 {
  public:
    SMPU6050() {}

    // Khởi tạo MPU6050 với địa chỉ I2C.
    void init(int address) {
      this->i2cAddress = address;
      this->gyroXOffset = 0;
      this->gyroYOffset = 0;
      this->gyroZOffset = 0;
      this->xAngle = 0;
      this->yAngle = 0;
      this->zAngle = 0;
      this->accX = 0;
      this->accY = 0;
      this->prevMillis = millis();

      // Reset và cấu hình MPU6050.
      Wire.begin();
      Wire.beginTransmission(this->i2cAddress);
      Wire.write(0x6B);  // PWR_MGMT_1 register.
      Wire.write(0);     // Thoát chế độ sleep.
      Wire.endTransmission(false);

      Wire.beginTransmission(this->i2cAddress);
      Wire.write(0x19);  // SMPLRT_DIV register.
      Wire.write(0);     // Chia tần số = 0.
      Wire.endTransmission(false);

      Wire.beginTransmission(this->i2cAddress);
      Wire.write(0x1B);  // GYRO_CONFIG register.
      Wire.write(0);     // Full scale = ±250 °/s.
      Wire.endTransmission(false);

      Wire.beginTransmission(this->i2cAddress);
      Wire.write(0x1C);  // ACCEL_CONFIG register.
      Wire.write(0);     // Full scale = ±2 g.
      Wire.endTransmission(false);
    }

    // Hiệu chuẩn gyro bằng cách lấy trung bình nhiều mẫu.
    void calibrate(int times) {
      long gyroXTotal = 0, gyroYTotal = 0, gyroZTotal = 0;
      int count = 0;
      int gyroRawX, gyroRawY, gyroRawZ;
      for (int i = 0; i < times; i++) {
        Wire.beginTransmission(this->i2cAddress);
        Wire.write(0x43);  // GYRO_XOUT register.
        Wire.endTransmission(false);
        Wire.requestFrom(this->i2cAddress, 6, true);

        gyroRawX = Wire.read() << 8 | Wire.read();
        gyroRawY = Wire.read() << 8 | Wire.read();
        gyroRawZ = Wire.read() << 8 | Wire.read();

        gyroXTotal += gyroRawX;
        gyroYTotal += gyroRawY;
        gyroZTotal += gyroRawZ;
        count += 1;
      }
      // Offset để đưa gyro về 0 khi đứng yên.
      gyroXOffset = -gyroXTotal * 1.0 / count;
      gyroYOffset = -gyroYTotal * 1.0 / count;
      gyroZOffset = -gyroZTotal * 1.0 / count;
    }

    // Đặt góc hiện tại làm mốc 0.
    void Reset() {
      this->xRef = this->xAngle;
      this->yRef = this->yAngle;
      this->zRef = this->zAngle;
    }

    // Lấy góc X (độ) so với mốc.
    double getXAngle() {
      this->readAngles();
      return this->xAngle - this->xRef;
    }

    // Lấy góc Y (độ) so với mốc.
    double getYAngle() {
      this->readAngles();
      return this->yAngle - this->yRef;
    }

    // Lấy góc Z (độ) so với mốc.
    double getZAngle() {
      this->readAngles();
      return this->zAngle - this->zRef;
    }

    // Lấy đồng thời góc X/Y/Z.
    void getXYZAngles(double &x, double &y, double &z) {
      this->readAngles();
      x = this->xAngle - this->xRef;
      y = this->yAngle - this->yRef;
      z = this->zAngle - this->zRef;
    }

  private:
    int i2cAddress;
    double accX, accY, gyroX, gyroY, gyroZ;
    double gyroXOffset, gyroYOffset, gyroZOffset;
    double xAngle, yAngle, zAngle;
    double xRef = 0, yRef = 0, zRef = 0;
    unsigned long prevMillis;

    // Đọc cảm biến và cập nhật góc bằng bộ lọc bổ sung.
    void readAngles() {
      // Giới hạn tần số đọc ~3 ms/lần.
      if (millis() - this->prevMillis < 3) return;

      int accRawX, accRawY, accRawZ, gyroRawX, gyroRawY, gyroRawZ;

      Wire.beginTransmission(this->i2cAddress);
      Wire.write(0x3B); // ACCEL_XOUT_H register.
      Wire.endTransmission(false);
      Wire.requestFrom(this->i2cAddress, 14, true);

      accRawX = Wire.read() << 8 | Wire.read();
      accRawY = Wire.read() << 8 | Wire.read();
      accRawZ = Wire.read() << 8 | Wire.read();
      Wire.read(); Wire.read(); // Bỏ qua nhiệt độ.
      gyroRawX = Wire.read() << 8 | Wire.read();
      gyroRawY = Wire.read() << 8 | Wire.read();
      gyroRawZ = Wire.read() << 8 | Wire.read();

      // Góc từ gia tốc kế (độ).
      accX = atan((accRawY / 16384.0) / sqrt(pow((accRawX / 16384.0), 2) + pow((accRawZ / 16384.0), 2))) * RAD2DEG;
      accY = atan(-1 * (accRawX / 16384.0) / sqrt(pow((accRawY / 16384.0), 2) + pow((accRawZ / 16384.0), 2))) * RAD2DEG;

      // Tốc độ quay từ gyro (độ/giây).
      gyroX = (gyroRawX + gyroXOffset) / 131.0;
      gyroY = (gyroRawY + gyroYOffset) / 131.0;
      gyroZ = (gyroRawZ + gyroZOffset) / 131.0;

      // Tính thời gian lấy mẫu (giây).
      unsigned long curMillis = millis();
      double duration = (curMillis - prevMillis) * 1e-3;
      prevMillis = curMillis;

      // Lọc bổ sung: gyro chiếm 98%, acc chiếm 2%.
      xAngle = 0.98 * (xAngle + gyroX * duration) + 0.02 * accX;
      yAngle = 0.98 * (yAngle + gyroY * duration) + 0.02 * accY;

      // Trục Z chỉ tích phân từ gyro (không có acc hỗ trợ).
      zAngle = zAngle + gyroZ * duration;
    }
};

// Hàm tiện ích: khởi tạo MPU6050.
void mpu6050Init(SMPU6050 &smpu, int address) {
  smpu.init(address);
}

// Hàm tiện ích: hiệu chuẩn MPU6050.
void mpu6050Calibrate(SMPU6050 &smpu, int times) {
  smpu.calibrate(times);
}

// Hàm tiện ích: đọc góc XYZ.
void mpu6050GetXYZAngles(SMPU6050 &smpu, double &xAngle, double &yAngle, double &zAngle) {
  smpu.getXYZAngles(xAngle, yAngle, zAngle);
}

#endif /* _SAIGONTECH_MPU6050_H_ */
