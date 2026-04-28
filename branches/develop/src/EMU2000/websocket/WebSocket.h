#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include "../share/global.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::lib::asio::ip::tcp;
typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;
class CTcpPrintMsg;

class CWebSocket
{
public:
	CWebSocket();
	~CWebSocket();
	static void on_message( CWebSocket* This, websocketpp::connection_hdl hdl, message_ptr msg);
	static void on_fail(server* s, websocketpp::connection_hdl hdl);
	static void on_close(websocketpp::connection_hdl);
	void init();
	void printUpdComMsg(websocketpp::connection_hdl hdl, message_ptr msg);

	void sendMsg(WORD wBusNo, BOOL bBegin);
	DWORD send(char * buf, int size);
	server echo_server;
	CTcpPrintMsg * m_pTcpPrintMsg;
	websocketpp::connection_hdl m_hdl;
	message_ptr m_msg;
};

