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
#include <ws2bth.h>
#include <BluetoothAPIs.h>
#include <initguid.h> 

#include "SimpleBluetooth4Win.h"

// {B62C4E8D-62CC-404b-BBBF-BF3E3BBB1374} 
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);

namespace SimpleBluetooth4Win
{
	union
	{
		char buf[5000];
		SOCKADDR_BTH _Unused_;
	} butuh;

	CBluetoothWrapper::CBluetoothWrapper()
	{


	}

	CBluetoothWrapper::~CBluetoothWrapper()
	{


	}

	void CBluetoothWrapper::Reset()
	{
		try
		{
			CloseConnection();

			if (WSACleanup() != 0)
			{
				// LOG...
			}
		}
		catch (...)
		{

		}
	}

	bool CBluetoothWrapper::Init()
	{
		try
		{
			Reset();

			WSADATA wsd;

			if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
			{
				// LOG...
				Reset();
				return false;
			}

			if (!FindPairedBluetoothDevices())
			{
				// LOG..
				Reset();
				return false;
			}

			return true;
		}
		catch (...)
		{
			// LOG...
			Reset();
			return false;
		}
	}

	bool CBluetoothWrapper::FindPairedBluetoothDevices()
	{
		try
		{
			WSAQUERYSET wsaq;
			HANDLE hLookup;
			LPWSAQUERYSET pwsaResults;
			DWORD dwSize;
			BOOL bHaveName;
			BT_ADDR btAddr;
			DWORD dwNameSpace;

			pwsaResults = (LPWSAQUERYSET)butuh.buf;
			dwSize = sizeof(butuh.buf);
			memset((void *) &wsaq, 0, sizeof(wsaq));
			wsaq.dwSize = sizeof(wsaq);
			wsaq.dwNameSpace = NS_BTH;
			wsaq.lpcsaBuffer = NULL;

			if (WSALookupServiceBegin(&wsaq, LUP_CONTAINERS, &hLookup) == SOCKET_ERROR)
			{
				// LOG...
				return false;
			}

			memset((void *) pwsaResults, 0, sizeof(WSAQUERYSET));
			pwsaResults->dwSize = sizeof(WSAQUERYSET);
			pwsaResults->dwNameSpace = NS_BTH;
			pwsaResults->lpBlob = NULL;

			while (WSALookupServiceNext(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults) == 0)
			{
				btAddr = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
				bHaveName = (pwsaResults->lpszServiceInstanceName) && *(pwsaResults->lpszServiceInstanceName);
				dwNameSpace = pwsaResults->dwNameSpace;

				BluetoothName BTName = pwsaResults->lpszServiceInstanceName;
				unsigned char BTAddrBuffer[20];
				sprintf_s((char *)BTAddrBuffer, 19, "%04X%08X", GET_NAP(btAddr), GET_SAP(btAddr));
				BluetoothAddress BTAddr = (char *)BTAddrBuffer;

				m_vtPairedBTDevices.insert(m_vtPairedBTDevices.begin(), std::tuple<BluetoothName, BluetoothAddress, BT_ADDR>(BTName, BTAddr, btAddr));
			}

			if (WSALookupServiceEnd(hLookup) != 0)
			{
				// LOG...
				return false;
			}
			return true;
		}
		catch (...)
		{
			// LOG ...
			return false;
		}
	}

