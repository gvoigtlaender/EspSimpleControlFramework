#if !defined SRC_CXBM_H
#define SRC_CXBM_H
#include <vector>
using std::vector;
class CXbm {
private:
  CXbm() {}

public:
  CXbm(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
      : m_uiX(x), m_uiY(y), m_uiW(w), m_uiH(h), m_uiB(0), m_uiS(0),
        m_pBuffer(NULL) {
    m_uiB = w / 8;
    m_uiS = m_uiB * h;
    m_pBuffer = new unsigned char[m_uiS];
  }
  ~CXbm() {
    if (m_pBuffer != NULL)
      delete[] m_pBuffer;
  }

  void SetPixel(uint8_t x, uint8_t y) {
    if (x < m_uiW && y < m_uiH) {
      uint16_t nOffs = y * m_uiB + x / 8;
      unsigned char &c = m_pBuffer[nOffs];
      c |= (1 << (x % 8));
      // CControl::Log(CControl::I, "CXbm::SetPixel(%u, %u), nOffs=%lu ->
      // 0x%02X",                    x, y, nOffs, c);
    }
  }
  void DelPixel(uint8_t x, uint8_t y) {
    if (x < m_uiW && y < m_uiH) {
      uint16_t nOffs = y * m_uiB + x / 8;
      unsigned char &c = m_pBuffer[nOffs];
      c &= ~(1 << (x % 8));
      // CControl::Log(CControl::I, "CXbm::DelPixel(%u, %u), nOffs=%lu ->
      // 0x%02X",                    x, y, nOffs, c);
    }
  }
  void Clear() { memset(m_pBuffer, 0x00, m_uiS); }

  void FromVector(vector<uint8> &vec) {
    Clear();
    for (uint8 n = 0; n < vec.size(); n++) {
      uint8 x = m_uiW - n - 1;
      SetPixel(x, vec[n]);
    }
  }
  void FromVectorI(vector<uint8> &vec) {
    Clear();
    for (uint8 n = 0; n < vec.size(); n++) {
      uint8 x = n;
      SetPixel(x, vec[n]);
    }
  }

  uint8_t m_uiX;
  uint8_t m_uiY;
  uint8_t m_uiW;
  uint8_t m_uiH;
  uint8_t m_uiB;
  uint16_t m_uiS;

  unsigned char *m_pBuffer;
};
#endif // SRC_CXBM_H