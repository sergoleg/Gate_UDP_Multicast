/*
 * Wmcast.cpp
 *
 *  Created on: 19.07.2019
 *      Author: Oleg Sergeev
 */

#include "Wmcast.hpp"
#include <LoggerCpp.h>

/*****************************************************************************
 * Initialize instance of Wmcast
 * This method MUST be called before an object can be used
 * Этот метод ДОЛЖЕН быть вызван перед использованием объекта
 * Return true if properly initialized
 *****************************************************************************/
bool Wmcast::Init(void) {
	errno = SocketSuccess;

#ifdef WIN32
	// Data structure containing general Windows Sockets Info
	//-------------------------------------------------------------------------
	memset(&m_hWSAData, 0, sizeof(m_hWSAData));
	WSAStartup(MAKEWORD(2, 0), &m_hWSAData);
#endif

	// Create the basic Socket Handle
	//-------------------------------------------------------------------------

	// Create a datagram socket on which to send
	//==================================================
	// Создать сокет дейтаграммы для отправки
	//==================================================
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);

	TranslateSocketError();

	if (IsSocketValid()) {
		logger.debug() << "Opening the datagram socket...OK";
	} else {
		logger.critic() << "Opening datagram socket error: "
				<< DescribeError();
	}

	return (IsSocketValid());
} // Init

/*****************************************************************************
 * Close socket
 * Return true if successfully closed otherwise returns false
 *****************************************************************************/
