head	1.17;
access;
symbols;
locks; strict;
comment	@// @;


1.17
date	2008.10.19.16.36.36;	author movieman523;	state Exp;
branches;
next	1.16;

1.16
date	2008.10.18.07.01.29;	author movieman523;	state Exp;
branches;
next	1.15;

1.15
date	2008.10.17.07.16.03;	author movieman523;	state Exp;
branches;
next	1.14;

1.14
date	2008.10.17.06.18.05;	author movieman523;	state Exp;
branches;
next	1.13;

1.13
date	2008.10.17.04.00.47;	author movieman523;	state Exp;
branches;
next	1.12;

1.12
date	2008.10.17.03.14.21;	author movieman523;	state Exp;
branches;
next	1.11;

1.11
date	2008.10.17.02.42.30;	author movieman523;	state Exp;
branches;
next	1.10;

1.10
date	2008.10.17.01.52.19;	author movieman523;	state Exp;
branches;
next	1.9;

1.9
date	2008.10.17.01.28.59;	author movieman523;	state Exp;
branches;
next	1.8;

1.8
date	2008.10.17.00.21.32;	author movieman523;	state Exp;
branches;
next	1.7;

1.7
date	2008.10.16.07.11.31;	author movieman523;	state Exp;
branches;
next	1.6;

1.6
date	2008.10.16.06.27.32;	author movieman523;	state Exp;
branches;
next	1.5;

1.5
date	2008.10.16.05.48.33;	author movieman523;	state Exp;
branches;
next	1.4;

1.4
date	2008.10.16.05.12.21;	author movieman523;	state Exp;
branches;
next	1.3;

1.3
date	2008.10.15.06.44.18;	author movieman523;	state Exp;
branches;
next	1.2;

1.2
date	2008.10.15.03.36.16;	author movieman523;	state Exp;
branches;
next	1.1;

1.1
date	2007.01.02.01.54.43;	author dseagrav;	state Exp;
branches;
next	;


desc
@@


1.17
log
@More telemetry.
@
text
@/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2008

  Telemetry Client main window

  Project Apollo is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Project Apollo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Project Apollo; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  See http://nassp.sourceforge.net/license/ for more details.

  **************************** Revision History ****************************
  *	$Log: Form1.cpp,v $
  *	Revision 1.16  2008/10/18 07:01:29  movieman523
  *	More telemetry and rationalisation.
  *	
  *	Revision 1.15  2008/10/17 07:16:03  movieman523
  *	More telemetry.
  *	
  *	Revision 1.14  2008/10/17 06:18:05  movieman523
  *	Moved all decoded data to new display() method.
  *	
  *	Revision 1.13  2008/10/17 04:00:47  movieman523
  *	More LBR support.
  *	
  *	Revision 1.12  2008/10/17 03:14:21  movieman523
  *	More low bit-rate decoding.
  *	
  **************************************************************************/

#include "stdafx.h"
#include "winsock2.h"
#include "Form1.h"
#include "StructuresForm.h"
#include "EPSForm.h"
#include "ELSForm.h"
#include "ECSForm.h"
#include "GNCForm.h"
#include "SCSForm.h"
#include "SPSForm.h"
#include "TelecomForm.h"
#include "CrewForm.h"
#include "CMCForm.h"
#include "UplinkForm.h"
#include <windows.h>
#include <stdio.h>

using namespace GroundStation;

// DS20070108 Telemetry measurement types
#define TLM_A	1
#define TLM_DP	2
#define TLM_DS	3
#define TLM_E	4
#define TLM_SRC 5

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	System::Threading::Thread::CurrentThread->ApartmentState = System::Threading::ApartmentState::STA;
	Application::Run(new Form1());
	return 0;
}

void Form1::WinsockInit(){
	// Do this here too
	// eps_form = NULL;

	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR ){
		StatusBox->Text = "Error at WSAStartup()";
		return;
	}else{
		StatusBox->Text = "Disconnected, Winsock Ready";
	}
};

void Form1::ConnectToHost(){
	char msg[256];
	char addr[256];

	if(conn_status == 0){
		sprintf(msg,"Connecting...");
		StatusBox->Text = msg;
		m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( m_socket == INVALID_SOCKET ) {
			sprintf(msg,"Error at socket(): %ld", WSAGetLastError() );
			StatusBox->Text = msg;
			closesocket(m_socket);
			return;
		}
		// Address to ANSI string
		sprintf(addr,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(HostAddrBox->Text));

		// Now connect
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr(addr);
		clientService.sin_port = htons( 14242 );

		if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
			sprintf(msg,"Failed to connect, Error %ld",WSAGetLastError());
			closesocket(m_socket);
			StatusBox->Text = msg;
			return;
		}else{
			StatusBox->Text = "LOCK/OUT FRAM/000 WORD/000";
			// Kick service thread
			ThreadStart *myThreadDelegate = new ThreadStart(this, &GroundStation::Form1::ThreadTask);
			trd = new Thread(myThreadDelegate);
			trd->IsBackground = true;
			trd->Start();
			ConnectBtn->Text = "Disconnect";
			conn_status = 1;
		}
	}else{
		sprintf(msg,"Disconnecting...");
		StatusBox->Text = msg;
		trd->Abort(); // Kill other thread
		conn_status = 0;
		shutdown(m_socket,SD_BOTH);
		closesocket(m_socket);
		ConnectBtn->Text = "Connect";
	}
};

// Control disabling
void Form1::end_lbr()
{
	// We should disable all the data displays here.
}

void Form1::end_hbr()
{
	// We should disable all the data displays here.
}

// Unscale PCM data to original value
double Form1::unscale_data( unsigned char data, double low, double high )
{
	if ( data == 0 )
	{    
		return low;
	}

	if ( data == 0xFF )
	{
		return high;
	}

	double step = ( ( high - low ) / 256.0);
	return ( data * step ) + low;
}

void Form1::showValue( textDisplay *tb, char *msg )
{
	tb->Text = msg;
	tb->Enabled = TRUE;
	tb->ReadOnly = TRUE;
}

void Form1::showPercentage( textDisplay *tb, unsigned char data, double maxPercent )
{
	char msg[64];
	sprintf(msg, "%05.2f %%", unscale_data(data, 0, maxPercent));
	showValue( tb, msg );				
}

void Form1::showPercentage( textDisplay *tb, unsigned char data )
{
	showPercentage( tb, data, 100.0 );
}

void Form1::showSci( textDisplay *tb, unsigned char data )
{
	char msg[64];
	sprintf(msg, "%05.1f", unscale_data(data, 0, 100));
	showValue( tb, msg );
}

void Form1::showPSIA( textDisplay *tb, unsigned char data, double low, double high )
{
	char msg[64];
	char *sFormat = "%05.3f PSIA";
	if ( high > 1000 )
	{
		sFormat = "%04.0f PSIA";
	}
	else if ( high > 100.0 )
	{
		sFormat = "%05.1f PSIA";
	}
	else if ( high > 10.0 )
	{
		sFormat = "%05.2f PSIA";
	}

	sprintf(msg, sFormat, unscale_data(data, low, high));
	showValue( tb, msg );
}

void Form1::showTempF( textDisplay *tb, unsigned char data, double low, double high )
{
	char msg[64];
	char *sFormat;

	// If low < 0 we need the sign. If low > 0 then high must also be > 0.
	if ( low < 0 )
	{
		sFormat = "%+07.2f �F";
	}
	else
	{
		sFormat = "%06.2f �F";
	}

	sprintf(msg, sFormat, unscale_data(data, low, high));
	showValue( tb, msg );
}

