#include <Arduino.h>
#include <ThermalPrinter.h>

#include "images.h"

ThermalPrinter printer(Serial1);

void setup() {
    Serial.begin(115200);

    Serial1.setPinout(5,6);
    Serial1.setCTS(7);
    Serial1.setRTS(8);
    Serial1.begin();

    printer.begin();

    printer.println("Hello World");
    // printer.printBinaryFile(aws_root_ca_pem_start, aws_root_ca_pem_end-aws_root_ca_pem_start);
    printer.printBarcode("123ABC", ThermalPrinter::BarcodeType::CODE39);
    printer.feed(3);

    printer.printQrCode("Hello World", 8);

    printer.printTiff(test_png);

    printer.println("Font test:");
    uint8_t i = 4;
    printer.setFont(i);
    delay(100);
    printer.printf("\tFont %d\n", i);
    for(char c = 0; c < 255; c++) {
        printer.print(c);
    }
    printer.println();
    delay(100);
}

void loop() {

    size_t count = Serial.available();
    while(count--){
        const char c = Serial.read();
        Serial1.print(c);
    }
    count = Serial1.available();
    while(count--) {
        const char c = Serial1.read();
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