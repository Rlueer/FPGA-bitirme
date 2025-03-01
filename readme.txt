# **FPGA & Arduino-Based Environmental Monitoring System**

## **Project Overview**
This project integrates **FPGA (DE0-CV) and Arduino Uno** to develop a **multi-functional environmental monitoring and control system**. Various communication protocols, including **I2C, One-Wire, and VGA**, are used to ensure seamless data exchange between components.

### **Key Features**
- **Real-time Temperature Monitoring**:  
  - DS18B20 temperature sensor connected to FPGA via **One-Wire** protocol.
  - Measured temperature values displayed on **7-segment LED**.
  - An **RGB LED** provides visual feedback on temperature thresholds:
    - **Blue**: Below threshold.
    - **Green**: Within the acceptable range.
    - **Red**: Above threshold.

- **User Interface with Touchscreen**:  
  - A **touchscreen interface on Arduino Uno** allows dynamic adjustment of temperature thresholds.
  - Arduino communicates with FPGA using **I2C** to update the threshold values.

- **VGA Display Integration**:  
  - An **FPGA-integrated VGA module** provides a **640x480 resolution graphical interface**.
  - Displays system status and real-time temperature data.

- **Communication Protocols**:  
  - **One-Wire**: FPGA retrieves temperature data from the DS18B20 sensor.
  - **I2C**: Arduino sends user-defined thresholds and retrieves temperature readings from FPGA.
  - **VGA**: FPGA generates a graphical representation of system data.
  - **SPI**: Initially used for Arduino touchscreen communication but later removed.

---

## **Project Structure**