void Form1::display(unsigned char data, int channel, int type, int ccode)
{
	char msg[256];
	double value;

	switch(type){
	case TLM_A:  // ANALOG
		switch(channel){
		case 10: // S10A
			switch(ccode)
			{
			case 3: // CO2 PARTIAL PRESS
				if ( ecs_form != NULL )
				{
					value = unscale_data(data, 0.0, 30.0);
					sprintf(msg,"%04.1f MM",value);
					showValue( ecs_form->s10A3, msg );				
				}
				break;

			case 4:	// GLY EVAP BACK PRESS
				if ( ecs_form != NULL )
				{
					showPSIA( ecs_form->s10A4, data,  0.05, 0.25 );			
				}
				break;

			case 6:	// CABIN PRESS
				if ( ecs_form != NULL )
				{
					showPSIA( ecs_form->s10A6, data,  0.0, 17.0 );					
				}
				break;

			case 8:	// SEC EVAP OUT STEAM PRESS
				if ( ecs_form != NULL )
				{
					showPSIA( ecs_form->s10A8, data,  0.5, 0.25 );			
				}
				break;

			case 9:	// WASTE H20 QTY
				if ( ecs_form != NULL )
				{
					showPercentage( ecs_form->s10A9, data );				
				}
				break;

			case 10: // SPS VLV ACT PRESS PRI
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A10, data,  0.0, 5000.0 );							
				}
				break;

			case 11: // SPS VLV ACT PRESS SEC
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A11, data,  0.0, 5000.0 );							
				}
				break;

			case 12: // GLY EVAP OUT TEMP
				if ( ecs_form != NULL )
				{
					showTempF( ecs_form->s10A12, data,  25, 75 );
				}
				break;

			case 14: // ENG CHAMBER PRESS
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A14, data,  0.0, 150.0 );							
				}
				break;

			case 15: // ECS RAD OUT TEMP
				if ( ecs_form != NULL )
				{
					showTempF( ecs_form->s10A15, data,  -50, 100 );
				}
				break;

			case 16: // HE TK TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A16, data,  -100, 200 );
				}
				break;

			case 17: // SM ENG PKG B TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A17, data,  0, 300 );							
				}
				break;

			case 18: // CM HE TK A PRESS
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A18, data,  0.0, 5000.0 );							
				}
				break;

			case 19: // SM ENG PKG C TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A19, data,  0, 300 );							
				}
				break;

			case 20: // SM ENG PKG D TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A20, data,  0, 300 );							
				}
				break;

			case 21: // CM HE TK B PRESS
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A21, data,  0.0, 5000.0 );							
				}
				break;

			case 22: // DOCKING PROBE TEMP
				if ( els_form != NULL )
				{
					showTempF( els_form->s10A22, data,  -100, 300 );			
				}
				break;

			case 24: // SM HE TK A PRESS
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A24, data,  0.0, 5000.0 );							
				}
				break;

			case 26: // OX TK 1 QTY -TOTAL AUX
				if ( sps_form )
				{
					showPercentage( sps_form->s10A26, data, 50.0 );
				}
				break;

			case 27: // SM HE TK B PRESS
				if ( sps_form != NULL )
				{
					showPSIA( sps_form->s10A27, data,  0.0, 5000.0 );
				}
				break;

			case 28: // OX TK 2 QTY
				if ( sps_form != NULL )
				{
					showPercentage( sps_form->s10A28, data, 60.0 );
				}
				break;

			case 29: // FU TK 1 QTY -TOTAL AUX
				if ( sps_form )
				{
					showPercentage( sps_form->s10A29, data, 50.0 );
				}
				break;

			case 30: // SM HE TK C PRESS
				if ( sps_form )
				{
					showPSIA( sps_form->s10A30, data,  0.0, 5000.0 );
				}
				break;

			case 31: // FU TK 2 QTY
				if ( sps_form != NULL )
				{
					showPercentage( sps_form->s10A28, data, 60.0 );
				}
				break;

			case 33: // SM HE TK D PRESS
				if ( sps_form )
				{
					showPSIA( sps_form->s10A33, data,  0.0, 5000.0 );
				}
				break;

			case 36: // H2 TK 1 PRESS
				if ( eps_form )
				{
					showPSIA( eps_form->s10A36, data,  0.0, 350.0 );
				}
				break;

			case 37: // SPS VLV BODY TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A37, data,  0, 200 );							
				}
				break;

			case 39: // H2 TK 2 PRESS
				if ( eps_form )
				{
					showPSIA( eps_form->s10A39, data,  0.0, 350.0 );
				}
				break;

			case 42: // O2 TK 2 QTY
				if ( eps_form != NULL )
				{
					showPercentage( eps_form->s10A42, data );
				}
				break;

			case 44: // OX LINE 1 TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A44, data,  0, 200 );							
				}
				break;

			case 45: // SUIT AIR HX OUT TEMP
				if ( ecs_form )
				{
					showTempF( ecs_form->s10A45, data,  20, 95 );
				}
				break;

			case 47: // SPS INJECTOR FLANGE TEMP 1
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A47, data,  0, 600 );							
				}
				break;

			case 48: // PRI RAD IN TEMP
				if ( ecs_form != NULL )
				{
					showTempF( ecs_form->s10A48, data,  55, 120 );							
				}
				break;

			case 49: // SPS INJECTOR FLANGE TEMP 2
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A49, data,  0, 600 );							
				}
				break;

			case 51: // FC 1 COND EXH TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A51, data,  145, 250 );							
				}
				break;

			case 54: // O2 TK 1 TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A54, data,  -325, 80 );							
				}
				break;

			case 57: // O2 TK 2 TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A57, data,  -325, 80 );							
				}
				break;

			case 59: // FU LINE 1 TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A59, data,  0, 200 );							
				}
				break;

			case 61: // NUCLEAR PARTICLE DETECTOR TEMP
				if ( tcm_form != NULL )
				{
					showTempF( tcm_form->s10A61, data,  -109, 140 );							
				}
				break;

			case 62: // NUCLEAR PARTICLE ANALYZER TEMP
				if ( tcm_form != NULL )
				{
					showTempF( tcm_form->s10A62, data,  -109, 140 );							
				}
				break;

			case 63: // H2 TK 2 TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A63, data,  -425, 200 );							
				}
				break;

			case 65: // SIDE HS BOND LOC 1 TEMP
				if ( str_form != NULL )
				{
					showTempF( str_form->s10A65, data,  -260, 600 );			
				}
				break;

			case 66: // O2 TK 2 PRESS
				if ( eps_form != NULL )
				{
					showPSIA( eps_form->s10A66, data,  50, 1050 );							
				}
				break;

			case 67: // FC 3 RAD IN TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A67, data,  -50, 300 );		
				}
				break;

			case 69: // FC 3 COND EXH TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A69, data,  145, 250 );							
				}
				break;

			case 70: // SIDE HS BOND LOC 2 TEMP
				if ( str_form != NULL )
				{
					showTempF( str_form->s10A70, data,  -260, 600 );			
				}
				break;

			case 72: // FC 1 SKIN TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A72, data,  80, 550 );							
				}
				break;

			case 74: // SIDE HS BOND LOC 3 TEMP
				if ( str_form != NULL )
				{
					showTempF( str_form->s10A74, data,  -260, 600 );			
				}
				break;

			case 75: // FC 2 SKIN TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A75, data,  80, 550 );							
				}
				break;

			case 78: // FC 3 SKIN TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A78, data,  80, 550 );							
				}
				break;

			case 79: // SIDE HS BOND LOC 4 TEMP
				if ( str_form != NULL )
				{
					showTempF( str_form->s10A79, data,  -260, 600 );			
				}
				break;

			case 81: // POTABLE H20 QTY
				if ( ecs_form != NULL )
				{
					showPercentage( ecs_form->s10A81, data );							
				}
				break;

			case 83: // PIPA +120 VDC

			case 84: // CABIN TEMP
				if ( ecs_form != NULL )
				{
					showTempF( ecs_form->s10A84, data,  40, 125 );			
				}
				break;

			case 85: // 3.2 KHz 28V SUPPLY
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, 0, 31.1);
					sprintf(msg,"%06.2f V",value);
					showValue( gnc_form->s10A85, msg );						
				}
				break;

			case 88: // INVERTER 2 TEMP
				if ( eps_form != NULL )
				{
					showTempF( eps_form->s10A88, data,  32, 248 );			
				}
				break;

			case 91: // IMU 28 VAC 800Hz
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, 0, 31.1);
					sprintf(msg,"%06.2f V",value);
					showValue( gnc_form->s10A91, msg );						
				}
				break;

			case 100: // PRI EVAP INLET TEMP
				if ( ecs_form != NULL )
				{
					showTempF( ecs_form->s10A100, data,  35, 100 );			
				}
				break;

			case 106: // SCI EXP #4
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A106, data );					
				}
				break;

			case 107: // SCI EXP #5
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A107, data );						
				}
				break;

			case 108: // SCI EXP #1
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A108, data );						
				}
				break;

			case 109: // SCI EXP #6
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A109, data );						
				}
				break;

			case 110: // SCI EXP #7
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A110, data );						
				}
				break;

			case 111: // SCI EXP #2
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A111, data );						
				}
				break;

			case 112: // SCI EXP #8
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A112, data );						
				}
				break;

			case 113: // SCI EXP #9
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A113, data );					
				}
				break;

			case 115: // SCI EXP #10
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A115, data );						
				}
				break;

			case 116: // SCI EXP #11
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A116, data );					
				}
				break;

			case 118: // SCI EXP #12
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A118, data );						
				}
				break;

			case 119: // SCI EXP #13
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A119, data );					
				}
				break;

			case 121: // SCI EXP #14
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A121, data );					
				}
				break;

			case 122: // SCI EXP #15
				if ( crw_form != NULL )
				{
					showSci( crw_form->s10A122, data );						
				}
				break;

			case 136: // SM ENG PKG A TEMP
				if ( sps_form != NULL )
				{
					showTempF( sps_form->s10A136, data,  0, 300 );							
				}
				break;
			}
			break;

		case 11: // S11A
			switch ( ccode )
			{
			case 1: // 11A1 SUIT MANF ABS PRESS
				if ( ecs_form != NULL )
				{
					showPSIA( ecs_form->s11A1, data, 0, 17);				
				}
				break;

			case 2: // 11A2 SUIT COMP DELTA P
				if( ecs_form != NULL )
				{
					value = unscale_data(data, 0, 1 );
					sprintf(msg,"%+03.2f PSID",value);
					showValue( ecs_form->s11A2, msg );						
				}
				break;

			case 3: // 11A3 GLY PUMP OUT PRESS
				if ( ecs_form != NULL )
				{
					value = unscale_data(data,0,60);
					sprintf(msg,"%04.1f PSIG",value);
					showValue( ecs_form->s11A3, msg );
				}
				break;

			case 4: // 11A4 ECS SURGE TANK PRESS
				if ( ecs_form != NULL )
				{
					value = unscale_data(data, 50, 1050);
					sprintf(msg,"%04.0f PSIG",value);
					showValue( ecs_form->s11A4, msg );						
				}
				break;

			case 5: // 11A5 PYRO BUS B VOLTS
				if ( els_form != NULL )
				{
					value = unscale_data(data, 0, 40);
					sprintf(msg,"%04.2f V",value);
					showValue( els_form->s11A5, msg );						
				}
				break;

			case 6: // 11A6 LES LOGIC BUS B VOLTS
				if ( els_form != NULL )
				{
					value = unscale_data(data, 0, 40);
					sprintf(msg,"%04.2f V",value);
					showValue( els_form->s11A6, msg );					
				}
				break;

			case 8: // 11A8 LES LOGIC BUS A VOLTS
				if ( els_form != NULL )
				{
					value = unscale_data(data, 0, 40);
					sprintf(msg,"%04.2f V",value);
					showValue( els_form->s11A8, msg );						
				}
				break;

			case 9: // 11A9 PYRO BUS A VOLTS
				if (els_form != NULL)
				{
					value = unscale_data(data, 0, 40);
					sprintf(msg,"%04.2f V",value);
					showValue( els_form->s11A9, msg );						
				}
				break;

			case 10: // 11A10 SPS HE TK PRESS
				if ( sps_form != NULL )
				{
					value = unscale_data(data, 0, 5000);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A10, msg );								
				}
				break;

			case 11: // 11A11 SPS OX TK PRESS
				if ( sps_form != NULL )
				{
					value = unscale_data(data, 0, 250);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A11, msg );								
				}
				break;

			case 12: // 11A12 SPS FU TK PRESS
				if ( sps_form != NULL )
				{
					value = unscale_data(data, 0, 250);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A12, msg );								
				}
				break;

			case 13: // 11A13 GLY ACCUM QTY
				if ( ecs_form != NULL )
				{
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.2f %",value);
					showValue( ecs_form->s11A13, msg );						
				}
				break;

			case 14: // 11A14 ECS O2 FLOW O2 SUPPLY MANF
				if ( ecs_form != NULL )
				{
					value = unscale_data(data, 0.2, 1);
					sprintf(msg, "%04.3f PPH", value);
					showValue( ecs_form->s11A14, msg );						
				}
				break;

			case 37:  // 11A37 SUIT-CABIN DELTA PRESS
				if( ecs_form != NULL )
				{
					value = unscale_data(data, -5, 5);
					sprintf(msg,"%+03.2f IN",value);
					showValue( ecs_form->s11A37, msg );
				}
				break;

			case 38: // 11A38 ALPHA CT RATE CHAN 1
				if ( tcm_form != NULL )
				{
					value = unscale_data(data,0.1,10000);
					sprintf(msg,"%06.0f C/S",value);
					showValue( tcm_form->s11A38, msg );
				}
				break;

			case 39: // 11A39 SM HE MANF A PRESS
				if ( sps_form != NULL )
				{
					value = unscale_data(data,0,400);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A39, msg );
				}
				break;

			case 40: // 11A40 SM HE MANF B PRESS
				if( sps_form != NULL )
				{
					value = unscale_data(data,0,400);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A40, msg );
				}
				break;

			case 41: // 11A41 ALPHA CT RATE CHAN 2
				if ( tcm_form != NULL )
				{
					value = unscale_data(data,0.1,10000);
					sprintf(msg,"%06.0f C/S",value);
					showValue( tcm_form->s11A41, msg );
				}
				break;

			case 42: // 11A42 ALPHA CT RATE CHAN 3
				if ( tcm_form != NULL )
				{
					value = unscale_data(data,0.1,10000);
					sprintf(msg,"%06.0f C/S",value);
					showValue( tcm_form->s11A42, msg );						
				}
				break;

			case 43: // 11A43 PROTON INTEG CT RATE
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, 1, 100000);
					sprintf(msg,"%06.0f C/S",value);
					showValue( tcm_form->s11A43, msg );						
				}
				break;

			case 46: // 11A46 SM HE MANF C PRESS
				if ( sps_form != NULL )
				{
					value = unscale_data(data,0,400);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A46, msg );						
				}
				break;

			case 47: // 11A47 LM HEATER CURRENT
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 10);
					sprintf(msg,"%05.2f A",value);
					showValue( eps_form->s11A47, msg );						
				}
				break;

			case 48: // 11A48 PCM HI LEVEL 85 PCT REF
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, 0, 5);
					sprintf(msg,"%04.2f V",value);
					showValue( tcm_form->s11A48, msg );						
				}
				break;

			case 49: // 11A49 PCM LO LEVEL 15 PCT REF
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, 0, 1);
					sprintf(msg,"%04.2f V",value);
					showValue( tcm_form->s11A49, msg );						
				}
				break;

			case 50: // 11A50 USB RCVR PHASE ERR
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, -90000, 90000);
					sprintf(msg,"%+05.0f Hz",value);
					showValue( tcm_form->s11A50, msg );						
				}
				break;

			case 57: // MAIN BUS A VOLTS
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					showValue( eps_form->s11A57, msg );						
				}
				break;

			case 58: // MAIN BUS B VOLTS
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					showValue( eps_form->s11A58, msg );						
				}
				break;

			case 73: // 11A73 BAT CHARGER AMPS
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 5);
					sprintf(msg,"%05.2f A",value);
					showValue( eps_form->s11A73, msg );
				}
				break;

			case 74: // 11A74 BAT A CUR
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,100);
					sprintf(msg,"%05.2f A",value);
					showValue( eps_form->s11A74, msg );
				}
				break;

			case 75: // 11A75 BAT RELAY BUS VOLTS
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,45);
					sprintf(msg,"%02.2f V",value);
					showValue( eps_form->s11A75, msg );
				}
				break;

			case 76: // 11A76 FC 1 CUR
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,100);
					sprintf(msg,"%03.2f A",value);
					showValue( eps_form->s11A76, msg );
				}
				break;

			case 77: // 11A77 FC 1 H2 FLOW
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 0.2);
					sprintf(msg,"%04.3f PPH",value);
					showValue( eps_form->s11A77, msg );
				}
				break;

			case 78: // 11A78 FC 2 H2 FLOW
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 0.2);
					sprintf(msg,"%04.3f PPH",value);
					showValue( eps_form->s11A78, msg );
				}
				break;

			case 79: // 11A79 FC 3 H2 FLOW
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 0.2);
					sprintf(msg,"%04.3f PPH",value);
					showValue( eps_form->s11A79, msg );						
				}
				break;

			case 80: // 11A80 FC 1 O2 FLOW
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 1.6);
					sprintf(msg,"%04.3f PPH",value);
					showValue( eps_form->s11A80, msg);						
				}
				break;

			case 81: // 11A81 FC 2 O2 FLOW
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 1.6);
					sprintf(msg,"%04.3f PPH",value);
					showValue( eps_form->s11A81, msg );						
				} 
				break;

			case 82: // 11A82 FC 3 O2 FLOW
				if ( eps_form != NULL )
				{
					value = unscale_data(data, 0, 1.6);
					sprintf(msg,"%04.3f PPH",value);
					showValue( eps_form->s11A82, msg );						
				}
				break;

			case 84: // 11A84 FC 2 CUR
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,100);
					sprintf(msg,"%03.2f A",value);
					showValue( eps_form->s11A84, msg );						
				}
				break;

			case 85: // 11A85 FC 3 CUR
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,100);
					sprintf(msg,"%03.2f A",value);
					showValue( eps_form->s11A85, msg );						
				}
				break;

			case 91: // BAT BUS A VOLTS
				if( eps_form != NULL )
				{
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					showValue( eps_form->s11A91, msg );					
				}
				break;

			case 93: // BAT BUS B VOLTS
				if( eps_form != NULL )
				{
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					showValue( eps_form->s11A93, msg );					
				}
				break;

			case 109: // 11A109 BAT B CUR
				if( eps_form != NULL )
				{
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.2f A",value);
					showValue( eps_form->s11A109, msg );
				}
				break;

			case 110: // BAT C CUR
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,100);
					sprintf(msg,"%05.2f A",value);
					showValue( eps_form->s11A110, msg );
				}
				break;

			case 111: // 11A111 SM FU MANF C PRESS
				if( sps_form != NULL )
				{
					value = unscale_data(data,0,400);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A111, msg );
				}
				break;

			case 112: // 11A112 SM FU MANF D PRESS
				if ( sps_form != NULL )
				{
					value = unscale_data(data,0,400);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A112, msg );
				}
				break;

			case 118: // 11A118 SEC EVAP OUT LIQ TEMP
				if ( ecs_form != NULL )
				{
					value = unscale_data(data, 25, 75);
					sprintf(msg,"%05.2f �F",value);
					showValue( ecs_form->s11A118, msg );						
				}
				break;

			case 119: // 11A119 SENSOR EXCITATION 5V
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, 0, 9);
					sprintf(msg,"%04.2f V",value);
					showValue( tcm_form->s11A119, msg );						
				}
				break;

			case 120: // 11A120 SENSOR EXCITATION 10V
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, 0, 15);
					sprintf(msg,"%04.2f V",value);
					showValue( tcm_form->s11A120, msg );						
				}
				break;

			case 121: // 11A121 USB RCVR AGC VOLTAGE
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, -130, -50);
					sprintf(msg,"%+04.1f DBM",value);
					showValue( tcm_form->s11A121, msg );						
				}
				break;

			case 147: // 11A147 AC BUS 1 PH A VOLTS
				if ( eps_form != NULL )
				{
					value = unscale_data(data,0,150);
					sprintf(msg,"%03.2f V",value);
					showValue( eps_form->s11A147, msg );
				}
				break;

			case 148: // 11A148 SCE POS SUPPLY VOLTS
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, 0, 30);
					sprintf(msg,"%04.2f V",value);
					showValue( tcm_form->s11A148, msg );
				}
				break;

			case 152: // 11A152 FUEL SM/ENG INTERFACE P
				if (sps_form != NULL)
				{
					value = unscale_data(data, 0, 300);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s11A152, msg );						
				}
				break;

			case 154: // 11A154 SCE NEG SUPPLY VOLTS
				if ( tcm_form != NULL )
				{
					value = unscale_data(data, -30, 0);
					sprintf(msg,"%04.2f V",value);
					showValue( tcm_form->s11A154, msg );						
				}
				break;

			case 155: // 11A155 CM HE TK A TEMP
				if (sps_form != NULL)
				{
					value = unscale_data(data, 0, 300);
					sprintf(msg,"%05.2f �F",value);
					showValue( sps_form->s11A155, msg );								
				}
				break;

			case 156: // 11A156 CM HE TK B TEMP
				if (sps_form != NULL)
				{
					value = unscale_data(data, 0, 300);
					sprintf(msg,"%05.2f �F",value);
					showValue( sps_form->s11A156, msg );								
				}
				break;

			case 157:
				if ( ecs_form != NULL )
				{
					value = unscale_data(data, 0, 60);
					sprintf(msg,"%04.0f PSIG",value);
					showValue( ecs_form->s11A157, msg );						
				}
				break;
			}
			break;

		case 12: // S12A
			switch ( ccode )
			{
			case 1: // 12A1 MGA SERVO ERR IN PHASE
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, -2.5,2.5);
					sprintf(msg,"%+03.2f",value);
					showValue( gnc_form->s12A1, msg );
				}
				break;

			case 2: // 12A2 IGA SERVO ERR IN PHASE
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, -2.5,2.5);
					sprintf(msg,"%+03.2f",value);
					showValue( gnc_form->s12A2, msg );
				}
				break;

			case 3: // 12A3 OGA SERVO ERR IN PHASE
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, -2.5,2.5);
					sprintf(msg,"%+03.2f",value);
					showValue( gnc_form->s12A3, msg );
				}
				break;

			case 4: // 12A4 ROLL ATT ERR
				if ( scs_form != NULL )
				{
					value = unscale_data(data, -50, 50);
					sprintf(msg,"%+04.2f �",value);
					showValue( scs_form->s12A4, msg );
				}
				break;

			case 5: // 12A5 SCS PITCH BODY RATE
				if( scs_form != NULL )
				{
					value = unscale_data(data, -10, 10);
					sprintf(msg,"%+04.2f �",value);
					showValue( scs_form->s12A5, msg );
				}
				break;

			case 6: // 12A6 SCS YAW BODY RATE
				if ( scs_form != NULL )
				{
					value = unscale_data(data, -10, 10);
					sprintf(msg,"%+04.2f �",value);
					showValue( scs_form->s12A6, msg );						
				}
				break;

			case 7: // 12A7 SCS ROLL BODY RATE
				if ( scs_form != NULL )
				{
					value = unscale_data(data, -50, 50);
					sprintf(msg,"%+04.2f �",value);
					showValue( scs_form->s12A7, msg );						
				}
				break;

			case 8: // 12A8 PITCH GIMBL POS 1 OR 2
				if ( scs_form != NULL )
				{
					value = unscale_data(data, -5, 5);
					sprintf(msg,"%+04.2f �",value);
					showValue( scs_form->s12A8, msg );						
				}
				break;

			case 9: // 12A9 CM X-AXIS ACCEL
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, -2, 10);
					sprintf(msg,"%+05.3f G",value);
					showValue( gnc_form->s12A9, msg );			
				}
				break;

			case 10: // 12A10 YAW GIMBL POS 1 OR 2
				if ( scs_form != NULL )
				{
					value = unscale_data(data, -5, 5);
					sprintf(msg,"%+04.2f �",value);
					showValue( scs_form->s12A10, msg );						
				}
				break;

			case 11: // 12A11 CM Y-AXIS ACCEL
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, -2, 2);
					sprintf(msg,"%+05.3f G",value);
					showValue( gnc_form->s12A11, msg );						
				}
				break;

			case 12: // 12A12 CM Z-AXIS ACCEL
				if ( gnc_form != NULL )
				{
					value = unscale_data(data, -2, 2);
					sprintf(msg,"%+05.3f G",value);
					showValue( gnc_form->s12A12, msg );						
				}
				break;
			}
			break;

		case 22: // S22A
			switch( ccode )
			{
			case 1: // 22A1 ASTRO 1 EKG AXIS 2
				if( crw_form != NULL)
				{
					value = unscale_data(data, 0.1, 0.5);
					sprintf(msg,"%05.4f MV",value);
					showValue( crw_form->s22A1, msg );
				}
				break;

			case 2: // 22A2 ASTRO 1 EKG AXIS 3
				if ( crw_form != NULL )
				{
					value = unscale_data(data, 0.1, 0.5);
					sprintf(msg,"%05.4f MV",value);
					showValue( crw_form->s22A2, msg );						
				}
				break;

			case 3: // 22A3 ASTRO 1 EKG AXIS 1
				if ( crw_form != NULL )
				{
					value = unscale_data(data, 0.1, 0.5);
					sprintf(msg,"%05.4f MV",value);	
					showValue( crw_form->s22A3, msg );
				}
				break;

			case 4: // 22A4 PITCH DIFF CLUTCH CURRENT
				if ( scs_form != NULL )
				{
					value = unscale_data(data, 0.1, 0.5);
					sprintf(msg,"%04.3f A",value);
					showValue( scs_form->s22A4, msg );
				}
				break;
			}
			break;
		}
	}
}

