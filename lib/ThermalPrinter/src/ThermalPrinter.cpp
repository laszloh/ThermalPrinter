#include <Arduino.h>
#include <bitset>

#include "QrCodeGen.hpp"
#include "ThermalPrinter.h"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;

void ThermalPrinter::begin() {
    // wait for printer startup
    timeoutSet(std::clamp<uint32_t>(2000 - millis(), 0, 2000) * 1000);

    // reset();
}

void ThermalPrinter::reset() {
    writeBytes(false, commandChar, '@');
    timeoutWait();

    writeBytes(true, commandChar, 'e', 0, 0);

    writeBytes(true, commandChar, 'A');
    writeBytes(true, commandChar, 'm', 0x05);
    writeBytes(true, commandChar, 'm', 0x01);
    writeBytes(true, commandChar, 'g', 2, 'O', 0x00);
    writeBytes(true, commandChar, '[', 64, 8);
    writeBytes(true, commandChar, 'Y', 0x1E);
    writeBytes(true, commandChar, 'A');
    writeBytes(true, commandChar, 'm', 0x05);
    writeBytes(true, commandChar, 'F', 0, 2);
    writeBytes(true, commandChar, 'F', 0, 73);

    bold = false;
    underline = false;
    inverse = false;
    upsideDown = false;
    fontIndex = 0;
    charSpacing = 0;
    textPosition = Align::left;
    heightZoom = ZoomLevel::single;
    widthZoom = ZoomLevel::single;

    barcodeHeight = 80;
    barcodeWithText = true;


    dotPrintTime = 30000;
    dotFeedTime = 2100;
}

void ThermalPrinter::timeoutSet(uint32_t timeout) { resumeTime = micros() + timeout; }

void ThermalPrinter::timeoutWait() {
    while(int32_t(micros() - resumeTime) < 0)
        yield();
}

size_t ThermalPrinter::write(uint8_t c) {
    // strip carriage return
    if(c != '\r') {
        timeoutWait();
        output.write(c);
        uint32_t delay = byteTime;
        if(c == '\n')
            delay += to_underlying(heightZoom) * 32 * dotFeedTime;
        timeoutSet(delay);
    }
    return 1;
}

void ThermalPrinter::setBold(bool on) {
    bold = on;
    writeBytes(true, commandChar, 'J', (on) ? '1' : '0');
}

void ThermalPrinter::setUnderline(bool on) {
    underline = on;
    writeBytes(true, commandChar, 'L', (on) ? '1' : '0');
}

void ThermalPrinter::setInverse(bool on) {
    inverse = on;
    writeBytes(true, commandChar, 'I', (on) ? '1' : '0');
}

void ThermalPrinter::setUpsideDown(bool on) {
    upsideDown = on;
    writeBytes(true, commandChar, 'D', (on) ? '1' : '0');
}

void ThermalPrinter::setTextPosition(Align align) { }

void ThermalPrinter::setHeightZoom(ZoomLevel level) {
    heightZoom = level;
    const uint8_t l = to_underlying(level);
    writeBytes(true, commandChar, 'H', l);
}

void ThermalPrinter::setWidthZoom(ZoomLevel level) {
    widthZoom = level;
    const uint8_t val = to_underlying(level);
    writeBytes(true, commandChar, 'D', val);
}

void ThermalPrinter::setFont(uint8_t f) {
    fontIndex = std::min<uint8_t>(f, 4);
    writeBytes(true, commandChar, 'P', fontIndex);
}


void ThermalPrinter::setCharSpacing(int spacing) {
    charSpacing = std::clamp<uint8_t>(spacing, 0, 15);
    writeBytes(true, commandChar, 'S', charSpacing);
}


void ThermalPrinter::feed(uint8_t lines) {
    for(int i = 0; i < lines; i++) {
        writeBytes(true, '\n');
    }
}

void ThermalPrinter::feedPixel(uint16_t px) { writeBytes(true, commandChar, 'F', (px >> 8) & 0xFF, px & 0xFF); }


void ThermalPrinter::printBarcode(const char *text, BarcodeType type) {
    char cType = to_underlying(type);
    cType = std::tolower(cType);
    const size_t sLen = strlen(text);
    const auto [size, width] = getMaxSizeCode(type, sLen);
    const uint16_t left = (pxLine - width) / 2;

    // print barcode
    writeBytes(true, commandChar, 'b', cType, size, (left >> 8) & 0xFF, left & 0xFF, (barcodeHeight >> 8) & 0xFF, barcodeHeight & 0xFF, sLen);
    Print::print(text);

    if(barcodeWithText) {
        const uint16_t textOffset = (pxLine - sLen * 16) / 2;
        setCursor(textOffset);
        Print::print(text);
    }
    timeoutSet((barcodeHeight + 40 * dotPrintTime));
}

