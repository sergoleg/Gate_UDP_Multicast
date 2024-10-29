/*
 * Wmcast.hpp - Поддержка многоадресной рассылки (мультикастинга)
 *
 *  Created on: 19.07.2019
 *      Author: Oleg Sergeev
 */

#ifndef _WMCAST_HPP_
#define _WMCAST_HPP_

// Utilities library
#include <cstdlib>

// Strings library
#include <string>

// Containers library
#include <vector>		// std::vector container
#include <deque>		// std::deque container
#include <list>			// std::list container
#include <set>			// std::set and std::multiset associative containers
#include <map>			// std::map and std::multimap associative containers
#include <stack>		// std::stack container adaptor
#include <queue>		// std::queue and std::priority_queue container adaptors

// Input/output library
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <sstream>

// Algorithms library
#include <algorithm>

#include <limits.h>

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* SOCKET Definition                                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#if defined(_LINUX) || defined (_DARWIN)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netdb.h>
#endif
#ifdef _LINUX
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <sys/sendfile.h>
#endif
#ifdef _DARWIN
#include <net/if.h>
#endif
#if defined(_LINUX) || defined (_DARWIN)
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

#define IPTOS_LOWDELAY  0x10

#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET    ~(0)
#endif

#define SOCKET_SENDFILE_BLOCKSIZE 8192

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Type Definition Macros                                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifndef __WORDSIZE
/* Assume 32 */
#define __WORDSIZE 32
#endif

#if defined(_LINUX) || defined(_DARWIN)
	typedef unsigned char uint8;
	typedef char int8;
	typedef unsigned short uint16;
	typedef short int16;
	typedef unsigned int uint32;
	typedef int int32;
	typedef int SOCKET;
#endif

#ifdef WIN32
struct iovec {
	void *iov_base;
	size_t iov_len;
};

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;
#endif

#ifdef WIN32
//typedef int socklen_t;
#endif

#if defined(WIN32)
typedef unsigned long long int uint64;
typedef long long int int64;
#elif (__WORDSIZE == 32)
__extension__
typedef long long int int64;
__extension__
typedef unsigned long long int uint64;
#elif (__WORDSIZE == 64)
typedef unsigned long int uint64;
typedef long int int64;
#endif

#ifdef WIN32

#ifndef UINT8_MAX
#define UINT8_MAX  (UCHAR_MAX)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX (USHRT_MAX)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX (ULONG_MAX)
#endif

#if __WORDSIZE == 64
#define SIZE_MAX (18446744073709551615UL)
#else
#ifndef SIZE_MAX
#define SIZE_MAX (4294967295U)
#endif
#endif
#endif

#if defined(WIN32)
#define ssize_t size_t
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef htonll
#ifdef _BIG_ENDIAN
#define htonll(x)   (x)
#define ntohll(x)   (x)
#else
#define htonll(x)   ((((uint64)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x)   ((((uint64)ntohl(x)) << 32) + ntohl(x >> 32))
#endif
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Socket Macros                                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifdef WIN32
#define SHUT_RD                0
#define SHUT_WR                1
#define SHUT_RDWR              2
#define ACCEPT(a,b,c)          accept(a,b,c)
#define CONNECT(a,b,c)         connect(a,b,c)
#define CLOSE(a)               closesocket(a)
#define READ(a,b,c)            read(a,b,c)
#define RECV(a,b,c,d)          recv(a, (char *)b, c, d)
#define RECVFROM(a,b,c,d,e,f)  recvfrom(a, (char *)b, c, d, (sockaddr *)e, (int *)f)
#define RECV_FLAGS             MSG_WAITALL
#define SELECT(a,b,c,d,e)      select((int32)a,b,c,d,e)
#define SEND(a,b,c,d)          send(a, (const char *)b, (int)c, d)
#define SENDTO(a,b,c,d,e,f)    sendto(a, (const char *)b, (int)c, d, e, f)
#define SEND_FLAGS             0
#define SENDFILE(a,b,c,d)      sendfile(a, b, c, d)
#define SET_SOCKET_ERROR(x,y)  errno=y
#define SOCKET_ERROR_INTERUPT  EINTR
#define SOCKET_ERROR_TIMEDOUT  EAGAIN
#define WRITE(a,b,c)           write(a,b,c)
#define WRITEV(a,b,c)          Writev(b, c)
#define GETSOCKOPT(a,b,c,d,e)  getsockopt(a,b,c,(char *)d, (int *)e)
#define SETSOCKOPT(a,b,c,d,e)  setsockopt(a,b,c,(char *)d, (int)e)
#define GETHOSTBYNAME(a)       gethostbyname(a)
#endif