// HBR datastream parsing
void Form1::parse_hbr(unsigned char data, int bytect){
	char msg[256];

	switch(bytect){
		case 0: // SYNC 1
			if(data != 05){ end_hbr(); lock_type = 0; cmc_lock_type = 0; }
			break;
		case 1: // SYNC 2
			if(data != 0171){ end_hbr(); lock_type = 0; cmc_lock_type = 0; }
			break;
		case 2: // SYNC 3
			if(data != 0267){ end_hbr(); lock_type = 0; cmc_lock_type = 0; }
			break;
		case 3: // FRAME COUNT
			framect = data&077; // 0-49 frame count
			// GENERATE 0-4 SUBFRAME #
			sprintf(msg,"%03d",framect);
			switch(msg[2]){
				case '0': 
				case '5': 
					framead = 0; break;
				case '1': 
				case '6': 
					framead = 1; break;
				case '2': 
				case '7': 
					framead = 2; break;
				case '3': 
				case '8': 
					framead = 3; break;
				case '4': 
				case '9': 
					framead = 4; break;
			}
			break;		

		case 4: // 22A1 ASTRO 1 EKG AXIS 2
		case 36:
		case 68:
		case 100:
			display( data, 22, TLM_A, 1 );
			break;

		case 5: // 22A2 ASTRO 1 EKG AXIS 3
		case 37:
		case 69:
		case 101:
			display( data, 22, TLM_A, 2 );
			break;

		case 6: // 22A3 ASTRO 1 EKG AXIS 1
		case 38:
		case 70:
		case 102:
			display( data, 22, TLM_A, 3 );
			break;

		case 7: // 22A4 PITCH DIFF CLUTCH CURRENT
		case 39:
		case 71:
		case 103:
			display( data, 22, TLM_A, 4 );
			break;

		case 8:
			switch(framead){
			case 0: // 11A1 SUIT MANF ABS PRESS
				display( data, 11, TLM_A, 1 );
				break;

			case 1: // 11A37 SUIT-CABIN DELTA PRESS
				display( data, 11, TLM_A, 37 );
				break;

			case 2: // 11A73 BAT CHARGER AMPS
				display( data, 11, TLM_A, 73 );
				break;

			case 3: // 11A109 BAT B CUR
				display( data, 11, TLM_A, 109 );
				break;
			}
			break;

		case 9:
			switch(framead){
			case 0: // 11A2 SUIT COMP DELTA P
				display( data, 11, TLM_A, 2 );
				break;

			case 1: // 11A38 ALPHA CT RATE CHAN 1
				display( data, 11, TLM_A, 38 );
				break;

			case 2: // 11A74 BAT A CUR
				display( data, 11, TLM_A, 74 );
				break;

			case 3: // BAT C CUR
				display( data, 11, TLM_A, 110 );
				break;
			}
			break;

		case 10:
			switch(framead) 
			{
			case 0: // 11A3 GLY PUMP OUT PRESS
				display( data, 11, TLM_A, 3 );
				break;

			case 1: // 11A39 SM HE MANF A PRESS
				display( data, 11, TLM_A, 39 );
				break;

			case 2: // 11A75 BAT RELAY BUS VOLTS
				display( data, 11, TLM_A, 75 );
				break;

			case 3: // 11A111 SM FU MANF C PRESS
				display( data, 11, TLM_A, 111 );
				break;

			case 4: // 11A147 AC BUS 1 PH A VOLTS
				display( data, 11, TLM_A, 147 );
				break;
			}
			break;

		case 11:
			switch(framead)
			{
			case 0: // 11A4 ECS SURGE TANK PRESS
				display( data, 11, TLM_A, 4 );
				break;

			case 1: // 11A40 SM HE MANF B PRESS
				display( data, 11, TLM_A, 40 );
				break;

			case 2: // 11A76 FC 1 CUR
				display( data, 11, TLM_A, 76 );
				break;

			case 3: // 11A112 SM FU MANF D PRESS
				display( data, 11, TLM_A, 112 );
				break;

			case 4: // 11A148 SCE POS SUPPLY VOLTS
				display( data, 11, TLM_A, 148 );
				break;
			}
			break;

		case 12: // 12A1 MGA SERVO ERR IN PHASE
		case 76:
			display( data, 12, TLM_A, 1 );
			break;

		case 13: // 12A2 IGA SERVO ERR IN PHASE
		case 77:
			display( data, 12, TLM_A, 2 );
			break;

		case 14: // 12A3 OGA SERVO ERR IN PHASE
		case 78:
			display( data, 12, TLM_A, 3 );
			break;

		case 15: // 12A4 ROLL ATT ERR
		case 79:
			display( data, 12, TLM_A, 4 );
			break;

		case 16:
			switch(framead)
			{
			case 0: // 11A5 PYRO BUS B VOLTS
				display( data, 11, TLM_A, 5 );
				break;

			case 1: // 11A41 ALPHA CT RATE CHAN 2
				display( data, 11, TLM_A, 41 );
				break;

			case 2: // 11A77 FC 1 H2 FLOW
				display( data, 11, TLM_A, 77 );
				break;

			case 3: // 11A113
				display( data, 11, TLM_A, 113 );
				break;

			case 4: // 11A149
				display( data, 11, TLM_A, 149 );
				break;
			}
			break;

		case 17: // 22DP1
			display( data, 22, TLM_DP, 1 );
			break;

		case 18: // 22DP2
			display( data, 22, TLM_DP, 2 );
			break;

		case 19: 
			switch (framead)
			{
			case 0: // 10DP1
				display( data, 10, TLM_DP, 1 ); 
				break;

			case 1: // SRC-0
				display( data, 0, TLM_SRC, 0 ); 
				break;

			case 2: // SRC-1
				display( data, 0, TLM_SRC, 1 ); 
				break;

			case 3: // (Zeroes?)
				break;

			case 4: // (Zeroes?)
				break;
			}
			break;

		case 20: // 12A5 SCS PITCH BODY RATE
		case 84:
			display( data, 12, TLM_A, 5 );
			break;

		case 21: // 12A6 SCS YAW BODY RATE
		case 85:
			display( data, 12, TLM_A, 6 );
			break;

		case 22: // 12A7 SCS ROLL BODY RATE
		case 86:
			display( data, 12, TLM_A, 7 );
			break;

		case 23: // 12A8 PITCH GIMBL POS 1 OR 2
		case 87:
			display( data, 12, TLM_A, 8 );
			break;

		case 24:
			switch(framead)
			{
			case 0: // 11A6 LES LOGIC BUS B VOLTS
				display( data, 11, TLM_A, 6 );
				break;

			case 1: // 11A42 ALPHA CT RATE CHAN 3
				display( data, 11, TLM_A, 42 );
				break;

			case 2: // 11A78 FC 2 H2 FLOW
				display( data, 11, TLM_A, 78 );
				break;

			case 3: // 11A114
				display( data, 11, TLM_A, 114 );
				break;

			case 4: // 11A150
				display( data, 11, TLM_A, 150 ); 
				break;
			}
			break;

		case 25:
			switch(framead)
			{
			case 0: // 11A7
				display( data, 11, TLM_A, 7 ); 
				break;

			case 1: // 11A43 PROTON INTEG CT RATE
				display( data, 11, TLM_A, 43 );
				break;

			case 2: // 11A79 FC 3 H2 FLOW
				display( data, 11, TLM_A, 79 );
				break;

			case 3: // 11A115
				display( data, 11, TLM_A, 115 ); 
				break;

			case 4: // 11A151
				display( data, 11, TLM_A, 151 ); 
				break;
			}
			break;

		case 26:
			switch(framead)
			{
			case 0: // 11A8 LES LOGIC BUS A VOLTS
				display( data, 11, TLM_A, 8 );
				break;

			case 1: // 11A44
				display( data, 11, TLM_A, 44 ); 
				break;

			case 2: // 11A80 FC 1 O2 FLOW
				display( data, 11, TLM_A, 80 ); 
				break;

			case 3: // 11A116
				display( data, 11, TLM_A, 116 ); 
				break;

			case 4: // 11A152 FUEL SM/ENG INTERFACE P
				display( data, 11, TLM_A, 152 );
				break;
			}
			break;

		case 27:
			switch(framead)
			{
			case 0: // 11A9 PYRO BUS A VOLTS
				display( data, 11, TLM_A, 9 );
				break;

			case 1: // 11A45
				display( data, 11, TLM_A, 45 );
				break;

			case 2: // 11A81 FC 2 O2 FLOW
				display( data, 11, TLM_A, 81 );
				break;

			case 3: // 11A117
				display( data, 11, TLM_A, 117 );
				break;

			case 4: // 11A153
				display( data, 11, TLM_A, 153 );
				break;
			}
			break;

		case 28: // 51A1
			display( data, 51, TLM_A, 1);
			break;

		case 29: // 51A2
			display( data, 51, TLM_A, 2);
			break;

		case 30: // 51A3
			display( data, 51, TLM_A, 3 ); 
			break;

		case 31: // CMC DATA WORD
			cmc_w0 = data&0177;
			cmc_w0 <<= 8;
			break;

		case 32: // CMC DATA WORD
			cmc_w0 |= data&0377;
			break;

		case 33: // CMC DATA WORD
			cmc_w1 = data&0177;
			cmc_w1 <<= 8;
			break;

		case 34: // CMC DATA WORD
			cmc_w1 |= data&0377;
			break;

		case 35: // CMC DATA WORD
			parse_cmc();
			break;

		case 40:
			switch(framead)
			{
			case 0: // 11A10 SPS HE TK PRESS
				display( data, 11, TLM_A, 10 );
				break;

			case 1: // 11A46 SM HE MANF C PRESS
				display( data, 11, TLM_A, 46 );
				break;

			case 2: // 11A82 FC 3 O2 FLOW
				display( data, 11, TLM_A, 82 );
				break;

			case 3: // 11A118 SEC EVAP OUT LIQ TEMP
				display( data, 11, TLM_A, 118 );
				break;

			case 4: // 11A154 SCE NEG SUPPLY VOLTS
				display( data, 11, TLM_A, 154 );
				break;
			}
			break;

		case 41:
			switch(framead)
			{
			case 0: // 11A11 SPS OX TK PRESS
				display( data, 11, TLM_A, 11 );
				break;

			case 1: // 11A47 LM HEATER CURRENT
				display( data, 11, TLM_A, 47 );
				break;

			case 2: // 11A83
				display( data, 11, TLM_A, 83 );
				break;

			case 3: // 11A119 SENSOR EXCITATION 5V
				display( data, 11, TLM_A, 119 );
				break;

			case 4: // 11A155 CM HE TK A TEMP
				display( data, 11, TLM_A, 155 );
				break;
			}
			break;

		case 42:
			switch(framead)
			{
			case 0: // 11A12 SPS FU TK PRESS
				display( data, 11, TLM_A, 12 );
				break;

			case 1: // 11A48 PCM HI LEVEL 85 PCT REF
				display( data, 11, TLM_A, 48 );
				break;

			case 2: // 11A84 FC 2 CUR
				display( data, 11, TLM_A, 84 );
				break;

			case 3: // 11A120 SENSOR EXCITATION 10V
				display( data, 11, TLM_A, 120 );
				break;

			case 4: // 11A156 CM HE TK B TEMP
				display( data, 11, TLM_A, 156 );
				break;
			}
			break;

		case 43:
			switch(framead)
			{
			case 0: // 11A13 GLY ACCUM QTY
				display( data, 11, TLM_A, 13 );
				break;

			case 1: // 11A49 PCM LO LEVEL 15 PCT REF
				display( data, 11, TLM_A, 49 );
				break;

			case 2: // 11A85 FC 3 CUR
				display( data, 11, TLM_A, 85 );
				break;

			case 3: // 11A121 USB RCVR AGC VOLTAGE
				display( data, 11, TLM_A, 121 );
				break;

			case 4: // 11A157 SEC GLY PUMP OUT PRESS
				display( data, 11, TLM_A, 157 );
				break;
			}
			break;

		case 44: // 12A9 CM X-AXIS ACCEL
		case 108:
			display( data, 12, TLM_A, 9 );
			break;

		case 45: // 12A10 YAW GIMBL POS 1 OR 2
		case 109:
			display( data, 12, TLM_A, 10 );
			break;

		case 46: // 12A11 CM Y-AXIS ACCEL
		case 110:
			display( data, 12, TLM_A, 11 );
			break;

		case 47: // 12A12 CM Z-AXIS ACCEL
		case 111:
			display( data, 12, TLM_A, 12 );
			break;

		case 48:
			switch(framead)
			{
			case 0: // 11A14 ECS O2 FLOW O2 SUPPLY MANF
				display( data, 11, TLM_A, 14 );
				break;

			case 1: // 11A50 USB RCVR PHASE ERR
				display( data, 11, TLM_A, 50 );
				break;

			case 2: // 11A86
				display( data, 11, TLM_A, 86 );
				break;

			case 3: // 11A122
				display( data, 11, TLM_A, 122 );
				break;

			case 4: // 11A158
				display( data, 11, TLM_A, 158 );
				break;
			}
			break;

		case 51: // MAGICAL WORD 1
			// 10A1
			// 10A4
			// 10A7
			// ...
			// 10A148
			display( data, 10, TLM_A, 1+(framect*3) );
			break;

		case 72:
			switch(framead){
			case 2:	// BAT BUS A VOLTS
				display( data, 11, TLM_A, 91 );
				break;
			}
			break;

		case 74:
			switch(framead){
			case 1: // MAIN BUS A VOLTS
				display( data, 11, TLM_A, 57 );
				break;

			case 2: // BAT BUS B VOLTS
				display( data, 11, TLM_A, 93 );
				break;
			}
			break;

		case 75:
			switch(framead){
			case 1: // MAIN BUS B VOLTS
				display( data, 11, TLM_A, 58 );
				break;
			}
			break;

		case 83: // MAGICAL WORD 2
			// 10A2
			// 10A5
			// 10A8
			// ...
			// 10A149
			display( data, 10, TLM_A, 2+(framect*3) );
			break;

		case 115: // MAGICAL WORD 3
			// 10A3
			// 10A6
			// 10A9
			// ...
			// 10A150
			display( data, 10, TLM_A, 3+(framect*3) );
			break;
	}
}

// Enable/disable stuff in CMCForm and setup for the given list
void Form1::setup_cmc_list(){
	if(cmc_form == NULL){ return; } // Safeguard
	bool onoff;
	if(cmc_lock_type != 0){ onoff = TRUE; }else{ onoff = FALSE; }

	cmc_form->cmcListID->Enabled = onoff;

	cmc_form->textBox1->Enabled = onoff;
	cmc_form->textBox2->Enabled = onoff;
	cmc_form->textBox3->Enabled = onoff;
	cmc_form->textBox4->Enabled = onoff;
	cmc_form->textBox5->Enabled = onoff;
	cmc_form->textBox6->Enabled = onoff;
	cmc_form->textBox7->Enabled = onoff;
	cmc_form->textBox8->Enabled = onoff;
	cmc_form->textBox9->Enabled = onoff;

	cmc_form->textBox10->Enabled = onoff;
	cmc_form->textBox11->Enabled = onoff;
	cmc_form->textBox12->Enabled = onoff;
	cmc_form->textBox13->Enabled = onoff;
	cmc_form->textBox14->Enabled = onoff;
	cmc_form->textBox15->Enabled = onoff;
	cmc_form->textBox16->Enabled = onoff;
	cmc_form->textBox17->Enabled = onoff;
	cmc_form->textBox18->Enabled = onoff;
	cmc_form->textBox19->Enabled = onoff;

	cmc_form->textBox20->Enabled = onoff;
	cmc_form->textBox21->Enabled = onoff;
	cmc_form->textBox22->Enabled = onoff;
	cmc_form->textBox23->Enabled = onoff;
	cmc_form->textBox24->Enabled = onoff;
	cmc_form->textBox25->Enabled = onoff;
	cmc_form->textBox26->Enabled = onoff;
	cmc_form->textBox27->Enabled = onoff;
	cmc_form->textBox28->Enabled = onoff;
	cmc_form->textBox29->Enabled = onoff;

	cmc_form->textBox30->Enabled = onoff;
	cmc_form->textBox31->Enabled = onoff;
	cmc_form->textBox32->Enabled = onoff;
	cmc_form->textBox33->Enabled = onoff;
	cmc_form->textBox34->Enabled = onoff;
	cmc_form->textBox35->Enabled = onoff;
	cmc_form->textBox36->Enabled = onoff;
	cmc_form->textBox37->Enabled = onoff;
	cmc_form->textBox38->Enabled = onoff;
	cmc_form->textBox39->Enabled = onoff;

	cmc_form->textBox40->Enabled = onoff;
	cmc_form->textBox41->Enabled = onoff;
	cmc_form->textBox42->Enabled = onoff;
	cmc_form->textBox43->Enabled = onoff;
	cmc_form->textBox44->Enabled = onoff;
	cmc_form->textBox45->Enabled = onoff;
	cmc_form->textBox46->Enabled = onoff;
	cmc_form->textBox47->Enabled = onoff;
	cmc_form->textBox48->Enabled = onoff;
	cmc_form->textBox49->Enabled = onoff;

	cmc_form->textBox50->Enabled = onoff;
	cmc_form->textBox51->Enabled = onoff;
	cmc_form->textBox52->Enabled = onoff;
	cmc_form->textBox53->Enabled = onoff;
	cmc_form->textBox54->Enabled = onoff;
	cmc_form->textBox55->Enabled = onoff;
	cmc_form->textBox56->Enabled = onoff;
	cmc_form->textBox57->Enabled = onoff;
	cmc_form->textBox58->Enabled = onoff;
	cmc_form->textBox59->Enabled = onoff;

	cmc_form->textBox60->Enabled = onoff;
	cmc_form->textBox61->Enabled = onoff;
	cmc_form->textBox62->Enabled = onoff;
	cmc_form->textBox63->Enabled = onoff;
	cmc_form->textBox64->Enabled = onoff;
	cmc_form->textBox65->Enabled = onoff;
	cmc_form->textBox66->Enabled = onoff;
	cmc_form->textBox67->Enabled = onoff;
	cmc_form->textBox68->Enabled = onoff;
	cmc_form->textBox69->Enabled = onoff;

	cmc_form->textBox70->Enabled = onoff;
	cmc_form->textBox71->Enabled = onoff;
	cmc_form->textBox72->Enabled = onoff;
	cmc_form->textBox73->Enabled = onoff;
	cmc_form->textBox74->Enabled = onoff;
	cmc_form->textBox75->Enabled = onoff;
	cmc_form->textBox76->Enabled = onoff;
	cmc_form->textBox77->Enabled = onoff;
	cmc_form->textBox78->Enabled = onoff;
	cmc_form->textBox79->Enabled = onoff;

	cmc_form->textBox80->Enabled = onoff;
	cmc_form->textBox81->Enabled = onoff;
	cmc_form->textBox82->Enabled = onoff;
	cmc_form->textBox83->Enabled = onoff;
	cmc_form->textBox84->Enabled = onoff;
	cmc_form->textBox85->Enabled = onoff;
	cmc_form->textBox86->Enabled = onoff;
	cmc_form->textBox87->Enabled = onoff;
	cmc_form->textBox88->Enabled = onoff;
	cmc_form->textBox89->Enabled = onoff;

	cmc_form->textBox90->Enabled = onoff;
	cmc_form->textBox91->Enabled = onoff;
	cmc_form->textBox92->Enabled = onoff;
	cmc_form->textBox93->Enabled = onoff;
	cmc_form->textBox94->Enabled = onoff;
	cmc_form->textBox95->Enabled = onoff;
	cmc_form->textBox96->Enabled = onoff;
	cmc_form->textBox97->Enabled = onoff;
	cmc_form->textBox98->Enabled = onoff;
	cmc_form->textBox99->Enabled = onoff;

	cmc_form->textBox100->Enabled = onoff;
	cmc_form->textBox101->Enabled = onoff;
	cmc_form->textBox102->Enabled = onoff;
	cmc_form->textBox103->Enabled = onoff;
	cmc_form->textBox104->Enabled = onoff;
	cmc_form->textBox105->Enabled = onoff;
	cmc_form->textBox106->Enabled = onoff;
	cmc_form->textBox107->Enabled = onoff;
	cmc_form->textBox108->Enabled = onoff;
	cmc_form->textBox109->Enabled = onoff;

	cmc_form->textBox110->Enabled = onoff;
	cmc_form->textBox111->Enabled = onoff;
	cmc_form->textBox112->Enabled = onoff;
	cmc_form->textBox113->Enabled = onoff;
	cmc_form->textBox114->Enabled = onoff;
	cmc_form->textBox115->Enabled = onoff;
	cmc_form->textBox116->Enabled = onoff;
	cmc_form->textBox117->Enabled = onoff;
	cmc_form->textBox118->Enabled = onoff;
	cmc_form->textBox119->Enabled = onoff;

	cmc_form->textBox120->Enabled = onoff;
	cmc_form->textBox121->Enabled = onoff;
	cmc_form->textBox122->Enabled = onoff;
	cmc_form->textBox123->Enabled = onoff;
	cmc_form->textBox124->Enabled = onoff;
	cmc_form->textBox125->Enabled = onoff;
	cmc_form->textBox126->Enabled = onoff;
	cmc_form->textBox127->Enabled = onoff;
	cmc_form->textBox128->Enabled = onoff;
	cmc_form->textBox129->Enabled = onoff;

	cmc_form->textBox130->Enabled = onoff;
	cmc_form->textBox131->Enabled = onoff;
	cmc_form->textBox132->Enabled = onoff;
	cmc_form->textBox133->Enabled = onoff;
	cmc_form->textBox134->Enabled = onoff;
	cmc_form->textBox135->Enabled = onoff;
	cmc_form->textBox136->Enabled = onoff;
	cmc_form->textBox137->Enabled = onoff;
	cmc_form->textBox138->Enabled = onoff;
	cmc_form->textBox139->Enabled = onoff;

	cmc_form->textBox140->Enabled = onoff;
	cmc_form->textBox141->Enabled = onoff;
	cmc_form->textBox142->Enabled = onoff;
	cmc_form->textBox143->Enabled = onoff;
	cmc_form->textBox144->Enabled = onoff;
	cmc_form->textBox145->Enabled = onoff;
	cmc_form->textBox146->Enabled = onoff;
	cmc_form->textBox147->Enabled = onoff;
	cmc_form->textBox148->Enabled = onoff;
	cmc_form->textBox149->Enabled = onoff;

	cmc_form->textBox150->Enabled = onoff;
	cmc_form->textBox151->Enabled = onoff;
	cmc_form->textBox152->Enabled = onoff;
	cmc_form->textBox153->Enabled = onoff;
	cmc_form->textBox154->Enabled = onoff;
	cmc_form->textBox155->Enabled = onoff;
	cmc_form->textBox156->Enabled = onoff;
	cmc_form->textBox157->Enabled = onoff;
	cmc_form->textBox158->Enabled = onoff;
	cmc_form->textBox159->Enabled = onoff;

	cmc_form->textBox160->Enabled = onoff;
	cmc_form->textBox161->Enabled = onoff;
	cmc_form->textBox162->Enabled = onoff;
	cmc_form->textBox163->Enabled = onoff;
	cmc_form->textBox164->Enabled = onoff;
	cmc_form->textBox165->Enabled = onoff;
	cmc_form->textBox166->Enabled = onoff;
	cmc_form->textBox167->Enabled = onoff;
	cmc_form->textBox168->Enabled = onoff;
	cmc_form->textBox169->Enabled = onoff;

	cmc_form->textBox170->Enabled = onoff;
	cmc_form->textBox171->Enabled = onoff;
	cmc_form->textBox172->Enabled = onoff;
	cmc_form->textBox173->Enabled = onoff;
	cmc_form->textBox174->Enabled = onoff;
	cmc_form->textBox175->Enabled = onoff;
	cmc_form->textBox176->Enabled = onoff;
	cmc_form->textBox177->Enabled = onoff;
	cmc_form->textBox178->Enabled = onoff;
	cmc_form->textBox179->Enabled = onoff;
}

