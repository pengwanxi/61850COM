#include "WebSocket.h"
#include "../share/global.h"
#include "UpdPrintMsg.h"
#include "../share/cmsg.h"
#include "../BayLayer/main.h"

CMsg  g_printMsgQueue;

CWebSocket::CWebSocket()
{
	m_pTcpPrintMsg = new CTcpPrintMsg;
	const DWORD channel = 20190830;
	int msgid = g_printMsgQueue.CreateMsgQueue(channel);
	m_pTcpPrintMsg->Init(50000, this);
	printf("%d %s \n", __LINE__, __FILE__);
	init(); //����������Ĳ��� 
}


CWebSocket::~CWebSocket()
{
	g_printMsgQueue.CloseMsgQueue();
}

void CWebSocket::on_message(CWebSocket* This, websocketpp::connection_hdl hdl, message_ptr msg)
{
	//std::cout << "on_message called with hdl: " << hdl.lock().get()
	//	<< " and message: " << msg->get_payload()
	//	<< std::endl;

	// check for a special command to instruct the server to stop listening so
	// it can be cleanly exited.
	//if (msg->get_payload() == "stop-listening") {
	//	This->echo_server.stop_listening();
	//	return;
	//}

	try {
	//	This->echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
		This->printUpdComMsg(hdl, msg );
	}
	catch (websocketpp::exception const & e) {
		std::cout << "Echo failed because: "
			<< "(" << e.what() << ")" << std::endl;
	}
}

void CWebSocket::on_fail(server* s, websocketpp::connection_hdl hdl)
{
     server::connection_ptr con = s->get_con_from_hdl(hdl);

     std::cout << "Fail handler: " << con->get_ec() << " " << con->get_ec().message() << std::endl;
}

void CWebSocket::on_close(websocketpp::connection_hdl hdl )
{
     std::cout << "Close handler" << std::endl;
}

void CWebSocket::printUpdComMsg(websocketpp::connection_hdl hdl, message_ptr msg)
{
	const char * pRecv = msg->get_payload().c_str();
	char pBus[5] = { 0 };
	memcpy(pBus, pRecv, 5);
	printf(pBus);
	printf("\n");

	bool bBegin = FALSE;
	if (memcmp(pBus, "Bus" , 3 ) == 0)
	{
		bBegin = TRUE;
	}
	else
		bBegin = FALSE;


	char pBusNo[3] = { 0 };
	memcpy(pBusNo, pRecv + 3, strlen(pRecv) - 3);
	WORD wBusNo = atoi(pBusNo);

	m_hdl = hdl;
	m_msg = msg;

	sendMsg(wBusNo, bBegin );
}

//bBegin == 1 Ϊopen 0Ϊclose
void CWebSocket::sendMsg(WORD wBusNo , BOOL bBegin )
{
	printf("%d %d\n", wBusNo , bBegin);

	const BYTE PRINT_UDP_MSG = 1;

	DWORD dwMsg = 0;
	BYTE byFirst = PRINT_UDP_MSG;
	BYTE bySec = bBegin;
	BYTE bythird = wBusNo & 0xFF;
	BYTE byfourth = (wBusNo >> 8) & 0xFF;
	WORD lw = MAKEWORD(byFirst, bySec);
	WORD hw = MAKEWORD(bythird, byfourth);
	dwMsg = MAKELONG(lw, hw);

	LMSG msg;
	msg.pVoid = ( LPVOID )dwMsg ;
	bool b = g_printMsgQueue.SendMsg(&msg);

}

DWORD CWebSocket::send(char * buf, int size)
{
	if (size == 0)
		return  0;

	try
	{
		server::connection_ptr con = echo_server.get_con_from_hdl(m_hdl);
		if (con->get_state() == 1)
		{
			echo_server.send(m_hdl, buf, m_msg->get_opcode());
		}
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Echo failed because: "
			<< "(" << e.what() << ")" << __LINE__ << std::endl;
		return 1;
	}
	return  0;
}

void CWebSocket::init()
{
	try {		
		echo_server.set_access_channels(websocketpp::log::alevel::debug_close);
		echo_server.init_asio();

		// Set SO_REUSEADDR option before binding
        echo_server.set_reuse_addr(true);
		
		// Register our message handler
		echo_server.set_message_handler(bind(&on_message, this, ::_1, ::_2));
		echo_server.set_fail_handler(bind(&on_fail, &echo_server, ::_1));
		echo_server.set_close_handler(&on_close);

		// Listen on port 9002
		echo_server.listen(boost::asio::ip::tcp::v4(), 9002);

		// Start the server accept loop
		echo_server.start_accept();

		// Start the ASIO io_service run loop
		echo_server.run();
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << __LINE__ << std::endl;
	}
	catch (...) {
		std::cout << "other exception" << std::endl;
	}
}
