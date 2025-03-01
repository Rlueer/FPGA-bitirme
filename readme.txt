# FPGA & Arduino-Based Environmental Monitoring System

## Project Overview
This project integrates an **FPGA (DE0-CV) and Arduino Uno** to create a real-time **environmental monitoring and control system**. Using various communication protocols, including **I2C, One-Wire, and VGA**, the system efficiently measures, processes, and visualizes temperature data.

### Features
- **Temperature Monitoring**  
  - DS18B20 temperature sensor connected to **FPGA via One-Wire**
  - Temperature values displayed on a **7-segment LED**
  - **RGB LED** indicates temperature status:
    - **Blue**: Below threshold
    - **Green**: Within the acceptable range
    - **Red**: Above threshold
- **User Interaction via Touchscreen**  
  - A **touchscreen on Arduino Uno** allows users to set temperature thresholds dynamically
  - **I2C communication** between Arduino and FPGA updates the thresholds in real-time
- **VGA Display for Visualization**  
  - FPGA generates a **640x480 VGA output** to display system status and real-time temperature data
- **Communication Protocols Used**  
  - **One-Wire**: FPGA retrieves temperature data from the DS18B20 sensor
  - **I2C**: Arduino sends user-defined thresholds and receives temperature updates
  - **VGA**: FPGA generates graphical output for system monitoring
  - **SPI**: Initially used for the Arduino touchscreen but later removed

## Project Structure
```
📂 FPGA_Arduino_Monitoring
├── 📂 DE0_CV_VGA_Pattern     # VGA function test outputs
├── 📂 arduino_libraries      # Required libraries for touchscreen interface
├── 📂 i2ctemp                # Arduino I2C code for threshold management
├── 📂 verilog_code           # Verilog implementation for FPGA
├── 📂 docs                   # Reports and documentation
├── 📄 README.md              # Project documentation
├── 📄 LICENSE                # License information
└── 📄 FPGA_PIN_ASSIGNMENTS   # DE0-CV FPGA pin mapping
```

## Setup Instructions

### 1. FPGA Configuration
- Compile and upload the **Verilog code** to **DE0-CV FPGA** using **Quartus II**
- Verify correct **pin assignments** based on the **DE0-CV User Guide**

### 2. Arduino Setup
- Upload the `i2ctemp` code to **Arduino Uno**
- Install necessary touchscreen libraries from the `arduino_libraries` folder

### 3. Hardware Connections
- **Connect FPGA and Arduino via I2C**
- **Connect DS18B20 sensor to FPGA** using **One-Wire**
- **Connect VGA output** from FPGA to a monitor

## Results
- Successful integration of **One-Wire, I2C, and VGA communication**
- **Real-time data processing and visualization** is achieved
- Demonstrates **FPGA's capability** in embedded control and visualization applications

## Future Enhancements
- Adding **wireless communication (Wi-Fi, Bluetooth)** for remote monitoring
- Improving **VGA graphics** for better user interaction
- Implementing **data logging** for long-term analysis

## License
This project is licensed under the **MIT License**.

## Contributors
- **Emre Yavuz**

---

This project showcases **FPGA & Arduino-based real-time monitoring and visualization**. 🚀
