/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CDisplay.h"
#include "CDisplayLine.h"
#include "CXbm.h"

CDisplayLine *CDisplayBase::AddLine(u8g2_uint_t x, u8g2_uint_t y,
                                    uint8_t uiNoOfColumns,
                                    const uint8_t *pFont) {
  auto *pLine = new CDisplayLine(x, y, uiNoOfColumns, pFont);
  m_Lines.push_back(pLine);
  return pLine;
}

CXbm *CDisplayBase::AddXbm(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w,
                           u8g2_uint_t h, uint16_t uiUpdateIntervalS) {
  CXbm *pXbm = new CXbm(x, y, w, h, uiUpdateIntervalS);
  m_Xbms.push_back(pXbm);
  this->_log(I, "AddXbm(%u, %u, %u, %u) s=%u, w/8=%u", x, y, w, h, pXbm->m_uiS,
             (uint8_t)(w / 8));
  return pXbm;
}

void CDisplayBase::Line(uint8_t nIdx, const std::string &sLineContent) {
  if (nIdx >= m_Lines.size()) {
    return;
  }
  CDisplayLine *pLine = m_Lines[nIdx];
  _log(I, "Line(%u, %s, %d, %s)", nIdx, sLineContent.c_str(),
       sLineContent.length(), pLine->m_sLineToDraw.c_str());
  pLine->Line(sLineContent);
  this->m_uiScrollTime = millis() + m_uiScrollDelay;
}

void CDisplayU8g2Base::drawXBM(const CXbm *pXbm) {
  drawXBM(pXbm->m_uiX, pXbm->m_uiY, pXbm->m_uiW, pXbm->m_uiH, pXbm->m_pBuffer);
}