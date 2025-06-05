/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CDISPLAY_H
#define SRC_CDISPLAY_H
#include <SPI.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <algorithm>
#include <string>
#include <vector>

#include "CControl.h"
#include "CTimer.hpp"

class CDisplayLine;
class CXbm;

class CDisplayBase : public CControl {
public:
  CDisplayBase(const char *szInstance)
      : CControl(szInstance), m_Lines(), m_uiScrollTime(millis()),
        /*m_sEmptyLine(""), */ m_uiScrollDelay(200), m_uiDrawDelay(100) {
    // _log("CDisplayBase::CDisplayBase()");
    // m_sEmptyLine.resize(m_uiNoOfColumns, ' ');
  }
  virtual ~CDisplayBase() override {
    for (auto &&line : m_Lines) {
      delete line;
    }
    m_Lines.clear();
  }

  bool setup() override = 0;

  virtual CDisplayLine *AddLine(u8g2_uint_t x, u8g2_uint_t y,
                                uint8_t uiNoOfColumns, const uint8_t *pFont);
  virtual CHoverLine *AddHoverLine(std::function<std::string()> textFunc,
                                   const uint8_t *font) {
    return nullptr;
  };

  CXbm *AddXbm(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
               uint16_t uiUpdateIntervalS = 0);

  enum PowerSafeMode { disabled, blank, hover };

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
      if (powerSafeMode_ != PowerSafeMode::disabled) {
        if (!powerSafeActive_ && millis() > powerSafeTimer_.time_) {
          powerSafeActive_ = true;
          _log(I, "CDisplayU8x8::SafeActive, Mode=%d", powerSafeMode_);
          if (powerSafeMode_ == PowerSafeMode::blank) {
            setPowerSafe(true);
          }
        }
      }
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
      for (auto &&pLine : m_Lines) {
        pLine->Scroll();
        /*
        if (pLine->m_sLine.length() > pLine->m_uiNoOfColumns) {
          pLine->Scroll();
          // _log("eScrollText Line %u, orig:{%s} Idx:%i Show:{%s} length:%d",
          // n, m_Lines[n].c_str(), m_iScrollIdx[n], m_LinesToDraw[n].c_str(),
          // m_LinesToDraw[n].length());
        }
        */
      }
      this->m_uiScrollTime += m_uiScrollDelay;
      m_nState = eWaitForDelay;
      break;
    default:
      break;
    }
  }

  virtual void updateDisplay() = 0;

  void Line(uint8_t nIdx, const std::string &sLineContent);

  CDisplayLine *GetLine(uint8_t n) {
    if (n < m_Lines.size())
      return m_Lines[n];
    return nullptr;
  }
  CXbm *GetXbm(uint8_t n) {
    if (n < m_Xbms.size())
      return m_Xbms[n];
    return nullptr;
  }
  size_t GetNoOfLines() { return m_Lines.size(); }

  virtual U8G2 *GetU8G2() { return nullptr; }

  void enablePowerSafe(PowerSafeMode mode, uint64_t idleTimeMs, uint64_t hoverTimeMs) {
    powerSafeMode_ = mode;
    powerSafeTimer_.period_ = idleTimeMs;
    powerSafeTimer_.time_ = millis() + idleTimeMs;
    hoverTimer_.time_ = powerSafeTimer_.time_;
    hoverTimer_.period_ = hoverTimeMs;
    if (mode == PowerSafeMode::disabled && powerSafeActive_) {
      setPowerSafe(0);
      _log(I, "CDisplayU8x8::setPowerSafe(false)");
      powerSafeActive_ = false;
    }
  }
  bool isPowerSafeActive() const { return powerSafeActive_; }

  void powerSafeWakeup() {
    if (powerSafeActive_) {
      powerSafeActive_ = false;
      powerSafeTimer_.time_ = millis() + powerSafeTimer_.period_;
      setPowerSafe(false);
      updateDisplay();
      _log(I, "CDisplayU8x8::setPowerSafe(false)");
    }
  }

protected:
  // uint8 m_uiNoOfLines;
  // uint8 m_uiNoOfColumns;
  std::vector<CDisplayLine *> m_Lines;
  std::vector<CXbm *> m_Xbms;
  uint64_t m_uiScrollTime;
  // std::string m_sEmptyLine;
  uint8_t m_uiScrollDelay;
  uint8_t m_uiDrawDelay;

  PowerSafeMode powerSafeMode_{PowerSafeMode::disabled};
  bool powerSafeActive_{false};
  CTimer powerSafeTimer_;
  CTimer hoverTimer_;
  CHoverLine *powerSafeHoverLine_{nullptr};
  uint64_t hoverTime_{0};

  virtual void setPowerSafe(bool enable) = 0;
};

