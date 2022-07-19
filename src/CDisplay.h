/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CDISPLAY_H_
#define SRC_CDISPLAY_H_
#include <SPI.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <algorithm>
#include <string>
#include <vector>

#include <CControl.h>
#include <CDisplayLine.h>
#include <CXbm.h>

class CDisplayBase : public CControl {
public:
  CDisplayBase(uint8_t nNoOfLines, uint8_t nNoOfColumns, const char *szInstance)
      : CControl(szInstance), m_Lines(), m_uiScrollTime(millis()),
        /*m_sEmptyLine(""), */ m_uiScrollDelay(250), m_uiDrawDelay(255) {
    // _log("CDisplayBase::CDisplayBase()");
    // m_sEmptyLine.resize(m_uiNoOfColumns, ' ');
  }
  virtual ~CDisplayBase() {
    for (uint8_t n = 0; n < m_Lines.size(); n++) {
      delete m_Lines[n];
    }
    m_Lines.clear();
  }

  bool setup() override = 0;

  CDisplayLine *AddLine(u8g2_uint_t x, u8g2_uint_t y, uint8_t uiNoOfColumns,
                        const uint8_t *pFont) {
    CDisplayLine *pLine = new CDisplayLine(x, y, uiNoOfColumns, pFont);
    m_Lines.push_back(pLine);
    return pLine;
  }

  CXbm *AddXbm(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) {
    CXbm *pXbm = new CXbm(x, y, w, h);
    m_Xbms.push_back(pXbm);
    this->_log(I, "AddXbm(%u, %u, %u, %u) s=%u, w/8=%u", x, y, w, h,
               pXbm->m_uiS, (uint8_t)(w / 8));
    return pXbm;
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
      updateDisplay();
      this->CControl::m_uiTime += m_uiDrawDelay;
      m_nState =
          (millis() > this->m_uiScrollTime) ? eScrollText : eWaitForDelay;
      break;

    case eScrollText:
      for (uint8_t n = 0; n < m_Lines.size(); n++) {
        CDisplayLine *pLine = m_Lines[n];
        pLine->Scroll();
        if (pLine->m_sLine.length() > pLine->m_uiNoOfColumns) {
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

  virtual void updateDisplay() = 0;

  void Line(uint8_t nIdx, std::string sLineContent) {
    if (nIdx >= m_Lines.size())
      return;
    while (sLineContent.length() < m_Lines.size())
      sLineContent += " ";
    m_Lines[nIdx]->Line(sLineContent);
    _log(I, "Line(%u, %s, %d, %s)", nIdx, sLineContent.c_str(),
         sLineContent.length(), m_Lines[nIdx]->m_sLineToDraw.c_str());
    this->m_uiScrollTime = millis() + m_uiScrollDelay;
  }

  CDisplayLine *GetLine(uint8_t n) {
    if (n < m_Lines.size())
      return m_Lines[n];
    return NULL;
  }
  CXbm *GetXbm(uint8_t n) {
    if (n < m_Xbms.size())
      return m_Xbms[n];
    return NULL;
  }
  size_t GetNoOfLines() { return m_Lines.size(); }

  virtual U8G2 *GetU8G2() { return NULL; }

protected:
  // uint8 m_uiNoOfLines;
  // uint8 m_uiNoOfColumns;
  std::vector<CDisplayLine *> m_Lines;
  std::vector<CXbm *> m_Xbms;
  uint64_t m_uiScrollTime;
  // std::string m_sEmptyLine;
  uint8_t m_uiScrollDelay;
  uint8_t m_uiDrawDelay;
};

template <typename T> class CDisplayU8x8 : public CDisplayBase {
public:
  CDisplayU8x8(int resetpin, uint8_t nNoOfLines, uint8_t nNoOfColumns)
      : CDisplayBase(nNoOfLines, nNoOfColumns, "CDisplay"),
        m_pDisplay(new T(resetpin)) {
    for (uint8_t n = 0; n < nNoOfLines; n++) {
      m_Lines.push_back(new CDisplayLine(0, n, nNoOfColumns, u8x8_font_5x8_r));
    }
    this->CControl::m_uiTime = millis() + 150;
  }
  bool setup() override {
    // _log(I, "CDisplayU8x8::setup(), %d lines", GetNoOfLines());
    m_pDisplay->begin();
    m_pDisplay->setPowerSave(0);

    m_pDisplay->clear();

    for (uint8_t n = 0; n < m_Lines.size(); n++) {
      String sLine = String("Line ") + String(n);
      Line(n, sLine.c_str());
      m_pDisplay->setFont(m_Lines[n]->m_pFont);
      m_pDisplay->drawUTF8(0, n, m_Lines[n]->m_sLineToDraw.c_str());
    }

    u8x8_cad_StartTransfer(m_pDisplay->getU8x8());
    u8x8_cad_SendCmd(m_pDisplay->getU8x8(), 0x0db);
    u8x8_cad_SendArg(m_pDisplay->getU8x8(), 0);
    u8x8_cad_EndTransfer(m_pDisplay->getU8x8());

    m_pDisplay->setContrast(0);

    return true;
  }
  void updateDisplay() override {
    CDisplayLine *pLine;
    for (uint8_t n = 0; n < m_Lines.size(); n++) {
      pLine = m_Lines[n];
      if (pLine->m_sLineToDraw != pLine->m_sLineDrawn) {
        m_pDisplay->setFont(pLine->m_pFont);
        m_pDisplay->drawUTF8(0, n, pLine->m_sLineToDraw.c_str());
        pLine->m_sLineDrawn = pLine->m_sLineToDraw;
        // _log("Line %u: %s", n, m_LinesDrawn[n].c_str());
      }
    }
  }

protected:
  U8X8 *m_pDisplay;
};
template <typename T> class CDisplayU8g2 : public CDisplayBase {
public:
  CDisplayU8g2(int resetpin, uint8_t nNoOfLines, uint8_t nNoOfColumns)
      : CDisplayBase(nNoOfLines, nNoOfColumns, "CDisplay"),
        m_pDisplay(new T(U8G2_R0, U8X8_PIN_NONE)) {
    this->CControl::m_uiTime = millis() + 150;
  }

  bool setup() override {
    // _log(I, "CDisplayU8g2::setup(), %d lines", GetNoOfLines());
    m_pDisplay->begin();
    m_pDisplay->setFont(u8g2_font_6x10_tf);
    m_pDisplay->setContrast(1);
    // m_pDisplay->setPowerSave(0);
    return true;
  }

  void updateDisplay() override {
    m_pDisplay->clearBuffer();
    CDisplayLine *pLine;
    for (uint8_t n = 0; n < m_Lines.size(); n++) {
      pLine = m_Lines[n];
      m_pDisplay->setFont(pLine->m_pFont);
      drawUTF8(pLine->m_uiX, pLine->m_uiY, pLine->m_sLineToDraw.c_str());
    }
    CXbm *pXbm;
    for (uint8_t n = 0; n < m_Xbms.size(); n++) {
      pXbm = m_Xbms[n];
      m_pDisplay->drawXBM(pXbm->m_uiX, pXbm->m_uiY, pXbm->m_uiW, pXbm->m_uiH,
                          pXbm->m_pBuffer);
    }
    m_pDisplay->sendBuffer();
  }

  u8g2_uint_t drawUTF8(u8g2_uint_t x, u8g2_uint_t y, const char *s) {
    return m_pDisplay->drawStr(x, y, s);
  }

  U8G2 *GetU8G2() { return m_pDisplay; }

protected:
  U8G2 *m_pDisplay;
};
#endif // SRC_CDISPLAY_H_
