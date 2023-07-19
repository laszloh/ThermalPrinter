#pragma once

#include <Arduino.h>


class ThermalPrinter : public Print {
public:
    enum class BarcodeType : uint8_t {
        CODE39 = 'A',
        ITF = 'B',
        EAN13 = 'C',
        EAN8 = 'D',
    };

    template <size_t rCount> struct tiffRaw {
        std::array<size_t, rCount> rowData;
        const uint8_t *data;
    };

    enum class ZoomLevel : uint8_t { single = 0, twice, fourfold, eightfold };

    enum class Align : uint8_t { left = 0, centered, right };

public:
    ThermalPrinter(HardwareSerial &s = Serial2, int8_t rx = -1, int8_t tx = -1) : output{s} { output.begin(115200, SERIAL_8N1, rx, tx); }
    ~ThermalPrinter() = default;

    virtual size_t write(uint8_t c) override;

    void begin();

    void setBold(bool on);
    bool isBold() const { return bold; }

    void setUnderline(bool on);
    bool isUnderline() const { return underline; }

    void setInverse(bool on);
    bool isInverse() const { return inverse; }

    void setUpsideDown(bool on);
    bool isUpsideDown() const { return upsideDown; }

    void setTextPosition(Align align);
    Align getTextPosition() const { return textPosition; }

    void setHeightZoom(ZoomLevel level);
    ZoomLevel getHeightZoom() const { return heightZoom; }

    void setWidthZoom(ZoomLevel level);
    ZoomLevel getWidthZoom() const { return widthZoom; }

    void setFont(uint8_t f);
    uint8_t getFont() const { return fontIndex; }

    void setCharSpacing(int spacing = 0);
    uint8_t getCharSpacing() const { return charSpacing; }

    void printTextSuperPosition(const char *text);

    void feed(uint8_t lines = 1);

    void feedPixel(uint16_t px);

    void flush() { output.flush(); }

    void setCursor(uint8_t cPos) { setCursor(uint16_t(cPos * 16)); }

    void setCursor(uint16_t pxPos);

    void setBarcodeHeight(uint16_t height = 50) { barcodeHeight = height; }
    uint16_t getBarcodeHeight() const { return barcodeHeight; }

    void setBarcodeWithText(bool on) { barcodeWithText = on; }
    bool isBarcodeWithText() const { return barcodeWithText; }

    void printBarcode(const char *text, BarcodeType type);

    void printBarcode(const String text, BarcodeType type) { printBarcode(text.c_str(), type); }

    void printQrCode(const char *text, int zoom = -1);

    void printQrCode(const String text, int zoom = -1) { printQrCode(text.c_str(), zoom); }

    void printBitmap(size_t width, size_t height, const uint8_t *bitmap);

    template <size_t N> void printTiff(const tiffRaw<N> &tiff) {
        writeBytes(true, commandChar, 'm', to_underlying(BitmapCompression::tiff));

        size_t offset = 0;
        for(auto it = tiff.rowData.cbegin(); it != tiff.rowData.cend(); it++) {
            const auto len = *it;
            writeBytes(false, commandChar, 'g', len);
            Print::write(tiff.data + offset, len);
            timeoutWait();
            offset += len;
        }
    }

    void reset();

    // void normal();

    // void tab();

    // void setCodePage(uint8_t code = 0);

    // void setLineHeight(int height = 30);

    // void setSize(uint8_t value);

    void clearBuffer() { writeBytes(commandChar, 'A'); }

    // void printTestPage();

    // bool hasPaper();

    void timeoutWait();

private:
    static constexpr char commandChar = 0x1B;
    static constexpr uint32_t byteTime = 250;
    static constexpr size_t pxLine = 384;

    HardwareSerial &output;

    bool bold{false};
    bool underline{false};
    bool inverse{false};
    bool upsideDown{false};
    uint8_t fontIndex{0};
    uint8_t charSpacing{0};
    Align textPosition{Align::left};
    ZoomLevel heightZoom{ZoomLevel::single};
    ZoomLevel widthZoom{ZoomLevel::single};

    uint16_t barcodeHeight{100};
    bool barcodeWithText{true};

    enum class BitmapCompression : uint8_t { uncompressed = 0, runLength, tiff, deltaRow };

    size_t column{0};
    size_t maxColumn{48};
    char prevByte{0};

    uint32_t dotPrintTime{30000};
    uint32_t dotFeedTime{2100};
    uint32_t resumeTime{0};
    void timeoutSet(uint32_t timeout);

    std::pair<size_t, size_t> getMaxSizeCode(BarcodeType t, size_t chars);

    template <typename... T> void writeBytes(bool delay, T const &...values) {
        if(delay)
            timeoutWait();
        ((void)output.write(values), ...);
        const size_t count = sizeof...(T);
        timeoutSet(count * byteTime);
    }

    template <typename E> constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }
};