bool Wmcast::Close(void) {
	bool bRetVal = false;

	// delete internal buffer
	//--------------------------------------------------------------------------
	if (m_pBuffer != NULL) {
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

	// if socket handle is currently valid, close and then invalidate
	//--------------------------------------------------------------------------
	if (IsSocketValid()) {

		if (CLOSE(m_socket) != SocketError) {
			m_socket = INVALID_SOCKET;
			bRetVal = true;
		}

		TranslateSocketError();

		if (bRetVal)
			logger.debug() << "Closing the datagram socket...OK";
		else
			logger.critic() << "Closing datagram socket error: "
					<< DescribeError();

	}

	return bRetVal;
} // Close

/*****************************************************************************
 * Закрыть все открытые ранее сокеты
 *****************************************************************************/
bool Wmcast::Close_Sockets(void) {

	// Пробежимся по всем элементам
	// std::map<std::string, SOCKET> m_socket_table;	// Таблица сокетов для отсылки/приема данных

	std::map<std::string, SOCKET>::iterator it;
	for (it = m_socket_table.begin(); it != m_socket_table.end(); it++) {

		// Извлекаем из таблицы готовый канал для передачи
		//------------------------------------------------
		m_socket = it->second;

		// Закрываем сокет
		//----------------
		Close();
	}

	return true;
} // Сlose_Sockets

/*****************************************************************************
 * Задать параметры многоадрессной группы
 *****************************************************************************/
bool Wmcast::Set_Send(std::string out_interface, std::string out_multiaddr,
		int out_multiport) {

	send_interface = out_interface;	// local IP address of interface
	send_multiaddr = out_multiaddr;	// IP multicast address of group
	send_multiport = out_multiport;	// and port

	return (true);
} // Set_Send

/*****************************************************************************
 * Set Send socket option (передатчик)
 * Return true if successfully closed otherwise returns false
 *****************************************************************************/
bool Wmcast::Set_Send_Opt(void) {

	// Initialize the group sockaddr structure with a
	// group address of (send_multiaddr) and port (send_multiport)
	//============================================================
	// Инициализировать группу sockaddr структуры
	// с групповым адресом (send_multiaddr) и портом (send_multiport)
	//===============================================================
	memset((char*) &mcastGroupAddr, 0, sizeof(mcastGroupAddr));
	mcastGroupAddr.sin_family = AF_INET;
	mcastGroupAddr.sin_addr.s_addr = inet_addr(send_multiaddr.c_str());
	mcastGroupAddr.sin_port = htons(send_multiport);

	logger.debug()
			<< "Initialize the group sockaddr structure with a group address of "
			<< send_multiaddr << " and port " << send_multiport << " ...OK";

	// Disable loopback so you do not receive your own datagrams
	//==========================================================
	// Отключить обратную связь, чтобы не получать собственные дейтаграммы
	// В Win - не работает, в Linux - работает
	//====================================================================
//	char loopch = 0;
	//if (SETSOCKOPT(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char* )&loopch, sizeof(loopch)) == SocketError) {
//	if (SETSOCKOPT(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char* )&loopch,
//			sizeof(loopch)) != SocketSuccess) {
//		TranslateSocketError();
//		logger.critic() << "Setting IP_MULTICAST_LOOP error: "
//				<< DescribeError();
//		Close(/*m_socket*/);
//		return (false);
//	} else
//		logger.debug() << "Disabling the loopback...OK";

	// Set local interface for outbound multicast datagrams
	// The IP address specified must be associated with a local,
	// multicast capable interface
	//==========================================================
	// Установить локальный интерфейс для исходящих многоадресных дейтаграмм
	// Указанный IP-адрес должен быть связан с локальным интерфейсом с поддержкой многоадресной передачи
	//==================================================================================================
	interfaceIP.s_addr = inet_addr(send_interface.c_str());
	//if (SETSOCKOPT(m_socket, IPPROTO_IP, IP_MULTICAST_IF, (char* ) &interfaceIP, sizeof(interfaceIP)) == SocketError) {
	if (SETSOCKOPT(m_socket, IPPROTO_IP, IP_MULTICAST_IF, (char* ) &interfaceIP,
			sizeof(interfaceIP)) != SocketSuccess) {
//		TranslateSocketError();
		logger.critic()
				<< "Setting local interface " << send_interface << " for outbound multicast datagrams IP_MULTICAST_IF "
				<< send_multiaddr << ":"
				<< send_multiport << " ERROR";
		Close(/*m_socket*/);
		return (false);
	} else
		logger.debug() << "Setting the local interface " << send_interface << " ...OK";

	TranslateSocketError();

	return (true);
} // Set_Send_Opt

/*****************************************************************************
 * Send a message to the multicast group specified by the
 * mcastGroupAddr sockaddr structure
 * ==================================================
 * Отправить сообщение в многоадресную группу,
 * указанную структурой sockaddr mcastGroupAddr
 *
 *  @param pBuf        - block of data to be sent.
 *  @param bytesToSend - size of data block to be sent.
 *
 *  @return number of bytes actually sent.
 *   возвращаемое количество байтов фактически отправлено
 *
 *  @return of zero means the connection has been shutdown on the other side.
 *   возврат нуля означает, что соединение было отключено на другой стороне
 *
 *  @return of -1 means that an error has occurred.
 *   возврат -1 означает, что произошла ошибка
 *****************************************************************************/
int32 Wmcast::Send(const uint8 *pBuf, size_t bytesToSend) {
	SetSocketError(SocketSuccess);
	m_nBytesSent = 0;

	if (IsSocketValid()) {
		if ((bytesToSend > 0) && (pBuf != NULL)) {

			// Check error condition and attempt to resend if call was
			// interrupted by a signal
			//---------------------------------------------------------
			// Проверьте состояние ошибки и повторите попытку,
			// если вызов был прерван сигналом
			//---------------------------------------------------------
			do {
				m_nBytesSent = SENDTO(m_socket, pBuf, bytesToSend, 0,
						(const sockaddr* )&mcastGroupAddr,
						sizeof(mcastGroupAddr));
				TranslateSocketError();
			} while (GetSocketError() == SocketInterrupted);

			if ((m_nBytesSent == 0) || (m_nBytesSent == -1)) {
				logger.critic() << "Sending datagram message error: "
						<< DescribeError();
			} else {
				logger.debug() << "Sending datagram message " << m_nBytesSent << " bytes";
			}
		}
	}

	return m_nBytesSent;
} // Send

/*****************************************************************************
 * Отправить сообщение в многоадресную группу
 *****************************************************************************/
bool Wmcast::Send_To_Ether(
		const uint8 *pBuf,
		size_t bytesToSend,
		std::string out_interface,
		std::string out_multiaddr,
		int out_multiport)
{

	int32 nBytesSent = 0;

/*
	// Уникальный идентификатор сокета
	// mcastKey = out_interface + out_multiaddr + out_multiport
	//---------------------------------------------------------
	std::stringstream ss;
	ss << out_multiport;
	std::string str = ss.str();
	std::string mcastKey = out_interface + out_multiaddr + ss.str();

	// Поиск mcastKey в Таблице сокетов для отсылки данных
	//----------------------------------------------------
	if (m_socket_table.find(mcastKey) == m_socket_table.end()) {
		// not found (not present)
		// Инициализируем новый канал для передачи
		//----------------------------------------
		if (!Init())
			return false;
		if (!Set_Send(out_interface, out_multiaddr, out_multiport))
			return false;
		if (!Set_Send_Opt())
			return false;
		m_socket_table[mcastKey] = m_socket;
	} else {
		// found (present)
		// Извлекаем из таблицы готовый канал для передачи
		//------------------------------------------------
		m_socket = m_socket_table[mcastKey];
	}
*/

	// Открыть канал для передачи
	//----------------------------------------
	if (!Init())												return false;
	if (!Set_Send(out_interface, out_multiaddr, out_multiport))	return false;
	if (!Set_Send_Opt())										return false;

	// Отправить сообщение в многоадресную группу
	//----------------------------------------
	nBytesSent = Send(pBuf, bytesToSend);

	// Закрыть канал для передачи
	//----------------------------------------
	Close();

	if (nBytesSent <= 0) {
		return false;
	}

	return true;
} // Send_To_Ether

/*****************************************************************************
 * Задать параметры многоадрессной группы
 *****************************************************************************/
bool Wmcast::Set_Recv(std::string in_interface, std::string in_multiaddr,
		int in_multiport) {

	recv_interface = in_interface;	// local IP address of interface
	recv_multiaddr = in_multiaddr;	// IP multicast address of group
	recv_multiport = in_multiport;	// and port

	return (true);
} // Set_Recv

/*****************************************************************************
 * Set Recv socket option (приемник)
 * Return true if successfully closed otherwise returns false
 *****************************************************************************/
bool Wmcast::Set_Recv_Opt(void) {

	// Enable SO_REUSEADDR to allow multiple instances of this
	// application to receive copies of the multicast datagrams
	//==================================================
	// Включить SO_REUSEADDR, чтобы разрешить нескольким
	// экземплярам этого приложения получать копии
	// многоадресных дейтаграмм
	//==================================================
	int reuse = 1;
	if (SETSOCKOPT(m_socket, SOL_SOCKET, SO_REUSEADDR, (char* ) &reuse,
			sizeof(reuse)) != SocketSuccess) {
		TranslateSocketError();
		logger.critic() << "Setting SO_REUSEADDR error: " << DescribeError();
		Close(/*m_socket*/);
		return (false);
	} else
		logger.debug() << "Setting SO_REUSEADDR...OK";

	// Bind to the proper port number with the IP address
	// specified as INADDR_ANY
	//===================================================
	// Привязать к правильному номеру порта с IP-адресом,
	// указанным как INADDR_ANY
	//===================================================
	memset((char*) &m_stMulticastGroup, 0, sizeof(m_stMulticastGroup));
	m_stMulticastGroup.sin_family = AF_INET;
	m_stMulticastGroup.sin_port = htons(recv_multiport);
	m_stMulticastGroup.sin_addr.s_addr = INADDR_ANY;
	if (bind(m_socket, (struct sockaddr*) &m_stMulticastGroup,
			sizeof(m_stMulticastGroup))) {
		TranslateSocketError();
		logger.critic() << "Binding datagram socket error: "
				<< DescribeError();
		Close(/*m_socket*/);
		return (false);
	} else
		logger.debug() << "Binding datagram socket (port = " << recv_multiport
				<< ") ...OK";

	//
	// Join the multicast group (send_multiaddr) on the local (recv_interface) interface.
	// Note that this IP_ADD_MEMBERSHIP option must be called for each local interface
	// over which the multicast datagrams are to be received
	//===================================================================================
	// Присоединиться к многоадресной группе (recv_multiaddr)
	// на локальном (recv_interface) интерфейсе.
	// Обратите внимание, что эта опция IP_ADD_MEMBERSHIP
	// должна вызываться для каждого локального интерфейса,
	// через который должны быть получены дейтаграммы многоадресной передачи.
	//=======================================================================
	m_stMulticastRequest.imr_multiaddr.s_addr = inet_addr(
			recv_multiaddr.c_str());
	m_stMulticastRequest.imr_interface.s_addr = inet_addr(
			recv_interface.c_str());
	if (SETSOCKOPT(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char* ) &m_stMulticastRequest, sizeof(m_stMulticastRequest))
			!= SocketSuccess) {
		TranslateSocketError();
		logger.critic() << "Adding multicast group error: "
				<< DescribeError();
		Close(/*m_socket*/);
		return (false);
	} else
		logger.debug() << "Join the multicast group (" << send_multiaddr
				<< ") on the local (" << recv_interface << ") interface ...OK";

	TranslateSocketError();

	return (true);
} // Set_Recv_Opt