template <typename T> class CDisplayU8x8 : public CDisplayBase {
public:
  CDisplayU8x8(int resetpin)
      : CDisplayBase("CDisplay"), m_pDisplay(new T(resetpin)) {
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

  void setPowerSafe(bool enable) override {
    m_pDisplay->setPowerSave(enable ? 1 : 0);
  }
};

class CDisplayU8g2Base : public CDisplayBase {
public:
  bool setup() override {
    // _log(I, "CDisplayU8g2::setup(), %d lines", GetNoOfLines());
    m_pDisplay->begin();
    m_pDisplay->setFont(u8g2_font_6x10_tf);
    m_pDisplay->setContrast(1);
    // m_pDisplay->setPowerSave(0);
    return true;
  }

  CDisplayLine *AddLine(u8g2_uint_t x, u8g2_uint_t y, uint8_t uiNoOfColumns,
                        const uint8_t *pFont) override {
    CDisplayLineU8g2 *pLine = new CDisplayLineU8g2(x, y, uiNoOfColumns, pFont);
    m_pDisplay->setFont(pFont);
    pLine->m_nCharWidth = m_pDisplay->getUTF8Width("X");
    pLine->m_nWidth = m_pDisplay->getWidth();
    pLine->m_uiNoOfColumns = pLine->m_nWidth / pLine->m_nCharWidth;
    // _log(D, "AddLine(%u, %u, %u->%u", x, y, uiNoOfColumns,
    // pLine->m_uiNoOfColumns);
    m_Lines.push_back(pLine);
    return pLine;
  }
  CHoverLine *AddHoverLine(std::function<std::string()> textFunc,
                           const uint8_t *font) override {
    CHoverLine *pLine = new CHoverLine(textFunc, font);
    powerSafeHoverLine_ = pLine;
    return pLine;
  }

protected:
  CDisplayU8g2Base(int resetpin, U8G2 *pDisplay)
      : CDisplayBase("CDisplay"), m_pDisplay(pDisplay) {
    this->CControl::m_uiTime = millis() + 150;
  }

  void updateDisplay() override {
    if (!powerSafeActive_) {
      m_pDisplay->clearBuffer();
      for (uint8_t n = 0; n < m_Lines.size(); n++) {
        CDisplayLineU8g2 *pLine = static_cast<CDisplayLineU8g2 *>(m_Lines[n]);
        m_pDisplay->setFont(pLine->m_pFont);
        drawUTF8(pLine->m_uiX + pLine->m_uiXOffset, pLine->m_uiY,
                 pLine->m_sLineToDraw.c_str());
      }
      for (const auto *pXbm : m_Xbms) {
        drawXBM(pXbm);
      }
      m_pDisplay->sendBuffer();
    } else if (powerSafeMode_ == PowerSafeMode::hover &&
               powerSafeHoverLine_ != nullptr) {
      if (millis() > hoverTimer_.time_) {
        hoverTimer_.time_ += hoverTimer_.period_;
        m_pDisplay->setFont(powerSafeHoverLine_->m_pFont);
        auto text = powerSafeHoverLine_->getText();
        const uint8_t margin = 2;
        int16_t text_width = m_pDisplay->getUTF8Width(text.c_str());
        int16_t text_height =
            m_pDisplay->getAscent() - m_pDisplay->getDescent();

        int16_t max_x = m_pDisplay->getDisplayWidth() - text_width - margin;
        int16_t max_y =
            m_pDisplay->getDisplayHeight() + m_pDisplay->getDescent() - margin;

        int16_t min_x = margin;
        int16_t min_y = text_height + margin;

        int16_t x = random(min_x, max_x + 1);
        int16_t y = random(min_y, max_y + 1);

        // Anzeige zeichnen
        m_pDisplay->clearBuffer();
        m_pDisplay->drawUTF8(x, y, text.c_str());
        m_pDisplay->sendBuffer();
      }
    }
  }

  u8g2_uint_t drawUTF8(u8g2_uint_t x, u8g2_uint_t y, const char *s) {
    // _log(I, "drawUTF8(%u, %u, [%s])", x, y, s);
    return m_pDisplay->drawStr(x, y, s);
  }
  void drawXBM(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h,
               const uint8_t *bitmap) {
    // _log(I, "drawXBM(%u, %u, %u, %u)", x, y, w, h);
    m_pDisplay->drawXBM(x, y, w, h, bitmap);
  }

  U8G2 *GetU8G2() override { return m_pDisplay; }
  void drawXBM(const CXbm *pXbm);
  U8G2 *m_pDisplay;
};

template <typename T> class CDisplayU8g2 : public CDisplayU8g2Base {
public:
  CDisplayU8g2(int resetpin)
      : CDisplayU8g2Base(resetpin, new T(U8G2_R0, U8X8_PIN_NONE)) {}

protected:
  void setPowerSafe(bool enable) override {
    m_pDisplay->setPowerSave(enable ? 1 : 0);
  }
};
#endif // SRC_CDISPLAY_H