void ThermalPrinter::setCursor(uint16_t pxPos) { writeBytes(commandChar, 'N', (pxPos >> 8) & 0xFF, pxPos & 0xFF); }

std::pair<size_t, size_t> ThermalPrinter::getMaxSizeCode(BarcodeType t, size_t chars) {
    constexpr std::array<std::pair<uint8_t, uint8_t>, 8> sizes = {{{2, 5}, {2, 6}, {3, 7}, {4, 9}, {5, 12}, {6, 14}, {7, 16}, {8, 18}}};

    auto lenLambda = +[](uint8_t narrow, uint8_t wide, size_t chars) -> size_t {
        // this is the default mode (CODE39)
        return 6 * wide + 14 * narrow + chars * (3 * wide + 7 * narrow);
    };

    switch(t) {
        case BarcodeType::ITF:
            lenLambda = +[](uint8_t narrow, uint8_t wide, size_t chars) -> size_t { return 1 * wide + 6 * narrow + chars * (2 * wide + 3 * narrow); };
            break;

        case BarcodeType::EAN13:
        case BarcodeType::EAN8:
            lenLambda = +[](uint8_t narrow, uint8_t wide, size_t chars) -> size_t { return 95 * narrow; };
            break;

        case BarcodeType::CODE39:
        default:
            break;
    }

    int idx = sizes.size();
    for(auto it = sizes.crbegin(); it != sizes.crend(); it++, idx--) {
        const auto [narrow, wide] = *it;
        size_t len = lenLambda(narrow, wide, chars);
        if(len < pxLine)
            return std::make_pair(idx - 1, len);
    }

    return std::make_pair(0, pxLine);
}

void ThermalPrinter::printQrCode(const char *text, int zoom) {
    constexpr QrCode::Ecc eccLvl = QrCode::Ecc::ECC_LOW;

    const QrCode qr = QrCode::encodeText(text, eccLvl);
    printQrCode(qr, zoom);
}

void ThermalPrinter::printQrCode(const qrcodegen::QrCode &qrCode, int zoom) {
    constexpr uint8_t border = 4;
    const size_t qrSize = qr.getSize();

    if(zoom == -1) {
        zoom = pxLine / (2 * border + qrSize);
    } else if((2 * border + qrSize) * zoom > pxLine) {
        log_e("QR code too big");
        return;
    }

    const int pxOffset = ((pxLine - (2 * border + qrSize) * zoom) / 2) - 1;

    log_i("Printing qr code: %d", qrSize);
    feed();
    writeBytes(true, commandChar, 'm', 0);
    for(int y = -border; y < int(qrSize + border); y++) {
        // here we prints a module row
        std::bitset<pxLine> rowBits(0);
        constexpr std::bitset<pxLine> mask(0xFF);
        int byteDelay = 0;
        uint8_t row[pxLine / 8];

        for(size_t i = 0; i < qrSize + 2 * border; i++) {
            if(qr.getModule(i - border, y)) {
                const int firstIdx = pxOffset + (i * zoom);
                byteDelay++;
                for(int j = firstIdx; j < firstIdx + zoom; j++) {
                    rowBits.set(j);
                }
            }
        }

        constexpr uint8_t lookup[16] = {
            0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
        };
        for(size_t i = 0; i < sizeof(row); i++) {
            const auto val = static_cast<uint8_t>(((rowBits >> (8 * i)) & mask).to_ulong());
            row[i] = (lookup[val & 0x0F] << 4) | lookup[val >> 4];
        }

        for(size_t i = 0; i < zoom; i++) {
            writeBytes(false, commandChar, 'g', sizeof(row));
            delayMicroseconds(10);
            output.write(row, sizeof(row));
            output.flush();
            delayMicroseconds(byteDelay * 32 * byteTime);
        }
        delay(100);
    }
}

// void ThermalPrinter::setBitmapCompression(BitmapCompression compression) {
//     this->compression = compression;
//     const uint8_t val = to_underlying(compression);
//     writeBytes(true, commandChar, 'm', val);
// }

void ThermalPrinter::printBitmap(size_t width, size_t height, const uint8_t *bitmap) {
    constexpr size_t maxRowBytes = 48;
    const size_t rowBytes = std::min(maxRowBytes, (width + 7) / 8);
    writeBytes(true, commandChar, 'm', to_underlying(BitmapCompression::uncompressed));

    for(size_t i = 0; i < height; i++) {
        writeBytes(false, commandChar, 'g', maxRowBytes);
        Print::write(bitmap + i * rowBytes, rowBytes);
        // print remaining bytes until the end of the line
        for(size_t j = rowBytes; j < maxRowBytes; j++)
            Print::write(uint8_t(0x00));
        timeoutWait();
        // move head to the next line
    }
}