#if defined(_LINUX) || defined(_DARWIN)
#define ACCEPT(a,b,c)          accept(a,b,c)
#define CONNECT(a,b,c)         connect(a,b,c)
#define CLOSE(a)               close(a)
#define READ(a,b,c)            read(a,b,c)
#define RECV(a,b,c,d)          recv(a, (void *)b, c, d)
#define RECVFROM(a,b,c,d,e,f)  recvfrom(a, (char *)b, c, d, (sockaddr *)e, f)
#define RECV_FLAGS             MSG_WAITALL
#define SELECT(a,b,c,d,e)      select(a,b,c,d,e)
#define SEND(a,b,c,d)          send(a, (const int8 *)b, c, d)
#define SENDTO(a,b,c,d,e,f)    sendto(a, (const int8 *)b, c, d, e, f)
#define SEND_FLAGS             0
#define SENDFILE(a,b,c,d)      sendfile(a, b, c, d)
#define SET_SOCKET_ERROR(x,y)  errno=y
#define SOCKET_ERROR_INTERUPT  EINTR
#define SOCKET_ERROR_TIMEDOUT  EAGAIN
#define WRITE(a,b,c)           write(a,b,c)
#define WRITEV(a,b,c)          writev(a, b, c)
#define GETSOCKOPT(a,b,c,d,e)  getsockopt((int)a,(int)b,(int)c,(void *)d,(socklen_t *)e)
#define SETSOCKOPT(a,b,c,d,e)  setsockopt((int)a,(int)b,(int)c,(const void *)d,(int)e)
#define GETHOSTBYNAME(a)       gethostbyname(a)
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* File Macros                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#define STRUCT_STAT         struct stat
#define LSTAT(x,y)          lstat(x,y)
#define FILE_HANDLE         FILE *
#define CLEARERR(x)         clearerr(x)
#define FCLOSE(x)           fclose(x)
#define FEOF(x)             feof(x)
#define FERROR(x)           ferror(x)
#define FFLUSH(x)           fflush(x)
#define FILENO(s)           fileno(s)
#define FOPEN(x,y)          fopen(x, y)
//#define FREAD(a,b,c,d)      fread(a, b, c, d)
#define FSTAT(s, st)        fstat(FILENO(s), st)
//#define FWRITE(a,b,c,d)     fwrite(a, b, c, d)
#define STAT_BLK_SIZE(x)    ((x).st_blksize)

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Misc Macros                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#if defined(WIN32)
#define GET_CLOCK_COUNT(x) QueryPerformanceCounter((LARGE_INTEGER *)x)
#else
#define GET_CLOCK_COUNT(x) gettimeofday(x, NULL)
#endif

#if defined(WIN32)
#define STRTOULL(x) _atoi64(x)
#else
#define STRTOULL(x) strtoull(x, NULL, 10)
#endif

#if defined(WIN32)
#define SNPRINTF _snprintf
#define PRINTF   printf
#define VPRINTF  vprintf
#define FPRINTF  fprintf
#else
#define SNPRINTF snprintf
#define PRINTF   printf
#define VPRINTF  vprintf
#define FPRINTF  fprintf
#endif

#ifdef __cplusplus
}
#endif

#include <LoggerCpp.h>

/*****************************************************************************
 *
 *****************************************************************************/

#if defined(WIN32)
#define GET_CLOCK_COUNT(x) QueryPerformanceCounter((LARGE_INTEGER *)x)
#else
#define GET_CLOCK_COUNT(x) gettimeofday(x, NULL)
#endif

#define MILLISECONDS_CONVERSION 1000
#define MICROSECONDS_CONVERSION 1000000

/*****************************************************************************
 *
 *****************************************************************************/

#define SEND_MCAST_ADDRESS	"224.0.0.224"
#define SEND_MCAST_PORT		4444
#define SEND_MCAST_IF		"192.0.0.192"
#define RECV_MCAST_IF		"192.0.0.192"

