#if !defined SRC_CDISPLAYLINE_H_
#define SRC_CDISPLAYLINE_H_
#include <Arduino.h>
#include <string>

class CDisplayLine {
public:
  CDisplayLine(uint8_t x, uint8_t y, uint8_t uiNoOfColumns,
               const uint8_t *pFont)
      : m_uiX(x), m_uiY(y), m_uiNoOfColumns(uiNoOfColumns), m_sLineToDraw(""),
        m_sLineDrawn(""), m_sLine(""), m_iScrollIdx(0), m_pFont(pFont) {}

  virtual ~CDisplayLine() {}

  virtual void Line(const std::string &sLineContent) {
    m_sLine = sLineContent;
    if (sLineContent.length() <= m_uiNoOfColumns) {
      m_iScrollIdx = 0;
      m_sLineToDraw = m_sLine; // .substr(m_iScrollIdx, m_uiNoOfColumns);
    } else
      m_sLineToDraw = m_sLine;
  }

  virtual void Scroll() {
    /*
    if (m_sLine.length() > m_uiNoOfColumns) {
      m_iScrollIdx++;
      if (m_iScrollIdx > (uint8_t)m_sLine.length())
        m_iScrollIdx = -m_uiNoOfColumns;
      std::string sEmptyLine(m_uiNoOfColumns, ' ');
      std::string sLineTmp = sEmptyLine + m_sLine + sEmptyLine;
      m_sLineToDraw =
          sLineTmp.substr(m_iScrollIdx + m_uiNoOfColumns, m_uiNoOfColumns);
    }
    */
  }
  uint8_t m_uiX;
  uint8_t m_uiY;
  uint8_t m_uiNoOfColumns;
  std::string m_sLineToDraw;
  std::string m_sLineDrawn;
  std::string m_sLine;
  uint8_t m_iScrollIdx;
  const uint8_t *m_pFont;
};

class CDisplayLineU8g2 : public CDisplayLine {
public:
  CDisplayLineU8g2(uint8_t x, uint8_t y, uint8_t uiNoOfColumns,
                   const uint8_t *pFont)
      : CDisplayLine(x, y, uiNoOfColumns, pFont) {}

  void Line(const std::string &sLineContent) override {
    m_sLine = sLineContent;
    m_sLineToDraw = m_sLine;
    m_iScrollIdx = 0;
    if (sLineContent.length() > m_uiNoOfColumns) {
      m_uiXOffset = m_nWidth;
      m_iScrollIdx = 0;
    }
  }
  void Scroll() override {
    if (m_sLine.length() <= m_uiNoOfColumns)
      return;
    if (m_uiXOffset > m_nCharWidth)
      m_uiXOffset -= m_nCharWidth;
    else if (m_iScrollIdx < m_sLine.length()) {
      m_uiXOffset = 0;
      m_sLineToDraw = m_sLine.substr(m_iScrollIdx);
      m_iScrollIdx++;
    } else {
      m_uiXOffset = m_uiNoOfColumns * m_nCharWidth;
      m_iScrollIdx = 0;
      m_sLineToDraw = m_sLine;
    }
  }

  int m_nCharWidth = 1;
  int m_nWidth = 128;
  uint8_t m_uiXOffset = 0;
};

#endif // SRC_CDISPLAYLINE_H_