/*****************************************************************************
 * Receive() - пытается получить блок данных по установленному соединению.
 * Данные принимаются во внутреннем буфере, управляемом классом.
 * Этот буфер действителен только до следующего вызова метода Receive(),
 * вызова метода Close() или до тех пор, пока объект не выйдет из области видимости.
 *
 * Attempts to receive a block of data on an established connection.
 *
 * @param nMaxBytes - maximum number of bytes to receive.
 * @param pBuffer   - memory where to receive the data,
 *                    NULL receives to internal buffer returned with GetData()
 *                    Non-NULL receives directly there, but GetData() will return WRONG ptr!
 *
 * @return number of bytes actually received.
 *
 * @return of zero means the connection has been shutdown on the other side.
 *
 * @return of -1 means that an error has occurred.
 *****************************************************************************/
int32 Wmcast::Receive(int32 nMaxBytes, uint8 *pBuffer) {
	m_nBytesReceived = 0;

	// If the socket is invalid then return false.
	//--------------------------------------------------------------------------
	if (IsSocketValid() == false) {
		return m_nBytesReceived;
	}

	uint8 *pWorkBuffer = pBuffer;

	if (pBuffer == NULL) {

		// Free existing buffer and allocate a new buffer the size of
		// nMaxBytes.
		//--------------------------------------------------------------------------
		if ((m_pBuffer != NULL) && (nMaxBytes != m_nBufferSize)) {
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}

		// Allocate a new internal buffer to receive data.
		//--------------------------------------------------------------------------
		if (m_pBuffer == NULL) {
			m_nBufferSize = nMaxBytes;
			m_pBuffer = new uint8[nMaxBytes];
		}

		pWorkBuffer = m_pBuffer;
	}

	SetSocketError(SocketSuccess);

	uint32 srcSize;
	srcSize = sizeof(struct sockaddr_in);

	do {
		m_nBytesReceived = RECVFROM(m_socket, pWorkBuffer, nMaxBytes, 0,
				&m_stMulticastGroup, &srcSize);
		TranslateSocketError();
	} while (GetSocketError() == SocketInterrupted);

	TranslateSocketError();

	if (m_nBytesReceived > 0) {
		logger.debug() << "Reading datagram message...OK";
		logger.debug() << "The message from multicast is: \"" << pWorkBuffer
				<< "\"";
	} else {
		logger.critic() << "Reading datagram message error: "
				<< DescribeError();
	}

	// If we encounter an error translate the error code and return.  One
	// possible error code could be EAGAIN (EWOULDBLOCK) if the socket is
	// non-blocking.  This does not mean there is an error, but no data is
	// yet available on the socket.
	//--------------------------------------------------------------------------
	if (m_nBytesReceived == SocketError) {
		if (m_pBuffer != NULL) {
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}
	}

	return m_nBytesReceived;
} // Receive