void Form1::parse_cmc(){
	char msg[256];
	// All CMC data lists are 200 registers long.

	if(cmc_form == NULL){ return; } // Don't waste time with this if we aren't reading the output.

	if(cmc_lock_type == 0){
		cmc_frame_addr = 1;   // Hold at one
		if(cmc_w1 == 077340){ // Check for SYNC 1
				switch(cmc_w0){   // Switch other halfword
					case 077777:  // COAST AND ALIGN 
						cmc_lock_type = 1; 
						cmc_form->cmcListID->Text = "COAST/ALIGN";
						setup_cmc_list();
						break;
					case 077776:  // ENTRY AND UPDATE
						cmc_lock_type = 2; 
						cmc_form->cmcListID->Text = "ENTRY/UPDATE"; 
						setup_cmc_list();
						break;
					case 077775:  // RDZ AND PRETHRUST
						cmc_lock_type = 3;
						cmc_form->cmcListID->Text = "RDZ/PRETHRUST";
						setup_cmc_list();
						break;
					case 077774:  // POWERED LIST
						cmc_lock_type = 4;
						cmc_form->cmcListID->Text = "POWERED"; 
						setup_cmc_list();
						break;
					case 077773:  // ORBITAL NAV
						cmc_lock_type = 5;
						cmc_form->cmcListID->Text = "ORBITAL NAV"; 
						setup_cmc_list();
						break;
					default:
						cmc_lock_type = 0;
						cmc_form->cmcListID->Text = "NO SYNC"; 
						break;
				}
			}else{
				cmc_lock_type = 0;
				cmc_form->cmcListID->Text = "NO SYNC"; 
			}
			return;
	}else{
		cmc_frame_addr++;
		if(cmc_frame_addr > 100){
			cmc_frame_addr = 1; // LOOP
		}
	}
	// ACTUAL DATA PARSING HERE
	switch(cmc_frame_addr){
		case 1: // SYNC WORDS
			if(cmc_w1 == 077340){ // Check for SYNC 1
				switch(cmc_w0){   // Switch other halfword
					case 077777:  // COAST AND ALIGN 
						cmc_lock_type = 1; 
						cmc_form->cmcListID->Text = "COAST/ALIGN"; 
						setup_cmc_list();
						break;
					case 077776:  // ENTRY AND UPDATE
						cmc_lock_type = 2; 
						cmc_form->cmcListID->Text = "ENTRY/UPDATE"; 
						setup_cmc_list();
						break;
					case 077775:  // RDZ AND PRETHRUST
						cmc_lock_type = 3;
						cmc_form->cmcListID->Text = "RDZ/PRETHRUST"; 
						setup_cmc_list();
						break;
					case 077774:  // POWERED LIST
						cmc_lock_type = 4;
						cmc_form->cmcListID->Text = "POWERED"; 
						setup_cmc_list();
						break;
					case 077773:  // ORBITAL NAV
						cmc_lock_type = 5;
						cmc_form->cmcListID->Text = "ORBITAL NAV"; 
						setup_cmc_list();
						break;
					default:
						cmc_lock_type = 0;
						cmc_form->cmcListID->Text = "NO SYNC"; 
						break;
				}
			}else{
				cmc_lock_type = 0;
				cmc_form->cmcListID->Text = "NO SYNC";
			}
			break;
		// CSM STATE VECTOR
		case 2: 
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox1->Text = msg; break;
		case 3:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox2->Text = msg; break;
		case 4:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox3->Text = msg; break;
		case 5:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox4->Text = msg; break;
		case 6:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox5->Text = msg; break;
		case 7:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox6->Text = msg; break;
		case 8:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox7->Text = msg; break;
		// CDU INPUT COUNTERS X,Y,Z,TRUN
		case 9:
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox10->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox9->Text = msg;
			break;
		case 10:
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox8->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox11->Text = msg;
			break;
		// ADOT X-Y-Z
		case 11:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox14->Text = msg; break;
		case 12:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox13->Text = msg; break;
		case 13:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox15->Text = msg; break;
		// AK (FDAI ERR) and RCSFLAGS
		case 14:
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox17->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox16->Text = msg;
			break;
		case 15:
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox18->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox19->Text = msg;
			break;
		// THETADx/y/z and DELCDUx
		case 16:
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox21->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox20->Text = msg;
			break;
		case 17:
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox22->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox23->Text = msg;
			break;
		// TIG               (LIST 1,2,3)
		/* ****************** LIST 4 SVMRKDAT START ***************** */
		// CMDAPMOD and PREL (LIST 5)
		case 18:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox24->Text = msg;
					cmc_form->textBox160->Text = "XXXXX";
					cmc_form->textBox163->Text = "XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox160->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox163->Text = msg;
					cmc_form->textBox24->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// BESTI and BESTJ (LIST 1)
		// DELLT4          (LIST 2,3)
		// QREL and RREL   (LIST 5)
		case 19:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox26->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox25->Text = msg;
					cmc_form->textBox65->Text = "XXXXX-XXXXX";
					cmc_form->textBox162->Text = "XXXXX";
					cmc_form->textBox161->Text = "XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox162->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox161->Text = msg;
					cmc_form->textBox65->Text = "XXXXX-XXXXX";
					cmc_form->textBox26->Text = "XXXXX";
					cmc_form->textBox25->Text = "XXXXX";
					break;
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox65->Text = msg; 
					cmc_form->textBox26->Text = "XXXXX";
					cmc_form->textBox25->Text = "XXXXX";
					cmc_form->textBox162->Text = "XXXXX";
					cmc_form->textBox161->Text = "XXXXX";
					break;
			}
			break;
		// MARKDOWN and BDT       (LIST 1)
		// RTARGx/y/z and VHFTIME (LIST 2)
		// RTARGx/y/z and TGO     (LIST 3)
		// LD1                    (LIST 5)
		case 20:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox27->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox28->Text = msg;
					cmc_form->textBox132->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox164->Text = msg; 
					cmc_form->textBox132->Text = "XXXXX-XXXXX";
					cmc_form->textBox27->Text = "XXXXX";
					cmc_form->textBox28->Text = "XXXXX";
					break;
				case 3: 
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox132->Text = msg; 
					break;
			}
			break;
		/* ****************** LIST 5 UPBUFF START ******************* */
		case 21:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox31->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox30->Text = msg;
					cmc_form->textBox131->Text = "XXXXX-XXXXX";
					break;
				case 2:
					cmc_form->textBox30->Text = "XXXXX";
					cmc_form->textBox31->Text = "XXXXX";
					cmc_form->textBox131->Text = "XXXXX-XXXXX";
					cmc_upbuff[0] = cmc_w0; cmc_upbuff[1] = cmc_w1;
					break;
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox131->Text = msg; 
					break;
			}
			break;
		case 22:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox33->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox29->Text = msg;
					cmc_form->textBox130->Text = "XXXXX-XXXXX";
					break;
				case 2:
					cmc_form->textBox33->Text = "XXXXX";
					cmc_form->textBox29->Text = "XXXXX";
					cmc_form->textBox130->Text = "XXXXX-XXXXX";
					cmc_upbuff[2] = cmc_w0; cmc_upbuff[3] = cmc_w1;
					break;
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox130->Text = msg; 
					break;
			}
			break;
		case 23:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox32->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox34->Text = msg;
					cmc_form->textBox66->Text = "XXXXX-XXXXX";
					cmc_form->textBox152->Text = "XXXXX-XXXXX";
					break;
				case 2:
					cmc_form->textBox32->Text = "XXXXX";
					cmc_form->textBox34->Text = "XXXXX";
					cmc_form->textBox152->Text = "XXXXX-XXXXX";
					cmc_form->textBox66->Text = "XXXXX-XXXXX";
					cmc_upbuff[4] = cmc_w0; cmc_upbuff[5] = cmc_w1;
					break;
				case 3: 
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox66->Text = msg; 
					cmc_form->textBox152->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox152->Text = msg; 
					cmc_form->textBox66->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// MARK2DWN and UNITV+2  (LIST 1)
		// MARKDOWN and RM       (LIST 2)
		// PIPTIME and DELVx/y/z (LIST 3)
		case 24:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox36->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox35->Text = msg;
					cmc_form->textBox153->Text = "XXXXX-XXXXX";
					break;
				case 2:
					cmc_form->textBox36->Text = "XXXXX";
					cmc_form->textBox35->Text = "XXXXX";
					cmc_form->textBox27->Text = "XXXXX";
					cmc_form->textBox28->Text = "XXXXX";
					//cmc_form->textBox153->Text = "XXXXX-XXXXX";
					cmc_upbuff[6] = cmc_w0; cmc_upbuff[7] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox27->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox28->Text = msg;
					cmc_form->textBox36->Text = "XXXXX";
					cmc_form->textBox35->Text = "XXXXX";
					cmc_form->textBox153->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox153->Text = msg; 
					cmc_form->textBox36->Text = "XXXXX";
					cmc_form->textBox35->Text = "XXXXX";					
					cmc_form->textBox27->Text = "XXXXX";
					cmc_form->textBox28->Text = "XXXXX";					
					break;
			}
			break;
		case 25:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox39->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox41->Text = msg;
					break;
				case 2:
					cmc_form->textBox39->Text = "XXXXX";
					cmc_form->textBox41->Text = "XXXXX";
					cmc_form->textBox31->Text = "XXXXX";
					cmc_form->textBox30->Text = "XXXXX";
					cmc_form->textBox138->Text = "XXXXX-XXXXX";
					cmc_upbuff[8] = cmc_w0; cmc_upbuff[9] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox31->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox30->Text = msg;
					cmc_form->textBox39->Text = "XXXXX";
					cmc_form->textBox41->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox138->Text = msg; 
					cmc_form->textBox39->Text = "XXXXX";
					cmc_form->textBox41->Text = "XXXXX";					
					cmc_form->textBox31->Text = "XXXXX";
					cmc_form->textBox30->Text = "XXXXX";					
					break;
			}
			break;
		case 26:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox37->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox40->Text = msg;
					break;
				case 2:
					cmc_form->textBox37->Text = "XXXXX";
					cmc_form->textBox40->Text = "XXXXX";
					cmc_form->textBox33->Text = "XXXXX";
					cmc_form->textBox29->Text = "XXXXX";
					cmc_form->textBox76->Text = "XXXXX-XXXXX";
					cmc_upbuff[10] = cmc_w0; cmc_upbuff[11] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox33->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox29->Text = msg;
					cmc_form->textBox37->Text = "XXXXX";
					cmc_form->textBox40->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox76->Text = msg; 
					cmc_form->textBox37->Text = "XXXXX";
					cmc_form->textBox40->Text = "XXXXX";					
					cmc_form->textBox33->Text = "XXXXX";
					cmc_form->textBox29->Text = "XXXXX";					
					break;
			}
			break;
		case 27:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox38->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox42->Text = msg;
					cmc_form->textBox67->Text = "XXXXX";
					break;
				case 2:
					cmc_form->textBox38->Text = "XXXXX";
					cmc_form->textBox42->Text = "XXXXX";
					cmc_form->textBox32->Text = "XXXXX";
					cmc_form->textBox67->Text = "XXXXX";
					cmc_form->textBox75->Text = "XXXXX-XXXXX";
					cmc_upbuff[12] = cmc_w0; cmc_upbuff[13] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox32->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox67->Text = msg;
					cmc_form->textBox38->Text = "XXXXX";
					cmc_form->textBox42->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox75->Text = msg; 
					cmc_form->textBox38->Text = "XXXXX";
					cmc_form->textBox42->Text = "XXXXX";					
					cmc_form->textBox32->Text = "XXXXX";
					cmc_form->textBox67->Text = "XXXXX";					
					break;
			}
			break;
		// HAPO and HPER             (LIST 1)
		// VHFCNT,TRKMCNT and TTPI   (LIST 2)
		// PACTOFF,YACTOFF,PCMD,YCMD (LIST 3)
		case 28:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox44->Text = msg;
					cmc_form->textBox70->Text = "XXXXX";
					cmc_form->textBox69->Text = "XXXXX";
					cmc_form->textBox156->Text = "XXXXX";
					cmc_form->textBox155->Text = "XXXXX";
					break;
				case 2:
					cmc_form->textBox70->Text = "XXXXX";
					cmc_form->textBox69->Text = "XXXXX";
					cmc_form->textBox156->Text = "XXXXX";
					cmc_form->textBox155->Text = "XXXXX";
					cmc_form->textBox44->Text = "XXXXX-XXXXX";
					cmc_upbuff[14] = cmc_w0; cmc_upbuff[15] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox70->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox69->Text = msg;
					cmc_form->textBox44->Text = "XXXXX-XXXXX";
					cmc_form->textBox156->Text = "XXXXX";
					cmc_form->textBox155->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox156->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox155->Text = msg;
					cmc_form->textBox44->Text = "XXXXX-XXXXX";
					cmc_form->textBox70->Text = "XXXXX";
					cmc_form->textBox69->Text = "XXXXX";
					break;
			}
			break;
		case 29:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox43->Text = msg;
					cmc_form->textBox68->Text = "XXXXX-XXXXX";
					cmc_form->textBox154->Text = "XXXXX";
					cmc_form->textBox157->Text = "XXXXX";
					break;
				case 2:
					cmc_form->textBox154->Text = "XXXXX";
					cmc_form->textBox157->Text = "XXXXX";
					cmc_form->textBox43->Text = "XXXXX-XXXXX";
					cmc_form->textBox68->Text = "XXXXX-XXXXX";
					cmc_upbuff[16] = cmc_w0; cmc_upbuff[17] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox68->Text = msg;
					cmc_form->textBox43->Text = "XXXXX-XXXXX";
					cmc_form->textBox154->Text = "XXXXX";
					cmc_form->textBox157->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox154->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox157->Text = msg;
					cmc_form->textBox43->Text = "XXXXX-XXXXX";
					cmc_form->textBox68->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// RSP-RREC         (LIST 1)
		// ECSTEER and LSAT (LIST 2)
		// CSTEER and ALP   (LIST 3)
		/* ****************** LIST 5 UPBUFF ENDS ******************** */
		case 30:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox45->Text = msg;
					cmc_form->textBox71->Text = "XXXXX";
					cmc_form->textBox72->Text = "XXXXX";
					cmc_form->textBox159->Text = "XXXXX";
					cmc_form->textBox158->Text = "XXXXX";
					break;
				case 2:
					cmc_form->textBox71->Text = "XXXXX";
					cmc_form->textBox72->Text = "XXXXX";
					cmc_form->textBox159->Text = "XXXXX";
					cmc_form->textBox158->Text = "XXXXX";
					cmc_form->textBox45->Text = "XXXXX-XXXXX";
					cmc_upbuff[18] = cmc_w0; cmc_upbuff[19] = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox71->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox72->Text = msg;
					cmc_form->textBox45->Text = "XXXXX-XXXXX";
					cmc_form->textBox159->Text = "XXXXX";
					cmc_form->textBox158->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox159->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox158->Text = msg;
					cmc_form->textBox45->Text = "XXXXX-XXXXX";
					cmc_form->textBox71->Text = "XXXXX";
					cmc_form->textBox72->Text = "XXXXX";
					break;
			}
			break;
		// VGTIGx/y/z         (LIST 1)
		// DELVTPF and SPARES (LIST 2)
		// SPARES             (LIST 3)
		// COMPNUMB,UPOLDMOD  (LIST 5)
		case 31:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox48->Text = msg;
					cmc_form->textBox73->Text = "XXXXX-XXXXX";
					break;
				case 2:
					cmc_form->textBox73->Text = "XXXXX-XXXXX";
					cmc_compnumb = cmc_w0; cmc_upoldmod = cmc_w1;
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox73->Text = msg;
					cmc_form->textBox48->Text = "XXXXX-XXXXX";
					break;
				case 4:
					cmc_form->textBox73->Text = "XXXXX-XXXXX";					
					break;
			}
			break;
		// UPVERB,UPCOUNT     (LIST 5)
		case 32:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox47->Text = msg;
					break;
				case 2:
					cmc_form->textBox47->Text = "XXXXX-XXXXX";
					cmc_upverb = cmc_w0; cmc_upcount = cmc_w1;
					break;
				case 3:
					cmc_form->textBox47->Text = "XXXXX-XXXXX";
					break;
				case 4:
					break;
			}
			// Kick uplink processor
			cmc_uplink_process();
			break;
		// PAXERRI,ROLLTM     (LIST 5)
		case 33:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox46->Text = msg;
					cmc_form->textBox165->Text = "XXXXX";
					cmc_form->textBox166->Text = "XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox166->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox165->Text = msg;
					cmc_form->textBox46->Text = "XXXXX-XXXXX";
					break;
				case 3:
					cmc_form->textBox46->Text = "XXXXX-XXXXX";
					cmc_form->textBox166->Text = "XXXXX";
					cmc_form->textBox165->Text = "XXXXX";
					break;
				case 4:
					break;
			}
			break;
		// REFSMMAT                            (LIST 1,3)
		// TPASS4,DELVSLVx/y/z,RANGE and RRATE (LIST 2)
		// LATANG                              (LIST 5)
		case 34:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox49->Text = msg;
					cmc_form->textBox74->Text = "XXXXX-XXXXX";
					cmc_form->textBox169->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox169->Text = msg;
					cmc_form->textBox74->Text = "XXXXX-XXXXX";
					cmc_form->textBox49->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox74->Text = msg;
					cmc_form->textBox169->Text = "XXXXX-XXXXX";
					cmc_form->textBox49->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox49->Text = msg;
					break;
			}
			break;
		/* ****************** LIST 4 SVMRKDAT ENDS ****************** */
		// RDOT                                (LIST 5)
		case 35:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox50->Text = msg;
					cmc_form->textBox138->Text = "XXXXX-XXXXX";
					cmc_form->textBox168->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox168->Text = msg;
					cmc_form->textBox50->Text = "XXXXX-XXXXX";
					cmc_form->textBox138->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox138->Text = msg;
					cmc_form->textBox50->Text = "XXXXX-XXXXX";
					cmc_form->textBox168->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox50->Text = msg;
					cmc_form->textBox168->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// LANDMARK and ECC (LIST 4)
		// THETAH           (LIST 5)
		case 36:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox51->Text = msg;
					cmc_form->textBox76->Text = "XXXXX-XXXXX";
					cmc_form->textBox167->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox167->Text = msg;
					cmc_form->textBox76->Text = "XXXXX-XXXXX";
					cmc_form->textBox51->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox76->Text = msg;
					cmc_form->textBox51->Text = "XXXXX-XXXXX";
					cmc_form->textBox167->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox51->Text = msg;
					cmc_form->textBox167->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// SPARE  (LIST 4)
		// LATSPL (LIST 5)
		case 37:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox52->Text = msg;
					cmc_form->textBox75->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox148->Text = msg;
					cmc_form->textBox75->Text = "XXXXX-XXXXX";
					cmc_form->textBox52->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox75->Text = msg;
					cmc_form->textBox52->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox52->Text = msg;
					break;
			}
			break;
		// SPARE  (LIST 4)
		// LNGSPL (LIST 5)
		case 38:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox53->Text = msg;
					cmc_form->textBox140->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox149->Text = msg;
					cmc_form->textBox53->Text = "XXXXX-XXXXX";
					cmc_form->textBox140->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox140->Text = msg;
					cmc_form->textBox53->Text = "XXXXX-XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox53->Text = msg;
					break;
			}
			break;
		// SPARE             (LIST 4)
		// ALFA/180,BETA/180 (LIST 5)
		case 39:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox54->Text = msg;
					cmc_form->textBox139->Text = "XXXXX-XXXXX";
					cmc_form->textBox170->Text = "XXXXX";
					cmc_form->textBox171->Text = "XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox171->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox170->Text = msg;
					cmc_form->textBox139->Text = "XXXXX-XXXXX";
					cmc_form->textBox54->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox139->Text = msg;
					cmc_form->textBox54->Text = "XXXXX-XXXXX";
					cmc_form->textBox170->Text = "XXXXX";
					cmc_form->textBox171->Text = "XXXXX";
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox54->Text = msg;
					cmc_form->textBox170->Text = "XXXXX";
					cmc_form->textBox171->Text = "XXXXX";
					break;
			}
			break;
		// FLAGWORDS
		case 40: // FW 0-1
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox55->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox56->Text = msg;
			break;
		case 41: // FW 2-3
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox58->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox57->Text = msg;
			break;
		case 42: // FW 4-5
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox60->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox59->Text = msg;
			break;
		case 43: // FW 6-7
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox63->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox62->Text = msg;
			break;
		case 44: // FW 8-9
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox61->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox64->Text = msg;
			break;
		// DSPTAB
		case 45:
			dsptab[0] = cmc_w0; dsptab[1] = cmc_w1;
			break;
		case 46: 
			dsptab[2] = cmc_w0; dsptab[3] = cmc_w1;
			break;
		case 47: 
			dsptab[4] = cmc_w0; dsptab[5] = cmc_w1;
			break;
		case 48: 
			dsptab[6] = cmc_w0; dsptab[7] = cmc_w1;
			break;
		case 49: 
			dsptab[8] = cmc_w0; dsptab[9] = cmc_w1;
			break;
		case 50: 
			dsptab[10] = cmc_w0; dsptab[11] = cmc_w1;			
			// PARSE DSPTAB AND CREATE DSKY DISPLAY
			char tmp[12];      // Temporary
			unsigned int bits; // Temporary
			// PROG
			bits = ((dsptab[10]&01740)>>5);
			tmp[0] = get_dsky_char(bits);
			bits = (dsptab[10]&037);
			tmp[1] = get_dsky_char(bits);
			tmp[2] = 0;
			cmc_form->textBox129->Text = tmp;
			if(upl_form != NULL){ upl_form->textBox129->Text = tmp; upl_form->textBox129->Enabled = true; }
			// VERB
			bits = ((dsptab[9]&01740)>>5);
			tmp[0] = get_dsky_char(bits);
			bits = (dsptab[9]&037);
			tmp[1] = get_dsky_char(bits);
			tmp[2] = 0;
			cmc_form->textBox133->Text = tmp;
			if(upl_form != NULL){ upl_form->textBox133->Text = tmp; upl_form->textBox133->Enabled = true;}
			// NOUN
			bits = ((dsptab[8]&01740)>>5);
			tmp[0] = get_dsky_char(bits);
			bits = (dsptab[8]&037);
			tmp[1] = get_dsky_char(bits);
			tmp[2] = 0;
			cmc_form->textBox134->Text = tmp;
			if(upl_form != NULL){ upl_form->textBox134->Text = tmp; upl_form->textBox134->Enabled = true;}
			// R1
			tmp[0] = '_';
			if(dsptab[5]&02000){
				tmp[0] = '-';
			}
			if(dsptab[6]&02000){
				tmp[0] = '+';
			}
			bits = (dsptab[7]&037);
			tmp[1] = get_dsky_char(bits);
			bits = ((dsptab[6]&01740)>>5);
			tmp[2] = get_dsky_char(bits);
			bits = (dsptab[6]&037);
			tmp[3] = get_dsky_char(bits);
			bits = ((dsptab[5]&01740)>>5);
			tmp[4] = get_dsky_char(bits);
			bits = (dsptab[5]&037);
			tmp[5] = get_dsky_char(bits);
			tmp[6] = 0;
			cmc_form->textBox135->Text = tmp;
			if(upl_form != NULL){ upl_form->textBox135->Text = tmp; upl_form->textBox135->Enabled = true;}
			// R2
			tmp[0] = '_';
			if(dsptab[3]&02000){
				tmp[0] = '-';
			}
			if(dsptab[4]&02000){
				tmp[0] = '+';
			}
			bits = ((dsptab[4]&01740)>>5);
			tmp[1] = get_dsky_char(bits);
			bits = (dsptab[4]&037);
			tmp[2] = get_dsky_char(bits);
			bits = ((dsptab[3]&01740)>>5);
			tmp[3] = get_dsky_char(bits);
			bits = (dsptab[3]&037);
			tmp[4] = get_dsky_char(bits);
			bits = ((dsptab[2]&01740)>>5);
			tmp[5] = get_dsky_char(bits);
			tmp[6] = 0;
			cmc_form->textBox136->Text = tmp;
			if(upl_form != NULL){ upl_form->textBox136->Text = tmp; upl_form->textBox136->Enabled = true;}
			// R3
			tmp[0] = '_';
			if(dsptab[0]&02000){
				tmp[0] = '-';
			}
			if(dsptab[1]&02000){
				tmp[0] = '+';
			}
			bits = (dsptab[2]&037);
			tmp[1] = get_dsky_char(bits);
			bits = ((dsptab[1]&01740)>>5);
			tmp[2] = get_dsky_char(bits);
			bits = (dsptab[1]&037);
			tmp[3] = get_dsky_char(bits);
			bits = ((dsptab[0]&01740)>>5);
			tmp[4] = get_dsky_char(bits);
			bits = (dsptab[0]&037);
			tmp[5] = get_dsky_char(bits);
			tmp[6] = 0;
			cmc_form->textBox137->Text = tmp;
			if(upl_form != NULL){ upl_form->textBox137->Text = tmp; upl_form->textBox137->Enabled = true;}
			// WARNING LIGHTS
			if(dsptab[11]&00400){ cmc_form->label72->Enabled = TRUE; }else{ cmc_form->label72->Enabled = FALSE; }
			if(dsptab[11]&00200){ cmc_form->label75->Enabled = TRUE; }else{ cmc_form->label75->Enabled = FALSE; }
			if(dsptab[11]&00040){ cmc_form->label70->Enabled = TRUE; }else{ cmc_form->label70->Enabled = FALSE; }
			if(dsptab[11]&00010){ cmc_form->label69->Enabled = TRUE; }else{ cmc_form->label69->Enabled = FALSE; }
			if(upl_form != NULL){
				if(dsptab[11]&00400){ upl_form->label72->Enabled = TRUE; }else{ upl_form->label72->Enabled = FALSE; }
				if(dsptab[11]&00200){ upl_form->label75->Enabled = TRUE; }else{ upl_form->label75->Enabled = FALSE; }
				if(dsptab[11]&00040){ upl_form->label70->Enabled = TRUE; }else{ upl_form->label70->Enabled = FALSE; }
				if(dsptab[11]&00010){ upl_form->label69->Enabled = TRUE; }else{ upl_form->label69->Enabled = FALSE; }
			}
			break;
		// TIME2 and TIME1
		case 51: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox78->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox77->Text = msg;
			break;
		// OTHER STATE VECTOR (LIST 1,2,3)
		// LAT                (LIST 4)
		// PIPTIME            (LIST 5)
		case 52: 
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox79->Text = msg;					
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox153->Text = msg;
					cmc_form->textBox79->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// LONG  (LIST 4)
		// DELVx (LIST 5)
		case 53:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox80->Text = msg;
					cmc_form->textBox174->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox174->Text = msg;
					cmc_form->textBox80->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// ALT   (LIST 4)
		// DELVy (LIST 5)
		case 54:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox81->Text = msg;
					cmc_form->textBox173->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox173->Text = msg;
					cmc_form->textBox81->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// SPARE (LIST 4)
		// DELVz (LIST 5)
		case 55:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox82->Text = msg;
					cmc_form->textBox172->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox172->Text = msg; break;
					cmc_form->textBox82->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// SPARE (LIST 4)
		// TTE   (LIST 5)
		case 56:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox83->Text = msg;
					cmc_form->textBox175->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox175->Text = msg;
					cmc_form->textBox83->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// SPARE (LIST 4)
		// VIO   (LIST 5)
		case 57:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox84->Text = msg; 
					cmc_form->textBox176->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox176->Text = msg;
					cmc_form->textBox84->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// SPARE (LIST 4)
		// VPRED (LIST 5)
		case 58:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox85->Text = msg; 
					cmc_form->textBox177->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox177->Text = msg;
					cmc_form->textBox85->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		// DUPES OF CDUX/Y/Z AND CDUT
		case 59:
		case 60:
		// DUPES OF ADOTx/y/z
		case 61:
		case 62:
		case 63:
		// DUPES OF AK-AK2 and RCSFLAGS
		case 64:
		case 65:
		// DUPES OF THEDADx/y/z and DELCDUx (LIST 1,2,3,4)
		// ERRORx/y/z and DUPE OF THETADx   (LIST 5)
		case 66:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox111->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox110->Text = msg;
					break;
			}
			break;
		case 67:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox109->Text = msg;
					// sprintf(msg,"%05o",cmc_w1); cmc_form->textBox110->Text = msg;
					break;
			}
			break;
		// RSBBQ and Q        (LIST 1,2,3,4)
		// DUPES OF THETADy/z (LIST 5)
		case 68:
			switch(cmc_lock_type){
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox86->Text = msg; break;
				case 2:
					break;
			}
			break;
		// CADRFLSH and FAILREG          (LIST 1,2,3,4)
		// DUPES OF CMDAPMOD and PREL    (LIST 5)
		case 69: 
			switch(cmc_lock_type){
				case 2:
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox90->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox89->Text = msg;
					break;
			}
			break;
		// DUPES OF QREL,RREL (LIST 5)
		case 70: 
			switch(cmc_lock_type){
				case 2:
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox88->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox99->Text = msg;
					break;
			}
			break;
		/* ****************** LIST 5 UPBUF DUPE STARTS ************** */
		case 71: 
			switch(cmc_lock_type){
				case 2:
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox98->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox97->Text = msg;
					break;
			}
			break;
		// CDUS and PIPAx/y/z
		case 72:
			switch(cmc_lock_type){
				case 2:
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox12->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox92->Text = msg;
					break;
			}
			break;
		case 73: 
			switch(cmc_lock_type){
				case 2:
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox91->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox93->Text = msg;
					break;
			}
			break;
		// O/I/M gyro torque angles       (LIST 1)
		// ELEV,CENTANG,DELTAR            (LIST 2,3)
		// 8NN,S22LOC,FLAGWORD 10-11,RLSx (LIST 4)
		case 74:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox96->Text = msg;
					cmc_form->textBox143->Text = "XXXXX-XXXXX";
					break;
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox143->Text = msg;
					cmc_form->textBox96->Text = "XXXXX-XXXXX";
					break;
				case 2:
					break;
			}
			break;
		case 75:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox94->Text = msg;
					cmc_form->textBox142->Text = "XXXXX-XXXXX";
					break;
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox142->Text = msg;
					cmc_form->textBox94->Text = "XXXXX-XXXXX";
					break;
				case 2:
					break;
			}
			break;
		case 76:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox95->Text = msg;
					cmc_form->textBox141->Text = "XXXXX-XXXXX";
					break;
				case 3:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox141->Text = msg;
					cmc_form->textBox95->Text = "XXXXX-XXXXX";
					break;
				case 2:
					break;
			}
			break;
		// FLAGWORD 10-11 (LIST 1,3)
		// DELVEET3x      (LIST 2)
		// RLSy           (LIST 4)
		case 77: 
			switch(cmc_lock_type){
				case 1:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox87->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox100->Text = msg;
					cmc_form->textBox146->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox146->Text = msg; break;
					break;
				case 2:
					break;
			}
			break;
		// TEVENT    (LIST 1,3)
		// DELVEET3y (LIST 2)
		// RLSz      (LIST 4)
		case 78:
			switch(cmc_lock_type){
				case 1:
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox102->Text = msg;
					cmc_form->textBox145->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox145->Text = msg;
					cmc_form->textBox102->Text = "XXXXX-XXXXX";
					break;
				case 2:
					break;
			}
			break;
		// LAUNCHAZ            (LIST 1)
		// DELVEET3z           (LIST 2)
		// DUPES OF PCMD,YCMD  (LIST 3)
		// SPARE               (LIST 4)
		case 79:
			switch(cmc_lock_type){
				case 1:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox101->Text = msg;
					cmc_form->textBox144->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox144->Text = msg;
					cmc_form->textBox101->Text = "XXXXX-XXXXX";
					break;
				case 4:
					cmc_form->textBox101->Text = "XXXXX-XXXXX";
					cmc_form->textBox144->Text = "XXXXX-XXXXX";
					break;
				case 2:
					break;
			}
			break;
		// OPTMODES and HOLDFLAG
		/* ****************** LIST 5 UPBUF DUPE ENDS **************** */
		case 80: 
			switch(cmc_lock_type){
				case 2:
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox103->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox106->Text = msg;
					break;
			}
			break;
		// LEMMASS and CSMMASS
		case 81: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox105->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox104->Text = msg;
			break;
		// DAPDATR1 and DAPDATR2
		case 82: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox107->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox108->Text = msg;
			break;
		// ERRORx/y/z and THETADx   (LIST 1,2,3,4)
		// DUPE OF ROLLTM and ROLLC (LIST 5)
		case 83: 
			switch(cmc_lock_type){
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox165->Text = msg;
					break;
				case 1:
				case 3:
				case 4:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox111->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox110->Text = msg;
					// cmc_form->textBox165->Text = "XXXXX-XXXXX"; // Gets blanked earlier.
					break;
			}
			break;
		// OPTMODES and HOLDFLAG
		case 84: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox109->Text = msg;
			//sprintf(msg,"%05o",cmc_w1); cmc_form->textBox108->Text = msg;
			break;
		// DESIRED R/P/Y RATES
		case 85:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox113->Text = msg; break;
		case 86:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox112->Text = msg; break;
		case 87:
			sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox114->Text = msg; break;
		// REDOCTR and THETAD
		case 88: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox115->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox118->Text = msg;
			break;
		case 89: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox117->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox116->Text = msg;
			break;
		// IMODES30 and IMODES33
		case 90: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox119->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox120->Text = msg;
			break;
		// CH11-12
		case 91: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox121->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox122->Text = msg;
			break;
		// CH13-14
		case 92: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox123->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox124->Text = msg;
			break;
		// CH30-31
		case 93: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox125->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox126->Text = msg;
			break;
		// CH32-33
		case 94: 
			sprintf(msg,"%05o",cmc_w0); cmc_form->textBox128->Text = msg;
			sprintf(msg,"%05o",cmc_w1); cmc_form->textBox127->Text = msg;
			break;
		// DUPES OF DSPTAB                                    (LIST 1)
		// RTHETA,LATSPL,LNGSPL,VPRED,GAMMAEI,FLAGWORD 10-11  (LIST 2)
		// VGTIGx/y/z and SPARES                              (LIST 3)
		// SPARES                                             (LIST 4)
		// RSBBQ,CADRFLSH,FAILREG,FLAGWORD 10-11,GAMMAEI,RTGO (LIST 5)
		case 95:
			switch(cmc_lock_type){
				case 1:
					cmc_form->textBox147->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox86->Text = msg;
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox147->Text = msg;
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox48->Text = msg;
					cmc_form->textBox147->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		case 96:
			switch(cmc_lock_type){
				case 1:
					cmc_form->textBox148->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox90->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox89->Text = msg;
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox148->Text = msg;
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox47->Text = msg;
					cmc_form->textBox148->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		case 97:
			switch(cmc_lock_type){
				case 1:
					cmc_form->textBox149->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox88->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox99->Text = msg;
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox149->Text = msg;
					break;
				case 4:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox46->Text = msg;
					cmc_form->textBox149->Text = "XXXXX-XXXXX";
					break;
			}
			break;
		case 98:
			switch(cmc_lock_type){
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox98->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox97->Text = msg;
					break;
				case 1:
				case 4:
					cmc_form->textBox150->Text = "XXXXX-XXXXX";
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox150->Text = msg;
					break;
			}
			break;
		case 99:
			switch(cmc_lock_type){
				case 1:
				case 4:
					cmc_form->textBox151->Text = "XXXXX-XXXXX";
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox87->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox100->Text = msg;
					break;
				case 3:
					sprintf(msg,"%05o-%05o",cmc_w0,cmc_w1); cmc_form->textBox151->Text = msg;
					break;
			}
			break;
		case 100:
			switch(cmc_lock_type){
				case 1:
				case 4:
					break;
				case 3:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox87->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox100->Text = msg;
					break;
				case 2:
					sprintf(msg,"%05o",cmc_w0); cmc_form->textBox151->Text = msg;
					sprintf(msg,"%05o",cmc_w1); cmc_form->textBox179->Text = msg;
					break;
			}
			break;
	}
}

