#include <stdint.h>
#include <string>

extern "C" {

void _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(uint32_t inWidth, uint32_t inHeight, int inFormat, uint32_t inUsage, std::string requestorName);

void _ZN7android13GraphicBufferC1Ejjij(uint32_t inWidth, uint32_t inHeight, int inFormat, uint32_t inUsage) {
    std::string requestorName = "<Unknown>";
    _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(inWidth, inHeight, inFormat, inUsage, requestorName);
}
}
