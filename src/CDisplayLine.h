#if !defined SRC_CDISPLAYLINE_H_
#define SRC_CDISPLAYLINE_H_

#include <string>

class CDisplayLine {
public:
  CDisplayLine(uint8 uiNoOfColumns /*, const std::string &sEmptyLine*/)
      : m_uiNoOfColumns(uiNoOfColumns), m_sLineToDraw(""), m_sLineDrawn(""),
        m_sLine(""), m_iScrollIdx(0) /*, m_sEmptyLine(sEmptyLine)*/ {}

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

  uint8 m_uiNoOfColumns;
  std::string m_sLineToDraw;
  std::string m_sLineDrawn;
  std::string m_sLine;
  int8 m_iScrollIdx;
  // std::string m_sEmptyLine;
};

#endif // SRC_CDISPLAYLINE_H_