/* Copyright 2019 Georg Voigtlaender gvoigtlaender@googlemail.com */
#include "CControl.h"

// static
std::vector<CControl *> CControl::ms_Instances;
// static
int CControl::ms_ulValuesPending = 0;
// static
int CControl::ms_ulProcessPending = 0;
// static
bool CControl::ms_bNetworkConnected = false;
// static
bool CControl::ms_bUsbChargingActive = false;

// static
Syslog *CControl::ms_pSyslog = NULL;
