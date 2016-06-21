//-----------------------------------------------------------------------
//
// This file is part of the SimpleBluetooth4Win Project
//
//  by Amos Tibaldi - tibaldi at users.sourceforge.net
//
// http://sourceforge.net/projects/simplebluetooth4win/
//
// http://simplebluetooth4win.sourceforge.net/
//
//
// COPYRIGHT: http://www.gnu.org/licenses/gpl.html
//            COPYRIGHT-gpl-3.0.txt
//
//     The SimpleBluetooth4Win Project
//         Simple bluetooth library for windows 
//
//     Copyright (C) 2015 Amos Tibaldi
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------

#include <vector>
#include <tuple>
#include <winsock2.h>


#include <iostream>

#include "SimpleBluetooth4Win.h"

using namespace SimpleBluetooth4Win;

int main(int argc, char * * argv)
{
	CBluetoothWrapper cbw;
	if (!cbw.Init())
	{
		cbw.Reset();
		return -1;
	}

	if (argc > 1)
	{ // bluetooth server mode
		if (!cbw.BlockingListen())
		{
			cbw.Reset();
			return -2;
		}
		Sleep(100);
		unsigned char vucInputBufferFromClient[50];
		memset((void *)vucInputBufferFromClient, 0, 50);
		int iBytesRead = cbw.NonBlockingRead(vucInputBufferFromClient, 50);
		if (iBytesRead > 0)
		{
			std::cout << "Read from client: " << vucInputBufferFromClient << std::endl;
		}
		else
		{
			std::cout << "Cannot read bytes from connected client..." << std::endl;
		}
		std::string sMessageFromServerToClient = "Welcome!";
		cbw.NonBlockingWrite((unsigned char *)sMessageFromServerToClient.c_str(), strlen(sMessageFromServerToClient.c_str()));
	}
	else
	{ // bluetooth client mode
		std::vector<std::tuple<BluetoothName, BluetoothAddress, BT_ADDR>> vtBTPairedDevices;
		if (cbw.GetPairedBluetoothDevices(vtBTPairedDevices))
		{
			for each (std::tuple<BluetoothName, BluetoothAddress, BT_ADDR> theTuple in vtBTPairedDevices)
			{
				std::cout << "---> Found Device Named " << std::get<0>(theTuple).c_str() << " with address " << std::get<1>(theTuple).c_str() << std::endl;
			}
		}
		if (!cbw.OpenOutgoingConnectionToPairedBluetoothDevice(0))
		{
			std::cout << "Cannot connect to bluetooth server..." << std::endl;
			cbw.Reset();
			return -3;
		}
		std::string sClientToServerMessage = "hello";
		cbw.NonBlockingWrite((unsigned char *)sClientToServerMessage.c_str(), strlen((const char *)sClientToServerMessage.c_str()));
		unsigned char vucInputBufferFromServer[100];
		memset((void *)vucInputBufferFromServer, 0, 100);
		Sleep(500);
		int iBytesRead = cbw.NonBlockingRead(vucInputBufferFromServer, 100);
		if (iBytesRead > 0)
		{
			std::cout << "Read from server: " << vucInputBufferFromServer << std::endl;
		}
		else
		{
			std::cout << "Cannot read bytes from connected server..." << std::endl;
		}
	}

	cbw.Reset();
	return 0;
}