// Translate bits into DSKY display character
char Form1::get_dsky_char(unsigned int bits){
	switch(bits){
		case 025:
			return('0');
		case 003:
			return('1');
		case 031:
			return('2');
		case 033:
			return('3');
		case 017:
			return('4');
		case 036:
			return('5');
		case 034:
			return('6');
		case 023:
			return('7');
		case 035:
			return('8');
		case 037:
			return('9');
		case 000:
			return('_');
		default:
			return('X');
	}
}

void Form1::parse_lbr(unsigned char data, int bytect)
{
	switch(bytect){
		case 0: // SYNC 1
			if(data != 05){ end_lbr(); lock_type = 0; cmc_lock_type = 0; }
			break;

		case 1: // SYNC 2
			if(data != 0171){ end_lbr(); lock_type = 0; cmc_lock_type = 0; }
			break;

		case 2: // SYNC 3
			if(data != 0267){ end_lbr(); lock_type = 0; cmc_lock_type = 0; }
			break;

		case 3: // FRAME COUNT
			framect = data&077;
			break;

		case 4:					
			switch(framect){
			case 0: // 11A1 ECS: SUIT MANF ABS PRESS
				display( data, 11, TLM_A, 1 );
				break;

			case 1: // 11A109 EPS: BAT B CURR
				display( data, 11, TLM_A, 109 );
				break;

			case 2: // 11A46 RCS: SM HE MANF C PRESS
				display( data, 11, TLM_A, 46 );
				break;

			case 3: // 11A154 CMI: SCE NEG SUPPLY VOLTS
				display( data, 11, TLM_A, 154 );
				break;

			case 4: // 11A91 EPS: BAT BUS A VOLTS
				display( data, 11, TLM_A, 91 );
				break;
			}
			break;

		case 5:
			switch(framect){
			case 0: // 11A2 ECS: SUIT COMP DELTA P
				display( data, 11, TLM_A, 2 );
				break;

			case 1: // 11A110 EPS: BAT C CURR
				display( data, 11, TLM_A, 110 );
				break;

			case 2: // 11A47 EPS: LM HEATER CURRENT
				display( data, 11, TLM_A, 47 );
				break;

			case 3: // 11A155 RCS: CM HE TK A TEMP
				display( data, 11, TLM_A, 155 );
				break;

			case 4: // 11A92 RCS: SM FU MANF A PRESS
				display( data, 11, TLM_A, 92 );
				break;
			}
			break;

		case 6:
			switch(framect)
			{
				case 0: // 11A3 ECS: GLY PUMP OUT PRESS
					display( data, 11, TLM_A, 3 );
					break;

				case 1: // 11A111 ECS: SM FU MANF C PRESS
					display( data, 11, TLM_A, 111 );
					break;

				case 2: // 11A48 PCM HI LEVEL 85 PCT REF
					display( data, 11, TLM_A, 48 );
					break;

				case 3: // 11A156 CM HE TK B TEMP
					display( data, 11, TLM_A, 156 );
					break;

				case 4: // 11A93 BAT BUS B VOLTS
					display( data, 11, TLM_A, 93 );
					break;
			}
			break;

		case 7:
			switch(framect)
			{
				case 0: // 11A4 ECS SURGE TANK PRESS
					display( data, 11, TLM_A, 4 );
					break;

				case 1: // 11A112 SM FU MANF D PRESS
					display( data, 11, TLM_A, 112 );
					break;

				case 2: // 11A49 PC HI LEVEL 15 PCT REF
					display( data, 11, TLM_A, 49 );
					break;

				case 3: // 11A157 SEC GLY PUMP OUT PRESS
					display( data, 11, TLM_A, 157 );
					break;

				case 4: // 11A94 SM FU MANF B PRESS
					display( data, 11, TLM_A, 94);
					break;
			}
			break;

		case 8:
		case 28: // CMC DATA WORD
			cmc_w0 = data&0177;
			cmc_w0 <<= 8;
			break;

		case 9:
		case 29: // CMC DATA WORD
			cmc_w0 |= data&0377;
			break;

		case 10:
		case 30: // CMC DATA WORD
			cmc_w1 = data&0177;
			cmc_w1 <<= 8;
			break;

		case 11:
		case 31: // CMC DATA WORD
			cmc_w1 |= data&0377;
			break;

		case 12:
		case 32: // CMC DATA WORD
			parse_cmc();
			break;

		case 13: // 51DP2 UP-DATA-LINK VALIDITY BITS (4 BITS)
		case 33:
			break;

	}
}