	bool CBluetoothWrapper::GetPairedBluetoothDevices(std::vector<std::tuple<BluetoothName, BluetoothAddress, BT_ADDR>> & rvtPairedBTDevices)
	{
		try
		{
			rvtPairedBTDevices = m_vtPairedBTDevices;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool CBluetoothWrapper::BlockingListen()
	{
		try
		{
			SOCKET          LocalSocket = INVALID_SOCKET;
			SOCKADDR_BTH    SockAddrBthLocal = { 0 };
			int             iAddrLen = sizeof(SOCKADDR_BTH);
			CSADDR_INFO     CSAddrInfo;
			LPCSADDR_INFO   lpCSAddrInfo = &CSAddrInfo;
			WSAQUERYSET     wsaQuerySet = { 0 };

			LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
			if (INVALID_SOCKET == LocalSocket)
			{
				return false;
			}
			SockAddrBthLocal.addressFamily = AF_BTH;
			SockAddrBthLocal.port = BT_PORT_ANY;
			if (SOCKET_ERROR == bind(LocalSocket, (struct sockaddr *) &SockAddrBthLocal, sizeof(SOCKADDR_BTH)))
			{
				if (INVALID_SOCKET != LocalSocket)
				{
					closesocket(LocalSocket);
					LocalSocket = INVALID_SOCKET;
				}
				return false;
			}
			if (SOCKET_ERROR == getsockname(LocalSocket, (struct sockaddr *)&SockAddrBthLocal, &iAddrLen))
			{
				if (INVALID_SOCKET != LocalSocket)
				{
					closesocket(LocalSocket);
					LocalSocket = INVALID_SOCKET;
				}
				return false;
			}
			lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
			lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
			lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
			lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
			lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
			lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;

			memset((void *) &wsaQuerySet, 0, sizeof(WSAQUERYSET));
			wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
			wsaQuerySet.lpServiceClassId = (LPGUID)&g_guidServiceClass;

			wsaQuerySet.lpszServiceInstanceName = (LPSTR) "SimpleBluetooth4Win Server";
			wsaQuerySet.lpszComment = "Example Service instance registered in the directory service through RnR";
			wsaQuerySet.dwNameSpace = NS_BTH;
			wsaQuerySet.dwNumberOfCsAddrs = 1;
			wsaQuerySet.lpcsaBuffer = lpCSAddrInfo;

			if (SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0))
			{
				if (INVALID_SOCKET != LocalSocket)
				{
					closesocket(LocalSocket);
					LocalSocket = INVALID_SOCKET;
				}
				return false;
			}

			if (SOCKET_ERROR == listen(LocalSocket, 4))
			{
				WSASetService(&wsaQuerySet, RNRSERVICE_DELETE, 0);
				if (INVALID_SOCKET != LocalSocket)
				{
					closesocket(LocalSocket);
					LocalSocket = INVALID_SOCKET;
				}
				return false;
			}

			Socket = INVALID_SOCKET;
			Socket = accept(LocalSocket, NULL, NULL);
			if (INVALID_SOCKET == Socket)
			{
				WSASetService(&wsaQuerySet, RNRSERVICE_DELETE, 0);
				if (INVALID_SOCKET != LocalSocket)
				{
					closesocket(LocalSocket);
					LocalSocket = INVALID_SOCKET;
				}
				return false;
			}

			WSASetService(&wsaQuerySet, RNRSERVICE_DELETE, 0);
			if (INVALID_SOCKET != LocalSocket)
			{
				closesocket(LocalSocket);
				LocalSocket = INVALID_SOCKET;
			}

			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	void CBluetoothWrapper::CloseConnection()
	{
		try
		{
			if (Socket != INVALID_SOCKET)
			{
				if (SOCKET_ERROR == closesocket(Socket))
				{
					return;
				}
				Socket = INVALID_SOCKET;
			}
		}
		catch (...)
		{
			// LOG...
		}
	}

	bool CBluetoothWrapper::OpenOutgoingConnectionToPairedBluetoothDevice(int iBTDevNum)
	{
		try
		{
			SOCKADDR_BTH    SockAddrBthServer;

			if ((iBTDevNum < 0) || (iBTDevNum >= (int) m_vtPairedBTDevices.size()))
			{
				// LOG...
				return false;
			}

			SockAddrBthServer.btAddr = std::get<2>(m_vtPairedBTDevices.at(iBTDevNum));
			SockAddrBthServer.addressFamily = AF_BTH;
			SockAddrBthServer.serviceClassId = g_guidServiceClass;
			SockAddrBthServer.port = 0;

			Socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
			if (INVALID_SOCKET == Socket)
			{
				return false;
			}

			if (SOCKET_ERROR == connect(Socket, (struct sockaddr *) &SockAddrBthServer, sizeof(SOCKADDR_BTH)))
			{
				if (SOCKET_ERROR == closesocket(Socket))
				{
					return false;
				}
				Socket = INVALID_SOCKET;
				return false;
			}

			return true;
		}
		catch (...)
		{
			// LOG...
			return false;
		}
	}

	int CBluetoothWrapper::NonBlockingWrite(unsigned char * buf, int iHowManyBytesToWrite)
	{
		try
		{
			if (Socket == INVALID_SOCKET)
			{
				return -1;
			}
			if (SOCKET_ERROR == send(Socket, (char *)buf, iHowManyBytesToWrite, 0))
			{
				return -1;
			}
			return iHowManyBytesToWrite;
		}
		catch (...)
		{
			// LOG...
			return 0;
		}
	}

	int CBluetoothWrapper::NonBlockingRead(unsigned char * outBuf, int iHowManyBytesToRead)
	{
		try
		{
			int iLengthReceived = 0;
			if (Socket == INVALID_SOCKET)
			{
				return -1;
			}
			iLengthReceived = recv(Socket, (char *)outBuf, iHowManyBytesToRead, 0);
			if (iLengthReceived == SOCKET_ERROR)
			{
				return -1;
			}
			if (iLengthReceived < 0)
			{
				return -1;
			}
			return iLengthReceived;
		}
		catch (...)
		{
			// LOG ...
			return 0;
		}
	}
}


