#pragma once

#include "QrCodeGen.hpp"
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

    enum class GraphicEncoding : uint8_t { uncompressed = 0, runLength, tiff, deltaRow };

public:
    ThermalPrinter(Stream &s, bool useTimeout = true) : output{s}, useTimeout{useTimeout} { }
    ~ThermalPrinter() = default;

    void begin();

    void setBold(bool on);
    bool isBold() const { return bold; }

    void setUnderline(bool on);
    bool isUnderline() const { return underline; }

    void setInverse(bool on);
    bool isInverse() const { return inverse; }

    void setUpsideDown(bool on);
    bool isUpsideDown() const { return upsideDown; }

    void setHeightZoom(ZoomLevel level);
    ZoomLevel getHeightZoom() const { return heightZoom; }

    void setDoubleWidth(bool on);
    bool getDoubleWidth() const { return doubleWidth; }

    void setFont(uint8_t f);
    uint8_t getFont() const { return fontIndex; }

    void setCharSpacing(int spacing = 0);
    uint8_t getCharSpacing() const { return charSpacing; }

    void printTextSuperPosition(const char *text);

    void feed(uint8_t lines = 1);

    void feedPixel(uint16_t px);

    void flush() { output.flush(); }

    void setAbsoluteCursor(uint8_t cPos) { setAbsoluteCursor(uint16_t(cPos * 16)); }

    void setAbsoluteCursor(uint16_t pxPos);

    void setBarcodeHeight(uint16_t height = 50) { barcodeHeight = height; }
    uint16_t getBarcodeHeight() const { return barcodeHeight; }

    void setBarcodeWithText(bool on) { barcodeWithText = on; }
    bool isBarcodeWithText() const { return barcodeWithText; }

    void printBarcode(const char *text, BarcodeType type);

    void printBarcode(const String text, BarcodeType type) { printBarcode(text.c_str(), type); }

    bool printQrCode(const char *text, int zoom = -1);

    bool printQrCode(const String text, int zoom = -1) { return printQrCode(text.c_str(), zoom); }

    bool printQrCode(const qrcodegen::QrCode &qrCode, int zoom = -1);

    void setGraphicEncoding(GraphicEncoding compression);

    void printBitmap(size_t width, size_t height, const uint8_t *bitmap);

    template <size_t N> void printTiff(const tiffRaw<N> &tiff) {
        writeCmd(true, cmd::graphicMode, to_underlying(GraphicEncoding::tiff));

        size_t offset = 0;
        for(auto it = tiff.rowData.cbegin(); it != tiff.rowData.cend(); it++) {
            const auto len = *it;
            writeCmd(false, cmd::printGraphicLine, len);
            Print::write(tiff.data + offset, len);
            timeoutWait();
            offset += len;
        }
    }

    void reset();

    // void normal();

    void tab();

    // void setCodePage(uint8_t code = 0);

    // void setLineHeight(int height = 30);

    // void setSize(uint8_t value);

    void clearBuffer() { writeCmd(commandChar, cmd::clearBuffer); }

    // void printTestPage();

    // bool hasPaper();

    void timeoutWait();

private:
    static constexpr char commandChar = 0x1B;
    enum class cmd : uint8_t {
        reset = '@',
        clearBuffer = 'A',
        printBarcode = 'b',
        cutPaper = 'C',
        setUpsideDown = 'D',
        sleepMode = 'e',
        paperFeed = 'F',
        printGraphicLine = 'g',
        setCharHeight = 'H',
        invert = 'I',
        bold = 'J',
        getStatus = 'k',
        underline = 'L',
        setPageLenght = 'l',
        grayValue = 'M',
        graphicMode = 'm',
        setAbsoluteCursorPos = 'N',
        serialEcho = 'n',
        setPageStart = 'o',
        setCharSet = 'P',
        setLightBarrier = 'p',
        relativeTabToDot = 'R',
        setBatteryCircuit = 'r',
        setHorizontalSpace = 'S',
        loadBatchfile = 's',
        doubleWidth = 'W',
        setBlackening = 'Y',
        setPrintQuality = '[',
        reverseFeed = '\\',
    };

    static constexpr uint32_t printerBootTime{2000};
    static constexpr uint32_t byteTime = 250;
    static constexpr size_t pxLine = 384;

    Stream &output;
    bool useTimeout;

    bool bold{false};
    bool underline{false};
    bool inverse{false};
    bool upsideDown{false};
    uint8_t fontIndex{0};
    uint8_t charSpacing{0};
    ZoomLevel heightZoom{ZoomLevel::single};
    GraphicEncoding compression{GraphicEncoding::uncompressed};
    bool doubleWidth{false};

    uint16_t barcodeHeight{100};
    bool barcodeWithText{true};

    size_t column{0};
    size_t maxColumn{48};
    char prevByte{0};

    static constexpr uint32_t dotPrintTime{30000};
    static constexpr uint32_t dotFeedTime{2100};
    uint32_t resumeTime{0};
    void timeoutSet(uint32_t timeout);

    std::pair<size_t, size_t> getMaxSizeCode(BarcodeType t, size_t chars);

    template <typename... T> void writeCmd(bool delay, cmd c, T const &...values) {
        if(delay)
            timeoutWait();
        output.write(commandChar);
        output.write(to_underlying(c));
        ((void)output.write(values), ...);
        const size_t count = sizeof...(T);
        timeoutSet(count * byteTime);
    }

    virtual size_t write(uint8_t c) override;

    template <typename E> constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }

    template <typename T> constexpr T clamp(T v, T min, T max) { return std::min(max, std::max(min, v)); }
};