// Service Thread
void Form1::CommThread(){
	int bytesRecv = SOCKET_ERROR;
	unsigned char recvbuf[1024];
	char msg[256];
	int die=0;

	int frame_count = 0;
	int word_addr = 0;
	int byte_offset = 0;
	int bytect = 0;

	lock_type = 0;   cmc_lock_type = 0;
	frame_addr = 0;	 cmc_frame_addr = 0;
	framect = 0;     cmc_framect = 0;
	while(!die){
		bytesRecv = recv( m_socket, (char *)recvbuf, 1024, 0 );
		if(bytesRecv == SOCKET_ERROR){
			sprintf(msg,"Failed to read, Error %ld",WSAGetLastError());
			closesocket(m_socket);
			StatusBox->Text = msg;
			ConnectBtn->Text = "Connect";
			conn_status = 0;
			return;
		}else{		
			// sprintf(msg,"Got %d bytes",bytesRecv);
			StatusBox->Text = msg;
			byte_offset = 0;
			while(byte_offset < bytesRecv){
				switch(lock_type){
					case 0: // OUT SYNC 0
						StatusBox->Text = "LOCK/OUT FRAM/000 WORD/000";
						bytect = 0;
						if(recvbuf[byte_offset] == 05){ // Sync char 1 recieved
							lock_type = 1;
						}
						break;

					case 1: // OUT SYNC 1
						StatusBox->Text = "LOCK/OUT FRAM/000 WORD/001";
						if(recvbuf[byte_offset] == 0171){ // Sync char 2 recieved
							lock_type = 2;
						}else{
							lock_type = 0;
						}
						break;

					case 2: // OUT SYNC 2
						StatusBox->Text = "LOCK/OUT FRAM/000 WORD/002";
						if(recvbuf[byte_offset] == 0267){ // Sync char 3 recieved
							lock_type = 3;
							bytect = 3; // Start at byte 3
						}else{
							lock_type = 0;
						}
						break;

					case 3: // CHECK FOR LBR SYNC 1
						if(bytect == 40){
							if(recvbuf[byte_offset] == 05){ // Sync char 1 recieved
								lock_type = 4; // Check LBR SYNC 2
							}else{
								// Check for HBR instead
								sprintf(msg,"FAILED AT LB1 - GOT %o",recvbuf[byte_offset]);
								StatusBox->Text = msg;
								lock_type = 6;
								bytect++;
								break;
							}
						}else{
							if(bytect > 100){ bytect = 0; lock_type=0; } // Start over 
						}
						bytect++;
						sprintf(msg,"LOCK/OUT FRAM/000 WORD/%.3d LB1",bytect);
						StatusBox->Text = msg;
						break;

					case 4: // CHECK FOR LBR SYNC 2
						if(recvbuf[byte_offset] == 0171){ // Sync char 2 recieved
							lock_type = 5; // Check LBR SYNC 3
						}else{
							// Check for HBR instead
							lock_type = 6;
						}
						bytect++;
						sprintf(msg,"LOCK/OUT FRAM/000 WORD/%.3d LB2",bytect);
						StatusBox->Text = msg;
						break;

					case 5: // CHECK FOR LBR SYNC 3
						if(recvbuf[byte_offset] == 0267){ // Sync char 2 recieved
							lock_type = 10; // LBR SYNC
							bytect = 3;
							break;
						}else{
							// Check for HBR instead
							lock_type = 6;
						}
						bytect++;
						sprintf(msg,"LOCK/OUT FRAM/000 WORD/%.3d LB3",bytect);
						StatusBox->Text = msg;
						break;
						
					case 6: // CHECK FOR HBR SYNC 1
						if(bytect == 128){
							if(recvbuf[byte_offset] == 05){ // Sync char 1 recieved
								lock_type = 7; // Check HBR SYNC 2
							}else{
								// Fail
								lock_type = 0;
								break;
							}
						}else{
							if(bytect > 200){ bytect = 0; lock_type=0; } // Start over 
						}
						bytect++;
						sprintf(msg,"LOCK/OUT FRAM/000 WORD/%.3d HB1",bytect);
						StatusBox->Text = msg;
						break;

					case 7: // CHECK FOR HBR SYNC 2
						if(recvbuf[byte_offset] == 0171){ // Sync char 2 recieved
							lock_type = 8; // Check HBR SYNC 3
						}else{
							// Fail
							lock_type = 0;
							break;
						}
						bytect++;
						sprintf(msg,"LOCK/OUT FRAM/000 WORD/%.3d HB2",bytect);
						StatusBox->Text = msg;
						break;

					case 8: // CHECK FOR HBR SYNC 3
						if(recvbuf[byte_offset] == 0267){ // Sync char 3 recieved
							lock_type = 20; // HBR SYNC
							bytect = 3;
							break;
						}else{
							// Fail
							lock_type = 0;
							break;
						}
						bytect++;
						sprintf(msg,"LOCK/OUT FRAM/000 WORD/%.3d HB3",bytect);
						StatusBox->Text = msg;
						break;

					case 9: // DEBUG - STALL
						break;

					case 10: // LBR
						parse_lbr(recvbuf[byte_offset],bytect);
						bytect++;
						if(bytect > 39){
							bytect = 0;
							sprintf(msg,"LOCK/LBR FRAM/%.3d",framect,bytect);
							StatusBox->Text = msg;
							frame_addr++;
							if(frame_addr > 4){
								frame_addr = 0;
							}
						}
						break;

					case 20: // HBR
						parse_hbr(recvbuf[byte_offset],bytect);
						bytect++;
						if(bytect > 127){
							bytect = 0;
							sprintf(msg,"LOCK/HBR FRAM/%.3d",framect,bytect);
							StatusBox->Text = msg;
							frame_addr++;
							if(frame_addr > 4){
								frame_addr = 0;
							}
						}
						break;
				}
				byte_offset++;
			}
		}
	}
}

void Form1::ShowStructures(){
	if(str_form == NULL){
		str_form = new StructuresForm();
	}
	str_form->Show();
}

void Form1::ShowEPS(){
	if(eps_form == NULL){
		eps_form = new EPSForm();
	}
	eps_form->Show();
}

void Form1::ShowELS(){
	if(els_form == NULL){
		els_form = new ELSForm();
	}
	els_form->Show();
}

void Form1::ShowECS(){
	if(ecs_form == NULL){
		ecs_form = new ECSForm();
	}
	ecs_form->Show();
}

void Form1::ShowGNC(){
	if(gnc_form == NULL){
		gnc_form = new GNCForm();
	}
	gnc_form->Show();
}

void Form1::ShowSCS(){
	if(scs_form == NULL){
		scs_form = new SCSForm();
	}
	scs_form->Show();
}

void Form1::ShowSPS(){
	if(sps_form == NULL){
		sps_form = new SPSForm();
	}
	sps_form->Show();
}

void Form1::ShowCrew(){
	if(crw_form == NULL){
		crw_form = new CrewForm();
	}
	crw_form->Show();
}

void Form1::ShowTelecom(){
	if(tcm_form == NULL){
		tcm_form = new TelecomForm();
	}
	tcm_form->Show();
}

void Form1::ShowCMC(){
	if(cmc_form == NULL){
		cmc_form = new CMCForm();
	}
	cmc_form->Show();
}

void Form1::ShowUplink(){
	// The uplink form uses the CMC form's data.
	if(cmc_form == NULL){
		cmc_form = new CMCForm();
		cmc_form->Hide();
	}
	if(upl_form == NULL){
		upl_form = new UplinkForm();
		upl_form->m_socket = &m_socket;
	}
	upl_form->Show();
}

// Uplink Helpers
void Form1::cmc_uplink_word(char * data){
	upl_form->send_agc_key(data[0]);
	upl_form->send_agc_key(data[1]);
	upl_form->send_agc_key(data[2]);
	upl_form->send_agc_key(data[3]);
	upl_form->send_agc_key(data[4]);
	upl_form->send_agc_key('E');
}

// Process uplink from uplink form
// This is here because passing pointers to arrays in Managed C++ is a royal pain in the ass.

