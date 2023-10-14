/* Copyright 2018 Georg Voigtlaender gvoigtlaender@googlemail.com */
#ifndef SRC_CFILTER_HPP
#define SRC_CFILTER_HPP
#include <vector>

template <typename T> class CFilter {
public:
  explicit CFilter(unsigned int nMaxSize)
      : m_nSizeMax(nMaxSize), m_OutputValue(), m_Sum(), m_Values() {}
  T Filter(T Value) {
    if (m_Values.empty()) {
      m_Sum = Value;
    } else {
      m_Sum += Value;
    }

    m_Values.push_back(Value);
    if (m_Values.size() > m_nSizeMax) {
      m_Sum -= m_Values[0];
      m_Values.erase(m_Values.begin());
    }
    m_nSize = m_Values.size();
    m_OutputValue = m_Sum / m_nSize;
    return m_OutputValue;
  }

  T getOuptuValue() { return m_OutputValue; }

private:
  const unsigned int m_nSizeMax = 0;
  unsigned int m_nSize = 0;
  T m_OutputValue;
  T m_Sum;
  std::vector<T> m_Values;
};
#endif // SRC_CFILTER_HPP
