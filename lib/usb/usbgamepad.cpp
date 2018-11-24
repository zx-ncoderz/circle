//
// usbgamepad.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
//
// Ported from the USPi driver which is:
// 	Copyright (C) 2014  M. Maccaferri <macca@maccasoft.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <circle/usb/usbgamepad.h>
#include <circle/devicenameservice.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <circle/debug.h>
#include <assert.h>

unsigned CUSBGamePadDevice::s_nDeviceNumber = 1;

static const char FromUSBPad[] = "usbpad";
static const char DevicePrefix[] = "upad";

CUSBGamePadDevice::CUSBGamePadDevice (CUSBFunction *pFunction)
:	CUSBHIDDevice (pFunction),
	m_pStatusHandler (0),
	m_usReportSize (0),
	m_nDeviceNumber (0)	// not assigned
{
	memset (&m_State, 0, sizeof m_State);
}

CUSBGamePadDevice::~CUSBGamePadDevice (void)
{
	m_pStatusHandler = 0;

	CDeviceNameService::Get ()->RemoveDevice (DevicePrefix, m_nDeviceNumber, FALSE);
}

boolean CUSBGamePadDevice::Configure (void)
{
	assert (m_usReportSize != 0);
	if (!CUSBHIDDevice::Configure (m_usReportSize))
	{
		CLogger::Get ()->Write (FromUSBPad, LogError, "Cannot configure HID device");

		return FALSE;
	}

	m_nDeviceNumber = s_nDeviceNumber++;

	CDeviceNameService::Get ()->AddDevice (DevicePrefix, m_nDeviceNumber, this, FALSE);

	return TRUE;
}

const TGamePadState *CUSBGamePadDevice::GetInitialState (void)
{
	return &m_State;
}

void CUSBGamePadDevice::RegisterStatusHandler (TGamePadStatusHandler *pStatusHandler)
{
	assert (m_pStatusHandler == 0);
	m_pStatusHandler = pStatusHandler;
	assert (m_pStatusHandler != 0);
}

void CUSBGamePadDevice::ReportHandler (const u8 *pReport)
{
	if (   pReport != 0
	    && m_pStatusHandler != 0)
	{
		//debug_hexdump (pReport, m_usReportSize, FromUSBPad);

		DecodeReport (pReport);

		(*m_pStatusHandler) (m_nDeviceNumber-1, &m_State);
	}
}
