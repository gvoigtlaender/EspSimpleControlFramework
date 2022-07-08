/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CDISPLAY_H_
#define SRC_CDISPLAY_H_
#include <SPI.h>
#include <U8x8lib.h>
#include <algorithm>
#include <string>
#include <vector>

#include <CControl.h>
#include <CDisplayLine.h>

class CDisplayBase : public CControl {
public:
  CDisplayBase(uint8 nNoOfLines, uint8 nNoOfColumns, const char *szInstance)
      : CControl(szInstance), m_pDisplay(NULL), m_uiNoOfLines(nNoOfLines),
        m_uiNoOfColumns(nNoOfColumns), m_Lines(), m_uiScrollTime(millis()),
        /*m_sEmptyLine(""), */ m_uiScrollDelay(250), m_uiDrawDelay(75) {
    // _log("CDisplayBase::CDisplayBase()");
    // m_sEmptyLine.resize(m_uiNoOfColumns, ' ');
    for (uint8 n = 0; n < m_uiNoOfLines; n++) {
      m_Lines.push_back(new CDisplayLine(nNoOfColumns /*, m_sEmptyLine*/));
    }
  }
  virtual ~CDisplayBase() {
    for (uint8 n = 0; n < m_uiNoOfLines; n++) {
      delete m_Lines[n];
    }
    m_Lines.clear();
  }

  bool setup() override {
    // _log("CDisplayBase::setup()");
    m_pDisplay->begin();
    m_pDisplay->setPowerSave(0);
    m_pDisplay->setFont(u8x8_font_chroma48medium8_r);
    m_pDisplay->setFont(u8x8_font_5x8_r);

    m_pDisplay->clear();

    for (uint8 n = 0; n < m_uiNoOfLines; n++) {
      String sLine = String("Line ") + String(n);
      Line(n, sLine.c_str());
      m_pDisplay->drawUTF8(0, n, m_Lines[n]->m_sLineToDraw.c_str());
    }

    u8x8_cad_StartTransfer(m_pDisplay->getU8x8());
    u8x8_cad_SendCmd(m_pDisplay->getU8x8(), 0x0db);
    u8x8_cad_SendArg(m_pDisplay->getU8x8(), 0);
    u8x8_cad_EndTransfer(m_pDisplay->getU8x8());

    m_pDisplay->setContrast(0);

    return true;
  }

  void control(bool bForce) override {
    enum {
      eStart = 0,
      eWaitForDelay,
      eUpdate,
      eScrollText,
    };
    switch (m_nState) {
    case eStart:
      m_nState = eWaitForDelay;

    case eWaitForDelay:
      if (millis() < this->CControl::m_uiTime)
        break;
      m_nState = eUpdate;

    case eUpdate:
      // _log("CDisplayBase::control::eUpdate");
      CDisplayLine *pLine;
      for (uint8 n = 0; n < m_uiNoOfLines; n++) {
        pLine = m_Lines[n];
        if (pLine->m_sLineToDraw != pLine->m_sLineDrawn) {
          m_pDisplay->drawUTF8(0, n, pLine->m_sLineToDraw.c_str());
          pLine->m_sLineDrawn = pLine->m_sLineToDraw;
          // _log("Line %u: %s", n, m_LinesDrawn[n].c_str());
        }
      }
      this->CControl::m_uiTime += m_uiDrawDelay;
      m_nState =
          (millis() > this->m_uiScrollTime) ? eScrollText : eWaitForDelay;
      break;

    case eScrollText:
      for (uint8 n = 0; n < m_uiNoOfLines; n++) {
        pLine = m_Lines[n];
        pLine->Scroll();
        if (pLine->m_sLine.length() > m_uiNoOfColumns) {
          pLine->Scroll();
          // _log("eScrollText Line %u, orig:{%s} Idx:%i Show:{%s} length:%d",
          // n, m_Lines[n].c_str(), m_iScrollIdx[n], m_LinesToDraw[n].c_str(),
          // m_LinesToDraw[n].length());
        }
      }
      this->m_uiScrollTime += m_uiScrollDelay;
      m_nState = eWaitForDelay;
      break;
    default:
      break;
    }
  }

  void Line(uint8 nIdx, std::string sLineContent) {
    while (sLineContent.length() < m_uiNoOfColumns)
      sLineContent += " ";
    m_Lines[nIdx]->Line(sLineContent);
    _log(I, "Line(%u, %s, %d, %s)", nIdx, sLineContent.c_str(),
         sLineContent.length(), m_Lines[nIdx]->m_sLineToDraw.c_str());
    this->m_uiScrollTime = millis() + m_uiScrollDelay;
  }

  CDisplayLine *GetLine(uint8 n) {
    if (n < m_Lines.size())
      return m_Lines[n];
    return NULL;
  }
  uint8 GetNoOfLines() { return m_uiNoOfLines; }

protected:
  U8X8 *m_pDisplay;

private:
  uint8 m_uiNoOfLines;
  uint8 m_uiNoOfColumns;
  std::vector<CDisplayLine *> m_Lines;
  uint64 m_uiScrollTime;
  // std::string m_sEmptyLine;
  uint8 m_uiScrollDelay;
  uint8 m_uiDrawDelay;
};

template <typename T> class CDisplay : public CDisplayBase {
public:
  CDisplay(int resetpin, uint8 nNoOfLines, uint8 nNoOfColumns)
      : CDisplayBase(nNoOfLines, nNoOfColumns, "CDisplay") {
    // delay(150);
    m_pDisplay = new T(resetpin);
    this->CControl::m_uiTime = millis() + 150;
  }
};
#endif // SRC_CDISPLAY_H_
