Arduino Uno ile FPGA arasındaki iletişimi I2C ve One-Wire protokolleri üzerinden sağlayan Verilog kodu hazır. FPGA için pin atamaları DE0-CV Kullanıcı Kılavuzu'na göre yapılmıştır. Sistemi çalıştırmak için kodu derleyip FPGA'ya yüklemeniz yeterlidir.

DE0_CV_VGA_Pattern klasöründe, VGA fonksiyonlarını basit desenlerle test ederek çıktıları doğruladım.

Başlangıçta SPI iletişimi uyguladım ancak daha sonra kullanmamaya karar verdim. arduino_libraries klasörü, dokunmatik ekran arayüzünü kullanmak için gerekli bazı kütüphaneleri içeriyor.

Ana Arduino kodu i2ctemp klasöründedir ve Arduino'ya yüklenmelidir. Bu kod, sıcaklık eşik değerlerini ve diğer iletişim görevlerini yönetir.

