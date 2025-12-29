/*
  Canbang2banh_Robot - Xe tự cân bằng 2 bánh dùng MPU6050 + L298N
  Tác giả: Tùng Lâm Automatic

  Mô tả:
  - Đọc góc nghiêng từ MPU6050 qua I2C.
  - Điều khiển PID để giữ thăng bằng.

  Sơ đồ chân (Arduino Mega/Uno):
  - I2C: SDA/SCL (Mega: D20/D21, Uno: A4/A5)
  - L298N:
    IN1 (động cơ 1) -> D6
    IN2 (động cơ 1) -> D9
    IN3 (động cơ 2) -> D11
    IN4 (động cơ 2) -> D10

  Lưu ý: Nguồn động cơ và Arduino phải chung GND.
*/

#include <Wire.h>        // Thư viện I2C.
#include "stmpu6050.h"  // Thư viện MPU6050 (lọc bổ sung).

SMPU6050 mpu; // Đối tượng MPU6050.

// Chân điều khiển cho L298N (giữ nguyên tên biến theo code gốc).
const int IN3 = 6;  // Chân IN1 cho động cơ 1.
const int IN4 = 9;  // Chân IN2 cho động cơ 1.
const int IN1 = 11; // Chân IN3 cho động cơ 2.
const int IN2 = 10; // Chân IN4 cho động cơ 2.

// Biến PID.
double Kp = 20.0; // Hệ số tỉ lệ.
double Ki = 0.8;  // Hệ số tích phân.
double Kd = 0.05; // Hệ số vi phân.

double targetAngle = 0; // Góc mục tiêu (0 = đứng thẳng).
double integral = 0;    // Thành phần tích phân.
double lastError = 0;   // Sai số ở lần trước.

// Giới hạn tích phân để chống bão hòa.
const double integralLimit = 100;

void setup() {
  Serial.begin(115200); // Mở Serial để theo dõi giá trị PID.

  mpu.init(0x68);     // Địa chỉ I2C mặc định của MPU6050.
  // mpu.Reset();     // Có thể gọi nếu muốn reset góc tham chiếu.
  mpu.calibrate(1000); // Hiệu chuẩn gyro (giữ robot đứng yên).

  // Cấu hình chân điều khiển L298N.
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  double yAngle = mpu.getZAngle(); // Lấy góc nghiêng (trục Z theo thư viện).

  // Tính sai số giữa góc mục tiêu và góc hiện tại.
  double error = targetAngle - yAngle;

  // Tính đạo hàm sai số (không chia dt để đơn giản).
  double derivative = error - lastError;

  // Tính điều khiển PID (integral dùng giá trị lần trước).
  int motorSpeed = Kp * error + Ki * integral + Kd * derivative;

  // Vùng chết: bỏ qua nhiễu nhỏ quanh 0.
  if (motorSpeed > -55 && motorSpeed < 55) motorSpeed = 0;

  // Giới hạn tốc độ PWM an toàn cho động cơ.
  motorSpeed = constrain(motorSpeed, -120, 120);

  // In thông số để theo dõi.
  Serial.print(error);
  Serial.print("     ");
  Serial.print(derivative);
  Serial.print("     ");
  Serial.print(integral);
  Serial.print("     ");
  Serial.println(motorSpeed);

  // Chống bão hòa tích phân (anti-windup).
  if (motorSpeed >= 180) {
    // Nếu vượt giới hạn dương thì giữ nguyên integral.
    integral = integral;
  } else if (motorSpeed <= -180) {
    // Nếu vượt giới hạn âm thì giữ nguyên integral.
    integral = integral;
  } else {
    // Cập nhật integral bình thường và giới hạn lại.
    integral += error;
    integral = constrain(integral, -integralLimit, integralLimit);
  }

  // Điều khiển động cơ theo chiều sai số.
  if (motorSpeed > 0) {
    // Động cơ 1 tiến.
    analogWrite(IN1, motorSpeed);  // PWM chiều tiến.
    analogWrite(IN2, 0);           // Tắt chiều ngược.

    // Động cơ 2 tiến.
    analogWrite(IN3, motorSpeed);  // PWM chiều tiến.
    analogWrite(IN4, 0);           // Tắt chiều ngược.
  } else {
    // Động cơ 1 lùi.
    analogWrite(IN1, 0);            // Tắt chiều tiến.
    analogWrite(IN2, -motorSpeed);  // PWM chiều lùi.

    // Động cơ 2 lùi.
    analogWrite(IN3, 0);            // Tắt chiều tiến.
    analogWrite(IN4, -motorSpeed);  // PWM chiều lùi.
  }

  // Cập nhật sai số lần trước.
  lastError = error;

  // Chu kỳ điều khiển (ms).
  delay(30);
}