/*****************************************************************************
 * Класс для абстрактного сокетного взаимодействия в кроссплатформенном виде
 *****************************************************************************/
class Wmcast {
public:
	// список методов доступных другим функциям и объектам программы
	//--------------------------------------------------------------
	Log::Logger logger;	// A named logger to produce log

	Wmcast() :
			logger("Wmcast") {

		send_interface = SEND_MCAST_IF;
		send_multiaddr = SEND_MCAST_ADDRESS;
		send_multiport = SEND_MCAST_PORT;

		recv_interface = RECV_MCAST_IF;
		recv_multiaddr = SEND_MCAST_ADDRESS;
		recv_multiport = SEND_MCAST_PORT;

		m_socket = INVALID_SOCKET;
		m_socketErrno = SocketInvalidSocket;
		m_pBuffer = NULL;
		m_nBufferSize = 0;
		m_nBytesSent = -1;
		m_nBytesReceived = -1;
	}
	;

	~Wmcast() {

	}
	;

	// Defines all error codes handled by the CSimpleSocket class
	//-----------------------------------------------------------
	typedef enum {
		SocketError = -1,	// Generic socket error translates to error below
		SocketSuccess = 0,			// No socket error
		SocketInvalidSocket,		// Invalid socket handle
		SocketInvalidAddress,		// Invalid destination address specified
		SocketInvalidPort,			// Invalid destination port specified
		SocketConnectionRefused,	// No server is listening at remote address
		SocketTimedout,				// Timed out while attempting operation
		SocketEwouldblock,		// Operation would block if socket were blocking
		SocketNotconnected,			// Currently not connected
		SocketEinprogress,// Socket is non-blocking and the connection cannot be completed immediately
		SocketInterrupted,// Call was interrupted by a signal that was caught before a valid connection arrived
		SocketConnectionAborted,	// The connection has been aborted
		SocketProtocolError,		// Invalid protocol for operation
		SocketFirewallError,		// Firewall rules forbid connection
		SocketInvalidSocketBuffer,// The receive buffer point outside the process's address space
		SocketConnectionReset,// Connection was forcibly closed by the remote host
		SocketAddressInUse,			// Address already in use
		SocketInvalidPointer,	// Pointer type supplied as argument is invalid
		SocketEunknown	// Unknown error please report to <sergoleg@gmail.com>
	} WmcastError;

	std::string send_interface;	// local IP address of interface
	std::string send_multiaddr;	// IP multicast address of group
	int send_multiport;			// and port

	std::string recv_interface;	// local IP address of interface
	std::string recv_multiaddr;	// IP multicast address of group
	int recv_multiport;			// and port

	SOCKET m_socket;			// socket handle

	std::map<std::string, SOCKET> m_socket_table;	// Таблица сокетов для отсылки/приема данных

	WmcastError m_socketErrno;	// number of last error

	uint8 *m_pBuffer;		// Internal send/receive buffer
	int32 m_nBufferSize;	// Size of internal send/receive buffer

	int32 m_nBytesSent;		// number of bytes sent
	int32 m_nBytesReceived;	// number of bytes received

#ifdef WIN32
	WSADATA m_hWSAData;	/// Windows
#endif

	// for Send
	struct sockaddr_in mcastGroupAddr;	// Группа с групповым адресом и портом
	struct in_addr interfaceIP;	// Локальный интерфейс для исходящих многоадресных дейтаграмм

	// for Receive
	struct sockaddr_in m_stMulticastGroup;// multicast group to bind to - адрес для получения многоадресных дейтаграмм
	struct ip_mreq m_stMulticastRequest;   /// group address for multicast

	// Initialize instance of Wmcast
	// This method MUST be called before an object can be used
	// Return true if properly initialized
	//--------------------------------------------------------
	bool Init(void);

	// Close socket
	// Return true if successfully closed otherwise returns false
	//-----------------------------------------------------------
	bool Close(void);

	//
	// Закрыть все открытые ранее сокеты
	//----------------------------------
	bool Close_Sockets(void);