void Form1::cmc_uplink_process(){

	char tmp[256];
	unsigned int tmi=0;

	if(upl_form == NULL){ return; }
	switch(upl_form->cmc_uplink_state){
		case 0:  // IDLE
			return;

		case 1: // COMMAND START
			switch(upl_form->comboBox1->SelectedIndex){
				case 1:  // V70 LIFTOFF TIME INCREMENT
					upl_form->cmc_uplink_state = 10;
					break;

				case 2: // V71 CONTIGUOUS ERASABLE
				case 3: // V71 STATE VECTOR
				case 4: // V71 DESIRED REFSMMAT
				case 5: // V71 CMC REFSMMAT
				case 6: // V71 EXTERNAL DV
				case 7: // V71 ENTRY DATA + EXTERNAL DV
				case 8: // V71 ENTRY DATA
				case 9: // V71 LAMBERT TARGET
				case 10: // V71 LUNAR LANDING SITE
					upl_form->cmc_uplink_state = 20;
					break;

				case 11: // V72 NONCONTIGUOUS ERASABLE UPDATE
					upl_form->cmc_uplink_state = 70;
					break;

				case 12: // V73 OCTAL CLOCK INCREMENT
					upl_form->cmc_uplink_state = 72;
					break;
			}
			break;

		case 10: // VERB 70 START
			upl_form->textBox26->Text = "SENDING V70";
			upl_form->send_agc_key('V');
			upl_form->send_agc_key('7');
			upl_form->send_agc_key('0');
			upl_form->send_agc_key('E');
			upl_form->cmc_uplink_state++;
			break;

		case 11: // V70 - VERIFY P27
			upl_form->textBox26->Text = "AWAITING P27";
			if(cmc_upcount != 1){ break; }
			if(cmc_lock_type != 2){ break; }
			if(cmc_upverb != 0){ break; }
			if(cmc_compnumb != 2){ break; }
			upl_form->cmc_uplink_state++;
			break;

		case 12: // TRANSMIT WORD 0
			upl_form->textBox26->Text = "SENDING WORD 0";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox6->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);
			sprintf(tmp,"%05o",tmi);
			upl_form->textBox6->Text = tmp;
			cmc_uplink_word(tmp);
			upl_form->cmc_uplink_state++;
			break;

		case 13: // VERIFY WORD 0
			sprintf(tmp,"VERIFYING WORD 0 VS %05o",cmc_upbuff[0]);
			upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox6->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);
			if(tmi != cmc_upbuff[0]){ break; }
			upl_form->cmc_uplink_state++;
			break;

		case 14: // TRANSMIT WORD 1
			upl_form->textBox26->Text = "SENDING WORD 1";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox7->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);
			sprintf(tmp,"%05o",tmi);
			upl_form->textBox7->Text = tmp;
			cmc_uplink_word(tmp);
			upl_form->cmc_uplink_state++;
			break;

		case 15: // VERIFY WORD 1
			sprintf(tmp,"VERIFYING WORD 1 VS %05o",cmc_upbuff[1]);
			upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox7->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);
			if(tmi != cmc_upbuff[0]){ break; }
			upl_form->cmc_uplink_state++;
			break;

		case 16: // COMMIT V70
			upl_form->textBox26->Text = "COMPLETED";
			upl_form->send_agc_key('V');
			upl_form->send_agc_key('3');
			upl_form->send_agc_key('3');
			upl_form->send_agc_key('E');
			// Re-enable controls
			upl_form->button64->Enabled = true;
			upl_form->comboBox1->Enabled = true;
			upl_form->textBox26->Enabled = false;
			// Go to idle
			upl_form->cmc_uplink_state = 0;
			break;			

		case 20: // VERB 71 START
			upl_form->textBox26->Text = "SENDING V71";
			upl_form->send_agc_key('V');
			upl_form->send_agc_key('7');
			upl_form->send_agc_key('1');
			upl_form->send_agc_key('E');
			upl_form->cmc_uplink_state++;
			break;

		case 21: // V71 - VERIFY P27
			upl_form->textBox26->Text = "AWAITING P27";
			if(cmc_upcount != 1){ break; }
			if(cmc_lock_type != 2){ break; }
			if(cmc_upverb != 1){ break; }
			upl_form->cmc_uplink_state++;
			break;

		case 22: // TRANSMIT INDEX
			upl_form->textBox26->Text = "SENDING INDEX";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox5->Text));
			tmi = (unsigned int)strtol(tmp,NULL,10);
			sprintf(tmp,"%02o",tmi);
			cmc_upindex = tmi; // Save index
			// Don't write back the index, leave it decimal
			upl_form->send_agc_key(tmp[0]);
			upl_form->send_agc_key(tmp[1]);
			upl_form->send_agc_key('E');
			upl_form->cmc_uplink_state++;
			break;
		case 23: // VERIFY INDEX
			sprintf(tmp,"VERIFYING INDEX %02o VS %02o",cmc_upindex,cmc_upbuff[0]);
			upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox5->Text));
			tmi = (unsigned int)strtol(tmp,NULL,10);
			if(tmi != cmc_upbuff[0]){ break; }
			upl_form->cmc_uplink_state++;
			break;

		case 24: // TRANSMIT WORD 0
			upl_form->textBox26->Text = "SENDING WORD 0";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox6->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox6->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 25: // VERIFY WORD 0
			sprintf(tmp,"VERIFYING WORD 0 VS %05o",cmc_upbuff[1]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox6->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); if(tmi != cmc_upbuff[1]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 26: // TRANSMIT WORD 1
			upl_form->textBox26->Text = "SENDING WORD 1";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox7->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	sprintf(tmp,"%05o",tmi); upl_form->textBox7->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++; break;
		case 27: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 1 VS %05o",cmc_upbuff[2]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox7->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); if(tmi != cmc_upbuff[2]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 28: // TRANSMIT WORD 2
			if(cmc_upindex == 3){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 2";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox9->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox9->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 29: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 2 VS %05o",cmc_upbuff[3]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox9->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); if(tmi != cmc_upbuff[3]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 30: // TRANSMIT WORD 3
			if(cmc_upindex == 4){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 3";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox8->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox8->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 31: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 3 VS %05o",cmc_upbuff[4]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox8->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[4]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 32: // TRANSMIT WORD 4
			if(cmc_upindex == 5){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 4";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox11->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox11->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 33: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 4 VS %05o",cmc_upbuff[5]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox11->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[5]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 34: // TRANSMIT WORD 5
			if(cmc_upindex == 6){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 5";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox10->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox10->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 35: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 5 VS %05o",cmc_upbuff[6]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox10->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[6]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 36: // TRANSMIT WORD 6
			if(cmc_upindex == 7){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 6";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox13->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox13->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 37: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 6 VS %05o",cmc_upbuff[7]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox13->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[7]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 38: // TRANSMIT WORD 7
			if(cmc_upindex == 8){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 7";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox12->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox12->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 39: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 7 VS %05o",cmc_upbuff[8]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox12->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[8]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 40: // TRANSMIT WORD 8
			if(cmc_upindex == 9){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 8";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox15->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox15->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 41: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 8 VS %05o",cmc_upbuff[9]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox15->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[9]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 42: // TRANSMIT WORD 9
			if(cmc_upindex == 10){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 9";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox14->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox14->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 43: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 9 VS %05o",cmc_upbuff[10]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox14->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[10]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 44: // TRANSMIT WORD 10
			if(cmc_upindex == 11){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 10";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox25->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox25->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 45: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 10 VS %05o",cmc_upbuff[11]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox25->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[11]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 46: // TRANSMIT WORD 11
			if(cmc_upindex == 12){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 11";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox24->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox24->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 47: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 11 VS %05o",cmc_upbuff[12]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox24->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[12]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 48: // TRANSMIT WORD 12
			if(cmc_upindex == 13){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 12";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox23->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox23->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 49: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 12 VS %05o",cmc_upbuff[13]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox23->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[13]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 50: // TRANSMIT WORD 13
			if(cmc_upindex == 14){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 13";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox22->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox22->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 51: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 13 VS %05o",cmc_upbuff[14]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox22->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[14]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 52: // TRANSMIT WORD 14
			if(cmc_upindex == 15){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 14";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox21->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox21->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 53: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 14 VS %05o",cmc_upbuff[15]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox21->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[15]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 54: // TRANSMIT WORD 15
			if(cmc_upindex == 16){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 15";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox20->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox20->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 55: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 15 VS %05o",cmc_upbuff[16]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox20->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[16]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 56: // TRANSMIT WORD 16
			if(cmc_upindex == 17){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 16";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox19->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox19->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 57: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 16 VS %05o",cmc_upbuff[17]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox19->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[17]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 58: // TRANSMIT WORD 17
			if(cmc_upindex == 18){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 17";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox18->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox18->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 59: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 17 VS %05o",cmc_upbuff[18]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox18->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[18]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 60: // TRANSMIT WORD 18
			if(cmc_upindex == 19){ upl_form->cmc_uplink_state = 62; break; } // DONE
			upl_form->textBox26->Text = "SENDING WORD 18";
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox17->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8); sprintf(tmp,"%05o",tmi); upl_form->textBox17->Text = tmp;
			cmc_uplink_word(tmp); upl_form->cmc_uplink_state++;	break;
		case 61: // VERIFY WORD
			sprintf(tmp,"VERIFYING WORD 18 VS %05o",cmc_upbuff[19]); upl_form->textBox26->Text = tmp;
			sprintf(tmp,"%s",System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(upl_form->textBox17->Text));
			tmi = (unsigned int)strtol(tmp,NULL,8);	if(tmi != cmc_upbuff[19]){ break; }
			upl_form->cmc_uplink_state++; break;

		case 62: // COMMIT 
			upl_form->textBox26->Text = "COMPLETED";
			upl_form->send_agc_key('V');
			upl_form->send_agc_key('3');
			upl_form->send_agc_key('3');
			upl_form->send_agc_key('E');
			// Re-enable controls
			upl_form->button64->Enabled = true;
			upl_form->comboBox1->Enabled = true;
			upl_form->textBox26->Enabled = false;
			// Go to idle
			upl_form->cmc_uplink_state = 0;
			break;			

		case 70: // VERB 72 START
			upl_form->textBox26->Text = "SENDING V72";
			upl_form->send_agc_key('V');
			upl_form->send_agc_key('7');
			upl_form->send_agc_key('2');
			upl_form->send_agc_key('E');
			upl_form->cmc_uplink_state++;
			break;

		case 71: // V72 - VERIFY P27
			upl_form->textBox26->Text = "AWAITING P27";
			if(cmc_upcount != 1){ break; }
			if(cmc_lock_type != 2){ break; }
			if(cmc_upverb != 2){ break; }
			upl_form->cmc_uplink_state = 22; // CONTINUE HERE
			break;

		case 72: // VERB 73 START
			upl_form->textBox26->Text = "SENDING V73";
			upl_form->send_agc_key('V');
			upl_form->send_agc_key('7');
			upl_form->send_agc_key('3');
			upl_form->send_agc_key('E');
			upl_form->cmc_uplink_state++;
			break;

		case 73: // V73 - VERIFY P27
			upl_form->textBox26->Text = "AWAITING P27";
			if(cmc_upcount != 1){ break; }
			if(cmc_lock_type != 2){ break; }
			if(cmc_upverb != 3){ break; }
			if(cmc_compnumb != 2){ break; }
			upl_form->cmc_uplink_state = 12; // CONTINUE HERE
			break;

	}
}@


1.16
log
@More telemetry and rationalisation.
@
text
@d25 3
d174 1
a174 1
void Form1::showPercentage( textDisplay *tb, unsigned char data )
d177 1
a177 1
	sprintf(msg, "%05.2f %%", unscale_data(data, 0, 100));
d181 5
d372 5
a376 1
			case 26:
d382 1
a382 1
					showPSIA( sps_form->s10A27, data,  0.0, 5000.0 );							
d389 15
a403 3
					value = unscale_data(data, 0, 60);
					sprintf(msg,"%06.2f %%",value);
					showValue( sps_form->s10A28, msg );						
d410 15
a424 3
					value = unscale_data(data, 0, 60);
					sprintf(msg,"%06.2f %%",value);
					showValue( sps_form->s10A31, msg );						
d435 42
d484 28
d519 28
d554 7
d568 28
d603 16
@


1.15
log
@More telemetry.
@
text
@d25 3
d164 1
a164 1
void Form1::showValue( System::Windows::Forms::TextBox *tb, char *msg )
d171 54
d236 9
d248 22
a269 3
					value = unscale_data(data, 0.05, 0.25);
					sprintf(msg,"%04.3f PSIA",value);
					showValue( ecs_form->s10A4, msg );				
d276 29
a304 3
					value = unscale_data(data, 0, 5000);
					sprintf(msg,"%04.0f PSIA",value);
					showValue( sps_form->s10A10, msg );								
d311 15
a325 3
					value = unscale_data(data, -100, 200);
					sprintf(msg,"%+07.2f �F",value);
					showValue( sps_form->s10A16, msg );								
d332 15
a346 3
					value = unscale_data(data, 0, 300);
					sprintf(msg,"%+07.2f �F",value);
					showValue( sps_form->s10A19, msg );								
d353 18
a370 3
					value = unscale_data(data, -100, 300);
					sprintf(msg,"%+07.2f �F",value);
					showValue( els_form->s10A22, msg );				
d378 1
a378 1
					sprintf(msg,"%06.2f %",value);
d387 1
a387 1
					sprintf(msg,"%06.2f %",value);
d395 1
a395 3
					value = unscale_data(data, 0, 200);
					sprintf(msg,"%+07.2f �F",value);
					showValue( sps_form->s10A37, msg );								
d402 1
a402 3
					value = unscale_data(data, 0, 600);
					sprintf(msg,"%+07.2f �F",value);
					showValue( sps_form->s10A49, msg );								
d409 1
a409 3
					value = unscale_data(data, -109, 140);
					sprintf(msg,"%+07.2f �F",value);
					showValue( tcm_form->s10A61, msg );								
d416 1
a416 3
					value = unscale_data(data, -50, 300);
					sprintf(msg,"%+07.2f �F",value);
					showValue( eps_form->s10A67, msg );				
d423 1
a423 3
					value = unscale_data(data, -260, 600);
					sprintf(msg,"%+07.2f �F",value);
					showValue( str_form->s10A70, msg );				
d430 1
a430 3
					value = unscale_data(data, -260, 600);
					sprintf(msg,"%+07.2f �F",value);
					showValue( str_form->s10A79, msg );				
d446 1
a446 3
					value = unscale_data(data, 32, 248);
					sprintf(msg,"%+07.2f �F",value);
					showValue( eps_form->s10A88, msg );				
d462 1
a462 3
					value = unscale_data(data, 35, 100);
					sprintf(msg,"%+07.2f �F",value);
					showValue( ecs_form->s10A100, msg );				
d469 1
a469 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A106, msg );						
d476 1
a476 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A107, msg );						
d483 1
a483 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A108, msg );						
d490 1
a490 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A109, msg );						
d497 1
a497 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A110, msg );						
d504 1
a504 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A111, msg );						
d511 1
a511 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A112, msg );						
d518 1
a518 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A113, msg );						
d525 1
a525 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A115, msg );						
d532 1
a532 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A116, msg );						
d539 1
a539 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A118, msg );						
d546 1
a546 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A119, msg );						
d553 1
a553 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A121, msg );						
d560 1
a560 3
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.1f",value);
					showValue( crw_form->s10A122, msg );						
d564 6
d579 1
a579 3
					value = unscale_data(data, 0, 17);
					sprintf(msg,"%03.1f PSIA",value);
					showValue( ecs_form->s11A1, msg );				
d1803 18
@


1.14
log
@Moved all decoded data to new display() method.
@
text
@d25 3
d177 281
a457 1
			switch(ccode){
d465 1
a465 1
				if( ecs_form != NULL )
d537 1
a537 1
				if (sps_form != NULL)
d546 1
a546 1
				if (sps_form != NULL)
@


1.13
log
@More LBR support.
@
text
@d25 3
d244 54
d361 62
d487 45
d586 36
d639 45
d762 35
d847 1
a847 1
	double value;
d1157 1
a1157 7
				if (eps_form != NULL)
				{
					value = unscale_data(data, 0, 1.6);
					sprintf(msg,"%04.3f PPH",value);
					eps_form->s11A80->Enabled = TRUE;
					eps_form->s11A80->Text = msg;						
				} 
d1165 1
a1165 7
				if (sps_form != NULL)
				{
					value = unscale_data(data, 0, 300);
					sprintf(msg,"%04.0f PSIA",value);
					sps_form->s11A152->Enabled = TRUE;
					sps_form->s11A152->Text = msg;						
				} 
d1173 19
a1191 19
				case 0: // 11A9 PYRO BUS A VOLTS
					if (els_form != NULL)
					{
						value = unscale_data(data, 0, 40);
						sprintf(msg,"%04.2f V",value);
						els_form->s11A9->Enabled = TRUE;
						els_form->s11A9->Text = msg;						
					} 
					break;

				case 2: // 11A81 FC 2 O2 FLOW
					if (eps_form != NULL)
					{
						value = unscale_data(data, 0, 1.6);
						sprintf(msg,"%04.3f PPH",value);
						eps_form->s11A81->Enabled = TRUE;
						eps_form->s11A81->Text = msg;						
					} 
					break;
d1195 12
d1211 1
d1215 1
d1220 1
d1224 1
d1232 19
a1250 49
				case 0: // 11A10 SPS HE TK PRESS
					if (sps_form != NULL)
					{
						value = unscale_data(data, 0, 5000);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A10->Enabled = TRUE;
						sps_form->s11A10->Text = msg;								
					} 
					break;

				case 1: // 11A46 SM HE MANF C PRESS
					if(sps_form != NULL)
					{
						value = unscale_data(data,0,400);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A46->Enabled = TRUE;
						sps_form->s11A46->Text = msg;						
					} 
					break;

				case 2: // 11A82 FC 3 O2 FLOW
					if (eps_form != NULL)
					{
						value = unscale_data(data, 0, 1.6);
						sprintf(msg,"%04.3f PPH",value);
						eps_form->s11A82->Enabled = TRUE;
						eps_form->s11A82->Text = msg;						
					} 
					break;

				case 3: // 11A118 SEC EVAP OUT LIQ TEMP
					if ( ecs_form != NULL )
					{
						value = unscale_data(data, 25, 75);
						sprintf(msg,"%05.2f �F",value);
						ecs_form->s11A118->Enabled = TRUE;
						ecs_form->s11A118->Text = msg;						
					} 
					break;

				case 4: // 11A154 SCE NEG SUPPLY VOLTS
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, -30, 0);
						sprintf(msg,"%04.2f V",value);
						tcm_form->s11A154->Enabled = TRUE;
						tcm_form->s11A154->Text = msg;						
					} 
					break;
d1257 19
a1275 38
				case 0: // 11A11 SPS OX TK PRESS
					if (sps_form != NULL)
					{
						value = unscale_data(data, 0, 250);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A11->Enabled = TRUE;
						sps_form->s11A11->Text = msg;								
					}
					break;

				case 1: // 11A47 LM HEATER CURRENT
					if (eps_form != NULL)
					{
						value = unscale_data(data, 0, 10);
						sprintf(msg,"%05.2f A",value);
						eps_form->s11A47->Enabled = TRUE;
						eps_form->s11A47->Text = msg;						
					}

				case 3: // 11A119 SENSOR EXCITATION 5V
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, 0, 9);
						sprintf(msg,"%04.2f V",value);
						tcm_form->s11A119->Enabled = TRUE;
						tcm_form->s11A119->Text = msg;						
					}
					break;

				case 4: // 11A155 CM HE TK A TEMP
					if (sps_form != NULL)
					{
						value = unscale_data(data, 0, 300);
						sprintf(msg,"%05.2f �F",value);
						sps_form->s11A155->Enabled = TRUE;
						sps_form->s11A155->Text = msg;								
					}
					break;
d1282 19
a1300 49
				case 0: // 11A12 SPS FU TK PRESS
					if (sps_form != NULL)
					{
						value = unscale_data(data, 0, 250);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A12->Enabled = TRUE;
						sps_form->s11A12->Text = msg;								
					}
					break;

				case 1: // 11A48 PCM HI LEVEL 85 PCT REF
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, 0, 5);
						sprintf(msg,"%04.2f V",value);
						tcm_form->s11A48->Enabled = TRUE;
						tcm_form->s11A48->Text = msg;						
					}
					break;

				case 2: // 11A84 FC 2 CUR
					if ( eps_form != NULL )
					{
						value = unscale_data(data,0,100);
						sprintf(msg,"%03.2f A",value);
						eps_form->s11A84->Enabled = TRUE;
						eps_form->s11A84->Text = msg;						
					}
					break;

				case 3: // 11A120 SENSOR EXCITATION 10V
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, 0, 15);
						sprintf(msg,"%04.2f V",value);
						tcm_form->s11A120->Enabled = TRUE;
						tcm_form->s11A120->Text = msg;						
					}
					break;

				case 4: // 11A156 CM HE TK B TEMP
					if (sps_form != NULL)
					{
						value = unscale_data(data, 0, 300);
						sprintf(msg,"%05.2f �F",value);
						sps_form->s11A156->Enabled = TRUE;
						sps_form->s11A156->Text = msg;								
					}
					break;
d1307 19
a1325 49
				case 0: // 11A13 GLY ACCUM QTY
					if ( ecs_form != NULL )
					{
						value = unscale_data(data, 0, 100);
						sprintf(msg,"%05.2f %",value);
						ecs_form->s11A13->Enabled = TRUE;
						ecs_form->s11A13->Text = msg;						
					}
					break;

				case 1: // 11A49 PCM LO LEVEL 15 PCT REF
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, 0, 1);
						sprintf(msg,"%04.2f V",value);
						tcm_form->s11A49->Enabled = TRUE;
						tcm_form->s11A49->Text = msg;						
					}
					break;

				case 2: // 11A85 FC 3 CUR
					if ( eps_form != NULL )
					{
						value = unscale_data(data,0,100);
						sprintf(msg,"%03.2f A",value);
						eps_form->s11A85->Enabled = TRUE;
						eps_form->s11A85->Text = msg;						
					}
					break;

				case 3: // 11A121 USB RCVR AGC VOLTAGE
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, -130, -50);
						sprintf(msg,"%+04.1f DBM",value);
						tcm_form->s11A121->Enabled = TRUE;
						tcm_form->s11A121->Text = msg;						
					}
					break;

				case 4: // 11A157 SEC GLY PUMP OUT PRESS
					if ( ecs_form != NULL )
					{
						value = unscale_data(data, 0, 60);
						sprintf(msg,"%04.0f PSIG",value);
						ecs_form->s11A157->Enabled = TRUE;
						ecs_form->s11A157->Text = msg;						
					}
					break;
d1331 1
a1331 7
			if ( gnc_form != NULL )
			{
				value = unscale_data(data, -2, 10);
				sprintf(msg,"%+05.3f G",value);
				gnc_form->s12A9->Enabled = TRUE;
				gnc_form->s12A9->Text = msg;						
			}
d1336 1
a1336 7
			if ( scs_form != NULL )
			{
				value = unscale_data(data, -5, 5);
				sprintf(msg,"%+04.2f �",value);
				scs_form->s12A10->Enabled = TRUE;
				scs_form->s12A10->Text = msg;						
			}
d1341 1
a1341 7
			if ( gnc_form != NULL )
			{
				value = unscale_data(data, -2, 10);
				sprintf(msg,"%+05.3f G",value);
				gnc_form->s12A11->Enabled = TRUE;
				gnc_form->s12A11->Text = msg;						
			}
d1346 1
a1346 7
			if ( gnc_form != NULL )
			{
				value = unscale_data(data, -2, 10);
				sprintf(msg,"%+05.3f G",value);
				gnc_form->s12A12->Enabled = TRUE;
				gnc_form->s12A12->Text = msg;						
			}
d1352 19
a1370 19
				case 0: // 11A14 ECS O2 FLOW O2 SUPPLY MANF
					if ( ecs_form != NULL )
					{
						value = unscale_data(data, 0.2, 1);
						sprintf(msg, "%04.3f PPH", value);
						ecs_form->s11A14->Enabled = TRUE;
						ecs_form->s11A14->Text = msg;						
					}
					break;

				case 1: // 11A50 USB RCVR PHASE ERR
					if ( tcm_form != NULL )
					{
						value = unscale_data(data, -90000, 90000);
						sprintf(msg,"%+05.0f Hz",value);
						tcm_form->s11A50->Enabled = TRUE;
						tcm_form->s11A50->Text = msg;						
					}
					break;
d1385 1
a1385 1
			case 2:	
d1393 2
a1394 7
			case 1:
				if(eps_form != NULL){
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					eps_form->s11A57->Enabled = TRUE;
					eps_form->s11A57->Text = msg;						
				}
d1396 2
a1397 1
			case 2:
d1405 2
a1406 7
			case 1:
				if(eps_form != NULL){
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					eps_form->s11A58->Enabled = TRUE;
					eps_form->s11A58->Text = msg;						
				}
a1409 1

