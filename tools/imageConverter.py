from PIL import Image
import numpy as np


def encode(data):
    """
    Encodes data using PackBits encoding.
    """
    if len(data) == 0:
        return data

    if len(data) == 1:
        return b'\x00' + data

    data = bytearray(data)

    result = bytearray()
    buf = bytearray()
    pos = 0
    repeat_count = 0
    MAX_LENGTH = 127

    # we can safely start with RAW as empty RAW sequences
    # are handled by finish_raw()
    state = 'RAW'

    def finish_raw():
        if len(buf) == 0:
            return
        result.append(len(buf)-1)
        result.extend(buf)
        buf[:] = bytearray()

    def finish_rle():
        result.append(256-(repeat_count - 1))
        result.append(data[pos])

    while pos < len(data)-1:
        current_byte = data[pos]

        if data[pos] == data[pos+1]:
            if state == 'RAW':
                # end of RAW data
                finish_raw()
                state = 'RLE'
                repeat_count = 1
            elif state == 'RLE':
                if repeat_count == MAX_LENGTH:
                    # restart the encoding
                    finish_rle()
                    repeat_count = 0
                # move to next byte
                repeat_count += 1

        else:
            if state == 'RLE':
                repeat_count += 1
                finish_rle()
                state = 'RAW'
                repeat_count = 0
            elif state == 'RAW':
                if len(buf) == MAX_LENGTH:
                    # restart the encoding
                    finish_raw()

                buf.append(current_byte)

        pos += 1

    if state == 'RAW':
        buf.append(data[pos])
        finish_raw()
    else:
        repeat_count += 1
        finish_rle()

    return bytes(result)


def level(x):
    return True if x > thresh else False


img = Image.open("test.png")
thresh = 200

r = img.convert('L').point(level, mode='1')
img_array = np.array(r.getdata()).reshape((r.size[1], r.size[0]))

output = {"rowData": [], "data": bytearray()}

for imgRow in img_array:
    packed = np.packbits(imgRow)
    packBits = encode(packed)
    row = {"offset": len(output['data']), "len": len(packBits)}
    output['rowData'].append(row)
    output['data'] += bytearray(packBits)


