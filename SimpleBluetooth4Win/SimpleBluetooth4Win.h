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

#ifndef SIMPLE_BLUETOOTH_4_WIN
#define SIMPLE_BLUETOOTH_4_WIN

namespace SimpleBluetooth4Win
{
	typedef std::string BluetoothName;
	typedef std::string BluetoothAddress;
	typedef unsigned long long bt_addr, * pbt_addr, BT_ADDR, * PBT_ADDR;

	class CBluetoothWrapper
	{
	public:
		CBluetoothWrapper();

		~CBluetoothWrapper();

		bool Init();

		void Reset();

		bool GetPairedBluetoothDevices(std::vector<std::tuple<BluetoothName, BluetoothAddress, BT_ADDR>> & rvtPairedBTDevices);

		bool BlockingListen(); 

		void CloseConnection();

		bool OpenOutgoingConnectionToPairedBluetoothDevice(int iBTDevNum);

		int NonBlockingWrite(unsigned char * buf, int iHowManyBytesToWrite); // if succeeded, returns as a positive number of bytes written, otherwise a negative number error code, otherwise zero

		int NonBlockingRead(unsigned char * outBuf, int iHowManyBytesToRead); // if succeeded, returns as a positive number of bytes read, otherwhise a negative number error code, otherwhise zero

	private:

		bool FindPairedBluetoothDevices();

		std::vector<std::tuple<BluetoothName, BluetoothAddress, BT_ADDR>> m_vtPairedBTDevices;

		SOCKET Socket;

	};






}












#endif

