<div align="center">
  <h1>Canbang2banh_Robot</h1>
  <p><strong>Xe tự cân bằng 2 bánh dùng MPU6050 + PID</strong></p>
  <p>Tác giả: Tùng Lâm Automatic</p>
  <p>
    <img src="https://img.shields.io/badge/Board-Arduino%20Mega%2FUno-0B7C3E" />
    <img src="https://img.shields.io/badge/Sensor-MPU6050-1D4ED8" />
    <img src="https://img.shields.io/badge/Driver-L298N-F97316" />
    <img src="https://img.shields.io/badge/Control-PID-111827" />
  </p>
</div>

---

## Giới thiệu
Dự án điều khiển xe tự cân bằng 2 bánh bằng MPU6050. Góc nghiêng được đọc qua I2C, lọc bổ sung (complementary filter), sau đó PID tính tốc độ động cơ để giữ robot đứng thẳng.

## Tính năng
- Đọc góc nghiêng từ MPU6050.
- PID điều khiển cân bằng theo thời gian thực.
- Điều khiển L298N bằng PWM.
- Có hiệu chuẩn gyro khi khởi động.

## Phần cứng
- Arduino Mega 2560 hoặc Arduino Uno.
- MPU6050 (GY-521).
- Driver L298N.
- 2 động cơ DC + bánh xe.

## Sơ đồ chân đấu nối

### MPU6050 (I2C)
| MPU6050 | Mega 2560 | Uno |
| --- | --- | --- |
| SDA | D20 | A4 |
| SCL | D21 | A5 |
| VCC | 5V (hoặc 3.3V) | 5V (hoặc 3.3V) |
| GND | GND | GND |

### L298N
| L298N | Arduino | Ghi chú |
| --- | --- | --- |
| IN1 | D6 | Động cơ 1 tiến |
| IN2 | D9 | Động cơ 1 lùi |
| IN3 | D11 | Động cơ 2 tiến |
| IN4 | D10 | Động cơ 2 lùi |
| ENA/ENB | PWM | Dùng `analogWrite` trên các chân PWM |

> Lưu ý: Nguồn động cơ và Arduino phải chung GND.

## Thuật toán
- Góc X/Y lấy từ gia tốc + gyro; góc Z tích phân từ gyro.
- Bộ lọc bổ sung: 98% gyro + 2% acc.
- PID: `motorSpeed = Kp*error + Ki*integral + Kd*derivative`.

## Thư viện
- `Wire.h` (I2C).
- `stmpu6050.h` (đã kèm trong thư mục dự án).

## Cách nạp
1. Mở `Canbang2banh_Robot/Canbang2banh_Robot.ino` trong Arduino IDE.
2. Chọn board phù hợp (Mega/Uno).
3. Kết nối đúng chân I2C và L298N.
4. Upload, giữ robot đứng yên để hiệu chuẩn gyro.

## Hiệu chỉnh PID
- Bắt đầu với Kp nhỏ, tăng dần đến khi robot có thể đứng.
- Tăng Kd để giảm rung/dao động.
- Thêm Ki để bù sai lệch lâu dài.
- Có thể chỉnh `targetAngle` nếu robot bị nghiêng về một phía.

## Lưu ý vận hành
- Cân chỉnh trọng tâm gần trục bánh.
- Dùng pin đủ dòng cho động cơ.
- Đảm bảo cảm biến MPU6050 được cố định chắc chắn.

## Tác giả
Tùng Lâm Automatic