d3029 2
a3030 3
void Form1::parse_lbr(unsigned char data, int bytect){
	char msg[256];
	double value;
@


1.12
log
@More low bit-rate decoding.
@
text
@d25 3
d130 1
d135 1
d139 6
a144 2
double Form1::unscale_data(unsigned char data,double low,double high){
	double step = 0;
d146 7
a152 4
	if(data == 0){    return low;  }
	if(data == 0xFF){ return high; }
	step = ((high - low)/256);
	return (data*step) + low;
d2957 19
a2975 2
			switch(framect){
				case 4:
d2980 26
d3011 1
d3016 1
d3022 1
d3027 1
d3033 3
@


1.11
log
@More rationalising of telemetry decoding.
@
text
@d1 26
d2883 1
d2887 1
d2891 1
d2895 2
a2896 1
		case 4:
d2898 5
a2902 1
			case 1:
d2906 9
a2914 1
			case 4:
d2919 25
@


1.10
log
@Added a common showValue() method for setting a text box contents.
@
text
@d99 2
a100 6
void Form1::end_lbr(){
	if(eps_form != NULL){
		eps_form->s11A91->Enabled = FALSE;
		eps_form->s11A109->Enabled = FALSE;
		eps_form->s11A93->Enabled = FALSE;
	}
d103 2
a104 6
void Form1::end_hbr(){
	if(eps_form != NULL){
		eps_form->s11A91->Enabled = FALSE;
		eps_form->s11A109->Enabled = FALSE;
		eps_form->s11A93->Enabled = FALSE;
	}
d185 18
d248 19
d312 36
d452 27
d726 31
d764 1
a764 7
			if(scs_form != NULL)
			{
				value = unscale_data(data, -10, 10);
				sprintf(msg,"%+04.2f �",value);
				scs_form->s12A6->Enabled = TRUE;
				scs_form->s12A6->Text = msg;						
			} 
d769 1
a769 7
			if(scs_form != NULL)
			{
				value = unscale_data(data, -50, 50);
				sprintf(msg,"%+04.2f �",value);
				scs_form->s12A7->Enabled = TRUE;
				scs_form->s12A7->Text = msg;						
			} 
d774 1
a774 7
			if ( scs_form != NULL )
			{
				value = unscale_data(data, -5, 5);
				sprintf(msg,"%+04.2f �",value);
				scs_form->s12A8->Enabled = TRUE;
				scs_form->s12A8->Text = msg;						
			}
d780 11
a790 9
				case 0: // 11A6 LES LOGIC BUS B VOLTS
					if (els_form != NULL)
					{
						value = unscale_data(data, 0, 40);
						sprintf(msg,"%04.2f V",value);
						els_form->s11A6->Enabled = TRUE;
						els_form->s11A6->Text = msg;						
					} 
					break;
d792 3
a794 8
				case 1: // 11A42 ALPHA CT RATE CHAN 3
					if(tcm_form != NULL){
						value = unscale_data(data,0.1,10000);
						sprintf(msg,"%06.0f C/S",value);
						tcm_form->s11A42->Enabled = TRUE;
						tcm_form->s11A42->Text = msg;						
					}
					break;
d796 3
a798 9
				case 2: // 11A78 FC 2 H2 FLOW
					if(eps_form != NULL)
					{
						value = unscale_data(data, 0, 0.2);
						sprintf(msg,"%04.3f PPH",value);
						eps_form->s11A78->Enabled = TRUE;
						eps_form->s11A78->Text = msg;						
					} 
					break;
d805 15
a819 8
				case 1: // 11A43 PROTON INTEG CT RATE
					if(tcm_form != NULL){
						value = unscale_data(data, 1, 100000);
						sprintf(msg,"%06.0f C/S",value);
						tcm_form->s11A43->Enabled = TRUE;
						tcm_form->s11A43->Text = msg;						
					}
					break;
d821 3
a823 9
				case 2: // 11A79 FC 3 H2 FLOW
					if(eps_form != NULL)
					{
						value = unscale_data(data, 0, 0.2);
						sprintf(msg,"%04.3f PPH",value);
						eps_form->s11A79->Enabled = TRUE;
						eps_form->s11A79->Text = msg;						
					} 
					break;
d830 17
a846 9
				case 0: // 11A8 LES LOGIC BUS A VOLTS
					if (els_form != NULL)
					{
						value = unscale_data(data, 0, 40);
						sprintf(msg,"%04.2f V",value);
						els_form->s11A8->Enabled = TRUE;
						els_form->s11A8->Text = msg;						
					} 
					break;
d848 3
a850 9
				case 2: // 11A80 FC 1 O2 FLOW
					if (eps_form != NULL)
					{
						value = unscale_data(data, 0, 1.6);
						sprintf(msg,"%04.3f PPH",value);
						eps_form->s11A80->Enabled = TRUE;
						eps_form->s11A80->Text = msg;						
					} 
					break;
d852 9
a860 9
				case 4: // 11A152 FUEL SM/ENG INTERFACE P
					if (sps_form != NULL)
					{
						value = unscale_data(data, 0, 300);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A152->Enabled = TRUE;
						sps_form->s11A152->Text = msg;						
					} 
					break;
d1197 1
a1197 6
				if(eps_form != NULL){
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					eps_form->s11A91->Enabled = TRUE;
					eps_form->s11A91->Text = msg;						
				}
d1213 1
a1213 6
				if(eps_form != NULL){
					value = unscale_data(data,0,45);
					sprintf(msg,"%+04.2f V",value);
					eps_form->s11A93->Enabled = TRUE;
					eps_form->s11A93->Text = msg;						
				}
d2868 7
a2874 16
				case 1:
					if(eps_form != NULL){
						value = unscale_data(data,0,100);
						sprintf(msg,"%+05.2f A",value);
						eps_form->s11A109->Enabled = TRUE;
						eps_form->s11A109->Text = msg;						
					}
					break;
				case 4:
					if(eps_form != NULL){
						value = unscale_data(data,0,45);
						sprintf(msg,"%+04.2f V",value);
						eps_form->s11A91->Enabled = TRUE;
						eps_form->s11A91->Text = msg;						
					}
					break;
d2880 1
a2880 6
					if(eps_form != NULL){
						value = unscale_data(data,0,45);
						sprintf(msg,"%+04.2f V",value);
						eps_form->s11A93->Enabled = TRUE;
						eps_form->s11A93->Text = msg;						
					}
@


1.9
log
@Moved about half the values to the display method. Also removing the 'enabling' of text boxes since that allows people to type into them; we should really change the color scheme instead.
@
text
@d125 7
d153 1
a153 1
					ecs_form->s11A1->Text = msg;					
d162 1
a162 1
					ecs_form->s11A2->Text = msg;						
d171 1
a171 1
					ecs_form->s11A3->Text = msg;						
d180 1
a180 1
					ecs_form->s11A4->Text = msg;						
d189 1
a189 1
					els_form->s11A5->Text = msg;						
d198 1
a198 1
					ecs_form->s11A37->Text = msg;						
d207 1
a207 1
					tcm_form->s11A38->Text = msg;						
d216 1
a216 1
					sps_form->s11A39->Text = msg;						
d225 1
a225 1
					sps_form->s11A40->Text = msg;						
d234 1
a234 1
					tcm_form->s11A41->Text = msg;						
d243 1
a243 1
					eps_form->s11A73->Text = msg;						
d252 1
a252 1
					eps_form->s11A74->Text = msg;						
d261 1
a261 1
					eps_form->s11A75->Text = msg;						
d270 1
a270 1
					eps_form->s11A76->Text = msg;						
d279 1
a279 1
					eps_form->s11A77->Text = msg;						
d288 1
a288 1
					eps_form->s11A109->Text = msg;						
d297 1
a297 1
					eps_form->s11A110->Text = msg;						
d306 1
a306 1
					sps_form->s11A111->Text = msg;						
d315 1
a315 1
					sps_form->s11A112->Text = msg;						
d324 1
a324 1
					eps_form->s11A147->Text = msg;						
d333 1
a333 1
					tcm_form->s11A148->Text = msg;						
d347 1
a347 1
					gnc_form->s12A1->Text = msg;						
d356 1
a356 1
					gnc_form->s12A2->Text = msg;						
d365 1
a365 1
					gnc_form->s12A3->Text = msg;						
d374 1
a374 1
					scs_form->s12A4->Text = msg;						
d383 1
a383 1
					scs_form->s12A5->Text = msg;						
d398 1
a398 1
					crw_form->s22A1->Text = msg;						
d407 1
a407 1
					crw_form->s22A2->Text = msg;						
d415 2
a416 2
					sprintf(msg,"%05.4f MV",value);
					crw_form->s22A3->Text = msg;						
d425 1
a425 1
					scs_form->s22A4->Text = msg;	
@


1.8
log
@Started rewriting to use a 'display' method equivalent to 'measure' in the telecoms code.
@
text
@d139 241
a379 1
			switch(ccode){
d384 2
a385 1
			switch(ccode){
a390 1
					crw_form->s22A1->Enabled = TRUE;
a399 1
					crw_form->s22A2->Enabled = TRUE;
d403 18
d482 1
a482 6
			if(crw_form != NULL){
				value = unscale_data(data, 0.1, 0.5);
				sprintf(msg,"%05.4f MV",value);
				crw_form->s22A3->Enabled = TRUE;
				crw_form->s22A3->Text = msg;						
			}
d489 1
a489 7
			if ( scs_form != NULL )
			{
				value = unscale_data(data, 0.1, 0.5);
				sprintf(msg,"%04.3f A",value);
				scs_form->s22A4->Enabled = TRUE;
				scs_form->s22A4->Text = msg;	
			}
d494 2
a495 7
				case 0: // 11A1 SUIT MANF ABS PRESS
				if(ecs_form != NULL){
					value = unscale_data(data, 0, 17);
					sprintf(msg,"%03.1f PSIA",value);
					ecs_form->s11A1->Enabled = TRUE;
					ecs_form->s11A1->Text = msg;						
				}
d498 2
a499 7
				case 1: // 11A37 SUIT-CABIN DELTA PRESS
				if(ecs_form != NULL){
					value = unscale_data(data, -5, 5);
					sprintf(msg,"%+03.2f IN",value);
					ecs_form->s11A37->Enabled = TRUE;
					ecs_form->s11A37->Text = msg;						
				}
d501 3
a503 7
				case 2: // 11A73 BAT CHARGER AMPS
				if(eps_form != NULL){
					value = unscale_data(data, 0, 5);
					sprintf(msg,"%05.2f A",value);
					eps_form->s11A73->Enabled = TRUE;
					eps_form->s11A73->Text = msg;						
				}
d505 3
a507 7
				case 3: // 11A109 BAT B CUR
				if(eps_form != NULL){
					value = unscale_data(data, 0, 100);
					sprintf(msg,"%05.2f A",value);
					eps_form->s11A109->Enabled = TRUE;
					eps_form->s11A109->Text = msg;						
				}
d514 2
a515 7
				case 0: // 11A2 SUIT COMP DELTA P
				if(ecs_form != NULL){
					value = unscale_data(data, 0, 1 );
					sprintf(msg,"%+03.2f PSID",value);
					ecs_form->s11A2->Enabled = TRUE;
					ecs_form->s11A2->Text = msg;						
				} 
d518 2
a519 7
				case 1: // 11A38 ALPHA CT RATE CHAN 1
				if(tcm_form != NULL){
					value = unscale_data(data,0.1,10000);
					sprintf(msg,"%06.0f C/S",value);
					tcm_form->s11A38->Enabled = TRUE;
					tcm_form->s11A38->Text = msg;						
				} 
d522 2
a523 7
				case 2: // 11A74 BAT A CUR
				if(eps_form != NULL){
					value = unscale_data(data,0,100);
					sprintf(msg,"%05.2f A",value);
					eps_form->s11A74->Enabled = TRUE;
					eps_form->s11A74->Text = msg;						
				} 
d526 2
a527 7
				case 3:
				if(eps_form != NULL){
					value = unscale_data(data,0,100);
					sprintf(msg,"%05.2f A",value);
					eps_form->s11A110->Enabled = TRUE;
					eps_form->s11A110->Text = msg;						
				}
d535 19
a553 40
				case 0: // 11A3 GLY PUMP OUT PRESS
					if(ecs_form != NULL){
						value = unscale_data(data,0,60);
						sprintf(msg,"%04.1f PSIG",value);
						ecs_form->s11A3->Enabled = TRUE;
						ecs_form->s11A3->Text = msg;						
					} 
					break;
				case 1: // 11A39 SM HE MANF A PRESS
					if(sps_form != NULL){
						value = unscale_data(data,0,400);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A39->Enabled = TRUE;
						sps_form->s11A39->Text = msg;						
					} 
					break;
				case 2: // 11A75 BAT RELAY BUS VOLTS
					if(eps_form != NULL){
						value = unscale_data(data,0,45);
						sprintf(msg,"%02.2f V",value);
						eps_form->s11A75->Enabled = TRUE;
						eps_form->s11A75->Text = msg;						
					}
					break;
				case 3: // 11A111 SM FU MANF C PRESS
					if(sps_form != NULL){
						value = unscale_data(data,0,400);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A111->Enabled = TRUE;
						sps_form->s11A111->Text = msg;						
					} 
					break;
				case 4: // 11A147 AC BUS 1 PH A VOLTS
					if(eps_form != NULL){
						value = unscale_data(data,0,150);
						sprintf(msg,"%03.2f V",value);
						eps_form->s11A147->Enabled = TRUE;
						eps_form->s11A147->Text = msg;						
					} 
					break;
d560 19
a578 44
				case 0: // 11A4 ECS SURGE TANK PRESS
					if(ecs_form != NULL){
						value = unscale_data(data, 50, 1050);
						sprintf(msg,"%04.0f PSIG",value);
						ecs_form->s11A4->Enabled = TRUE;
						ecs_form->s11A4->Text = msg;						
					} 
					break;
				case 1: // 11A40 SM HE MANF B PRESS
					if(sps_form != NULL)
					{
						value = unscale_data(data,0,400);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A40->Enabled = TRUE;
						sps_form->s11A40->Text = msg;						
					} 
					break;
				case 2: // 11A76 FC 1 CUR
					if(eps_form != NULL)
					{
						value = unscale_data(data,0,100);
						sprintf(msg,"%03.2f A",value);
						eps_form->s11A76->Enabled = TRUE;
						eps_form->s11A76->Text = msg;						
					} 
					break;
				case 3: // 11A112 SM FU MANF D PRESS
					if(sps_form != NULL)
					{
						value = unscale_data(data,0,400);
						sprintf(msg,"%04.0f PSIA",value);
						sps_form->s11A112->Enabled = TRUE;
						sps_form->s11A112->Text = msg;						
					} 
					break;
				case 4: // 11A148 SCE POS SUPPLY VOLTS
					if(tcm_form != NULL)
					{
						value = unscale_data(data, 0, 30);
						sprintf(msg,"%04.2f V",value);
						tcm_form->s11A148->Enabled = TRUE;
						tcm_form->s11A148->Text = msg;						
					} 
					break;
d584 1
a584 7
			if(gnc_form != NULL)
			{
				value = unscale_data(data, -2.5,2.5);
				sprintf(msg,"%+03.2f",value);
				gnc_form->s12A1->Enabled = TRUE;
				gnc_form->s12A1->Text = msg;						
			} 
d589 1
a589 7
			if(gnc_form != NULL)
			{
				value = unscale_data(data, -2.5,2.5);
				sprintf(msg,"%+03.2f",value);
				gnc_form->s12A2->Enabled = TRUE;
				gnc_form->s12A2->Text = msg;						
			} 
d594 1
a594 7
			if(gnc_form != NULL)
			{
				value = unscale_data(data, -2.5,2.5);
				sprintf(msg,"%+03.2f",value);
				gnc_form->s12A3->Enabled = TRUE;
				gnc_form->s12A3->Text = msg;						
			} 
d599 1
a599 7
			if(scs_form != NULL)
			{
				value = unscale_data(data, -50, 50);
				sprintf(msg,"%+04.2f �",value);
				scs_form->s12A4->Enabled = TRUE;
				scs_form->s12A4->Text = msg;						
			} 
d605 11
a615 9
				case 0: // 11A5 PYRO BUS B VOLTS
					if(els_form != NULL)
					{
						value = unscale_data(data, 0, 40);
						sprintf(msg,"%04.2f V",value);
						els_form->s11A5->Enabled = TRUE;
						els_form->s11A5->Text = msg;						
					} 
					break;
d617 3
a619 8
				case 1: // 11A41 ALPHA CT RATE CHAN 2
					if(tcm_form != NULL){
						value = unscale_data(data,0.1,10000);
						sprintf(msg,"%06.0f C/S",value);
						tcm_form->s11A41->Enabled = TRUE;
						tcm_form->s11A41->Text = msg;						
					}
					break;
d621 3
a623 9
				case 2: // 11A77 FC 1 H2 FLOW
					if(eps_form != NULL)
					{
						value = unscale_data(data, 0, 0.2);
						sprintf(msg,"%04.3f PPH",value);
						eps_form->s11A77->Enabled = TRUE;
						eps_form->s11A77->Text = msg;						
					} 
					break;
d629 1
a629 7
			if(scs_form != NULL)
			{
				value = unscale_data(data, -10, 10);
				sprintf(msg,"%+04.2f �",value);
				scs_form->s12A5->Enabled = TRUE;
				scs_form->s12A5->Text = msg;						
			} 
@


1.7
log
@Yet more telemetry.
@
text
@d20 7
d125 45
d211 1
a211 6
			if(crw_form != NULL){
				value = unscale_data(data, 0.1, 0.5);
				sprintf(msg,"%05.4f MV",value);
				crw_form->s22A1->Enabled = TRUE;
				crw_form->s22A1->Text = msg;						
			}
d218 1
a218 6
			if(crw_form != NULL){
				value = unscale_data(data, 0.1, 0.5);
				sprintf(msg,"%05.4f MV",value);
				crw_form->s22A2->Enabled = TRUE;
				crw_form->s22A2->Text = msg;						
			}
d533 1
a533 1
			if(scs_form != NULL)
d539 1
a539 1
			} 
d887 78
@


1.6
log
@More telemetry.
@
text
@d593 25
d636 209
@


1.5
log
@More telemetry.
@
text
@d378 215
@


1.4
log
@More telemetry displays.
@
text
@d155 49
d209 1
a209 1
					sprintf(msg,"%+03.1f PSIA",value);
d214 1
d226 1
a226 1
					sprintf(msg,"%+05.2f A",value);
d234 1
a234 1
					sprintf(msg,"%+05.2f A",value);
d256 1
a256 1
					sprintf(msg,"%+06.0f C/S",value);
d265 1
a265 1
					sprintf(msg,"%+05.2f A",value);
d274 1
a274 1
					sprintf(msg,"%+05.2f A",value);
d283 18
a300 1
			switch(framead){
d304 1
a304 1
						sprintf(msg,"%+02.2f V",value);
d307 8
d320 1
a320 1
						sprintf(msg,"%+03.2f V",value);
d328 50
@


1.3
log
@Added a couple more displayed values.
@
text
@d115 1
a115 1
	return (data*step);
d157 64
d225 2
a226 2
					eps_form->s11A109->Enabled = TRUE;
					eps_form->s11A109->Text = msg;						
d232 21
@


1.2
log
@Made it build in VC2008.
@
text
@d201 8
d220 13
@


1.1
log
@Initial Commit
@
text
@d73 1
a73 1
			ThreadStart *myThreadDelegate = new ThreadStart(this, ThreadTask);
@

