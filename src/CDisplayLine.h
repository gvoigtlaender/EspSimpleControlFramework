#if !defined SRC_CDISPLAYLINE_H_
#define SRC_CDISPLAYLINE_H_

#include <string>

class CDisplayLine {
public:
  CDisplayLine(uint8_t x, uint8_t y, uint8 uiNoOfColumns, const uint8_t *pFont)
      : m_uiX(x), m_uiY(y), m_uiNoOfColumns(uiNoOfColumns), m_sLineToDraw(""),
        m_sLineDrawn(""), m_sLine(""), m_iScrollIdx(0), m_pFont(pFont) {}

  void Line(std::string sLineContent) {
    while (sLineContent.length() < m_uiNoOfColumns)
      sLineContent += " ";
    m_sLine = sLineContent;
    if (sLineContent.length() <= m_uiNoOfColumns) {
      m_iScrollIdx = 0;
      m_sLineToDraw = m_sLine; // .substr(m_iScrollIdx, m_uiNoOfColumns);
    }
  }

  void Scroll() {
    if (m_sLine.length() > m_uiNoOfColumns) {
      m_iScrollIdx++;
      if (m_iScrollIdx > (int8)m_sLine.length())
        m_iScrollIdx = -m_uiNoOfColumns;
      std::string sEmptyLine(m_uiNoOfColumns, ' ');
      std::string sLineTmp = sEmptyLine + m_sLine + sEmptyLine;
      m_sLineToDraw =
          sLineTmp.substr(m_iScrollIdx + m_uiNoOfColumns, m_uiNoOfColumns);
    }
  }
  uint8_t m_uiX;
  uint8_t m_uiY;
  uint8 m_uiNoOfColumns;
  std::string m_sLineToDraw;
  std::string m_sLineDrawn;
  std::string m_sLine;
  int8 m_iScrollIdx;
  const uint8_t *m_pFont;
};

#endif // SRC_CDISPLAYLINE_H_