	// Задать параметры многоадрессной группы
	// Set Send socket option (передатчик)
	// Return true if successfully closed otherwise returns false
	//-----------------------------------------------------------
	bool Set_Send( std::string out_interface, std::string out_multiaddr, int out_multiport);
	bool Set_Send_Opt(void);
	// Send a message to the multicast group specified by the
	// mcastGroupAddr sockaddr structure
	// ==================================================
	// Отправить сообщение в многоадресную группу,
	// указанную структурой sockaddr mcastGroupAddr
	//
	//  @param pBuf        - block of data to be sent.
	//  @param bytesToSend - size of data block to be sent.
	//
	//  @return number of bytes actually sent.
	//   возвращаемое количество байтов фактически отправлено
	//
	//  @return of zero means the connection has been shutdown on the other side.
	//   возврат нуля означает, что соединение было отключено на другой стороне
	//
	//  @return of -1 means that an error has occurred.
	//   возврат -1 означает, что произошла ошибка
	//-----------------------------------------------------------
	int32 Send(const uint8 *pBuf, size_t bytesToSend);

	// Отправить сообщение в многоадресную группу
	//-------------------------------------------
	bool Send_To_Ether(const uint8 *pBuf, size_t bytesToSend, std::string out_interface, std::string out_multiaddr, int out_multiport);

	// Задать параметры многоадрессной группы
	// Set server socket option (приемник)
	// Return true if successfully closed otherwise returns false
	//-----------------------------------------------------------
	bool Set_Recv( std::string in_interface, std::string in_multiaddr, int in_multiport);
	bool Set_Recv_Opt(void);

	// Attempts to receive a block of data on an established connection.
	//
	// @param nMaxBytes - maximum number of bytes to receive.
	// @param pBuffer   - memory where to receive the data,
	//                    NULL receives to internal buffer returned with GetData()
	//                    Non-NULL receives directly there, but GetData() will return WRONG ptr!
	//
	// @return number of bytes actually received.
	//
	//
	// @return of zero means the connection has been shutdown on the other side.
	//
	//
	// @return of -1 means that an error has occurred.
	//
	//-----------------------------------------------------------
	int32 Receive(int32 nMaxBytes = 1, uint8 *pBuffer = 0);

	// Получить сообщение для многоадресной группы
	bool Receive_From_Ether(int32 nMaxBytes, uint8 *pBuffer, std::string in_interface, std::string in_multiaddr, int in_multiport);

	// Get a pointer to internal receive buffer.  The user MUST not free this
	// pointer when finished.  This memory is managed internally by the class.
	//
	// @return pointer to data if valid, else returns NULL.
	//-----------------------------------------------------------
	uint8* GetData(void) {
		return m_pBuffer;
	}
	;

	// Returns the number of bytes received on the last call to Receive()
	//
	// @return number of bytes received
	//-----------------------------------------------------------
	int32 GetBytesReceived(void) {
		return m_nBytesReceived;
	}
	;

	// Returns the number of bytes sent on the last call to Send()
	//
	// @return number of bytes sent.
	//-----------------------------------------------------------
	int32 GetBytesSent(void) {
		return m_nBytesSent;
	}
	;

private:
	// список свойств и методов для использования внутри класса
	//-----------------------------------------------------------

	// Provides a standard error code for cross platform development by
	// mapping the operating system error to an error defined by the class
	//--------------------------------------------------------------------
	void TranslateSocketError(void);

	// Returns a human-readable description of the given error code
	// or the last error code of a socket
	//-------------------------------------------------------------
	static const char* DescribeError(WmcastError err);
	inline const char* DescribeError() {
		return DescribeError(m_socketErrno);
	}
	;

	// Set internal socket error to that specified error
	//--------------------------------------------------
	void SetSocketError(WmcastError error) {
		m_socketErrno = error;
	}
	;

	// Returns the last error that occured for the instace.
	// This method should be called immediately to retrieve
	// the error code for the failing mehtod call.
	//-----------------------------------------------------
	WmcastError GetSocketError(void) {
		return m_socketErrno;
	}
	;

	// Does the current instance of the socket object
	// contain a valid socket descriptor.
	// Return true if the socket object contains a valid socket descriptor
	//--------------------------------------------------------------------
	bool IsSocketValid(void) {
		return (m_socket != SocketError);
	}
	;

protected:
	// список средств, доступных при наследовании
	//---------------------------------------------
};

#endif /* _WMCAST_HPP_ */
