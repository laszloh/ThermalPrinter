#include <Arduino.h>
#include <ThermalPrinter.h>
#include <esp32-hal-log.h>

ThermalPrinter printer(Serial2, 25, 26);

void setup() {
    Serial.begin(115200);
    // Serial2.begin(115200, SERIAL_8N1, 25, 26);
    // log_i("Booting up");

    // Serial.onReceive([](){
    //     size_t available = Serial.available();
    //     while(available--)
    //         Serial2.write(Serial.read());
    // }, true);
    // Serial2.onReceive([](){
    //     size_t available = Serial2.available();
    //     while(available--)
    //         Serial.write(Serial2.read());
    // }, true);

    delay(5000);

    printer.begin();

    // printer.println("Hello World");
    // // // printer.printBinaryFile(aws_root_ca_pem_start, aws_root_ca_pem_end-aws_root_ca_pem_start);
    // printer.printBarcode("123ABC", ThermalPrinter::BarcodeType::CODE39);
    // printer.feed(3);

    // printer.printQrCode("Hello World", 8);

    // printer.println("Font test:");
    // uint8_t i = 4;
    // printer.setFont(i);
    // delay(100);
    // printer.printf("\tFont %d\n", i);
    // for(char c = 0; c < 255; c++) {
    //     printer.print(c);
    // }
    // printer.println();
    // delay(100);
}

void loop() {

    size_t count = Serial.available();
    while(count--){
        const char c = Serial.read();
        Serial2.print(c);
    }
    count = Serial2.available();
    while(count--) {
        const char c = Serial2.read();
        Serial.print(c);
    }

    // while(Serial2.available()) {
    //     int c = Serial2.read();
    //     Serial.printf("%c (0x%02x) | ", c, c);
    // }
    // static uint32_t lastPrint = millis();
    // if((millis() - lastPrint) > 10000) {
    //     // log_d("alive");
    //     lastPrint = millis();
    // }
}