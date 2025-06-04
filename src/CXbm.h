#if !defined SRC_CXBM_H
#define SRC_CXBM_H
#include "CBase.h"
#include <vector>
using std::vector;
class CXbm : CNonCopyable {
private:
  CXbm() = delete;

public:
  CXbm(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
       uint16_t uiUpdateIntervalS = 0)
      : m_uiX(x), m_uiY(y), m_uiW(w), m_uiH(h), m_uiB(w / 8), m_uiS(w / 8 * h),
        m_uiUpdateIntervalS(uiUpdateIntervalS), m_uiTimer(0),
        m_pBuffer(nullptr) {
    m_pBuffer = new unsigned char[m_uiS];
  }
  ~CXbm() override {
    if (m_pBuffer != nullptr) {
      delete[] m_pBuffer;
    }
  }

  void SetPixel(uint8_t x, uint8_t y) {
    if (x < m_uiW && y < m_uiH) {
      uint16_t nOffs = y * m_uiB + x / 8;
      m_pBuffer[nOffs] |= (1 << (x % 8));
      // CControl::Log(CControl::I, "CXbm::SetPixel(%u, %u), nOffs=%lu ->
      // 0x%02X",                    x, y, nOffs, c);
    }
  }
  void DelPixel(uint8_t x, uint8_t y) {
    if (x < m_uiW && y < m_uiH) {
      uint16_t nOffs = y * m_uiB + x / 8;
      m_pBuffer[nOffs] &= ~(1 << (x % 8));
      // CControl::Log(CControl::I, "CXbm::DelPixel(%u, %u), nOffs=%lu ->
      // 0x%02X",                    x, y, nOffs, c);
    }
  }
  void Clear() { memset(m_pBuffer, 0x00, m_uiS); }

  void FromVector(vector<uint8_t> &vec) {
    Clear();
    for (uint8_t n = 0; n < vec.size(); n++) {
      uint8_t x = m_uiW - n - 1;
      SetPixel(x, vec[n]);
    }
  }
  void FromVectorI(vector<uint8_t> &vec) {
    Clear();
    for (uint8_t n = 0; n < vec.size(); n++) {
      uint8_t x = n;
      // uint8_t y = vec[n];
      // if (y >= m_uiH)
      //   y = m_uiH - 1;
      // SetPixel(x, y);
      SetPixel(x, vec[n]);
    }
  }
  
  const unsigned char* getBuffer() const { return m_pBuffer; }

  const uint16_t getBufferSize() const { return m_uiS; }

  const uint8_t m_uiX;
  const uint8_t m_uiY;
  const uint8_t m_uiW;
  const uint8_t m_uiH;
  const uint8_t m_uiB;
  uint16_t m_uiS;
  uint16_t m_uiUpdateIntervalS;
  uint32_t m_uiTimer;

  unsigned char *m_pBuffer;
};
#endif // SRC_CXBM_H