/*****************************************************************************
 * Получить сообщение для многоадресной группы
 *****************************************************************************/
bool Wmcast::Receive_From_Ether(int32 nMaxBytes, uint8 *pBuffer,
		std::string in_interface, std::string in_multiaddr, int in_multiport) {

	int32 nBytesRecv = 0;

	// Уникальный идентификатор сокета
	// mcastKey = in_interface + in_multiaddr + in_multiport
	//---------------------------------------------------------
	std::stringstream ss;
	ss << in_multiport;
	std::string str = ss.str();
	std::string mcastKey = in_interface + in_multiaddr + ss.str();

	// Поиск mcastKey в Таблице сокетов для приема данных
	//----------------------------------------------------
	if (m_socket_table.find(mcastKey) == m_socket_table.end()) {
		// not found (not present)
		// Инициализируем новый канал для приема
		//--------------------------------------
		if (!Init())
			return false;
		if (!Set_Recv(in_interface, in_multiaddr, in_multiport))
			return false;
		if (!Set_Recv_Opt())
			return false;
		m_socket_table[mcastKey] = m_socket;
	} else {
		// found (present)
		// Извлекаем из таблицы готовый канал для приема
		//----------------------------------------------
		m_socket = m_socket_table[mcastKey];
	}

	nBytesRecv = Receive(nMaxBytes, pBuffer);

	if (nBytesRecv <= 0) {
		return false;
	}

	return true;
} // Receive_From_Ether

/*****************************************************************************
 * Provides a standard error code for cross platform development by
 * mapping the operating system error to an error defined by the class
 *****************************************************************************/
void Wmcast::TranslateSocketError(void) {
#if defined(_LINUX) || defined(_DARWIN)
	switch (errno)
	{
		case EXIT_SUCCESS:
		SetSocketError(SocketSuccess);
		break;
		case ENOTCONN:
		SetSocketError(SocketNotconnected);
		break;
		case ENOTSOCK:
		case EBADF:
		case EACCES:
		case EAFNOSUPPORT:
		case EMFILE:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case EPROTONOSUPPORT:
		case EPIPE:
		SetSocketError(SocketInvalidSocket);
		break;
		case ECONNREFUSED :
		SetSocketError(SocketConnectionRefused);
		break;
		case ETIMEDOUT:
		SetSocketError(SocketTimedout);
		break;
		case EINPROGRESS:
		SetSocketError(SocketEinprogress);
		break;
		case EWOULDBLOCK:
		//case EAGAIN:
		SetSocketError(SocketEwouldblock);
		break;
		case EINTR:
		SetSocketError(SocketInterrupted);
		break;
		case ECONNABORTED:
		SetSocketError(SocketConnectionAborted);
		break;
		case EINVAL:
		case EPROTO:
		SetSocketError(SocketProtocolError);
		break;
		case EPERM:
		SetSocketError(SocketFirewallError);
		break;
		case EFAULT:
		SetSocketError(SocketInvalidSocketBuffer);
		break;
		case ECONNRESET:
		case ENOPROTOOPT:
		SetSocketError(SocketConnectionReset);
		break;
		default:
		SetSocketError(SocketEunknown);
		break;
	}
#endif
#ifdef WIN32
	int32 nError = WSAGetLastError();
	switch (nError) {
	case EXIT_SUCCESS:
		SetSocketError(SocketSuccess);
		break;
	case WSAEBADF:
	case WSAENOTCONN:
		SetSocketError(SocketNotconnected);
		break;
	case WSAEINTR:
		SetSocketError(SocketInterrupted);
		break;
	case WSAEACCES:
	case WSAEAFNOSUPPORT:
	case WSAEINVAL:
	case WSAEMFILE:
	case WSAENOBUFS:
	case WSAEPROTONOSUPPORT:
		SetSocketError(SocketInvalidSocket);
		break;
	case WSAECONNREFUSED:
		SetSocketError(SocketConnectionRefused);
		break;
	case WSAETIMEDOUT:
		SetSocketError(SocketTimedout);
		break;
	case WSAEINPROGRESS:
		SetSocketError(SocketEinprogress);
		break;
	case WSAECONNABORTED:
		SetSocketError(SocketConnectionAborted);
		break;
	case WSAEWOULDBLOCK:
		SetSocketError(SocketEwouldblock);
		break;
	case WSAENOTSOCK:
		SetSocketError(SocketInvalidSocket);
		break;
	case WSAECONNRESET:
		SetSocketError(SocketConnectionReset);
		break;
	case WSANO_DATA:
		SetSocketError(SocketInvalidAddress);
		break;
	case WSAEADDRINUSE:
		SetSocketError(SocketAddressInUse);
		break;
	case WSAEFAULT:
		SetSocketError(SocketInvalidPointer);
		break;
	default:
		SetSocketError(SocketEunknown);
		break;
	}
#endif
} // TranslateSocketError

/*****************************************************************************
 * Returns a human-readable description of the given error code
 * or the last error code of a socket
 *****************************************************************************/
const char* Wmcast::DescribeError(WmcastError err) {
	switch (err) {
	case SocketError:
		return "Generic socket error translates to error below.";
	case SocketSuccess:
		return "No socket error.";
	case SocketInvalidSocket:
		return "Invalid socket handle.";
	case SocketInvalidAddress:
		return "Invalid destination address specified.";
	case SocketInvalidPort:
		return "Invalid destination port specified.";
	case SocketConnectionRefused:
		return "No server is listening at remote address.";
	case SocketTimedout:
		return "Timed out while attempting operation.";
	case SocketEwouldblock:
		return "Operation would block if socket were blocking.";
	case SocketNotconnected:
		return "Currently not connected.";
	case SocketEinprogress:
		return "Socket is non-blocking and the connection cannot be completed immediately";
	case SocketInterrupted:
		return "Call was interrupted by a signal that was caught before a valid connection arrived.";
	case SocketConnectionAborted:
		return "The connection has been aborted.";
	case SocketProtocolError:
		return "Invalid protocol for operation.";
	case SocketFirewallError:
		return "Firewall rules forbid connection.";
	case SocketInvalidSocketBuffer:
		return "The receive buffer point outside the process's address space.";
	case SocketConnectionReset:
		return "Connection was forcibly closed by the remote host.";
	case SocketAddressInUse:
		return "Address already in use.";
	case SocketInvalidPointer:
		return "Pointer type supplied as argument is invalid.";
	case SocketEunknown:
		return "Unknown error";
	default:
		return "No such CSimpleSocket error";
	}
} // DescribeError
