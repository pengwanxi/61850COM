#ifndef HIPULSEU_H_
#define HIPULSEU_H_

#include "CProtocol_UpsMaster.h"

#define UPSMSGFALSE (1)
#define UPSMSGTRUE (0)

#define HIGBYTE(x)	(((x) >> 4) & 0xF)
#define LOWBYTE(x)	((x) & 0xF)
#define UPSMASTER_HIPULSEU_MAX_POX			7		//报文发送类别最多个数


class CUpsHipulse : public CProtocol_UpsMaster
{
public:
	CUpsHipulse();

	virtual ~CUpsHipulse();

	int m_iSendFlag;
	int m_iInitFlag;
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	double m_dVersion;
	BYTE m_byVersion[2];
	int m_iYxRunStateNum;
	int m_iYxWarnStateNum;

	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;

	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;

	virtual BOOL Init( BYTE byLineNo  ) ;

	virtual void TimerProc() ;
	//获得装置通讯状态
	virtual BOOL GetDevCommState( ) ;

	void ChangeSendPos(void);

	WORD GetCrc(BYTE *buf, int len);

	BOOL WhetherBufValue(BYTE *buf, int &len);

	BOOL UPSQueryStatePack(BYTE * buf , int &len);

	BOOL UPSQueryProtocolVersion(BYTE * buf , int &len);

	BOOL UPSQueryTotalStandardAnalogData(BYTE * buf, int &len); //序号0-10 为电总标准模拟量 

	BOOL UPSQueryUserDefinedAnalogData1(BYTE * buf, int &len);//序号11-23 为自定义模拟量量化数据1

	BOOL UPSQueryUserDefinedAnalogData2(BYTE * buf, int &len);//序号24-38 为自定义模拟量量化数据2

	BOOL UPSQueryUserDefinedAnalogData3(BYTE * buf, int &len);//序号39-54 为自定义模拟量量化数据3

	BOOL UPSQueryAlarmInfo(BYTE * buf , int &len);

	BYTE UPSCharToHex(BYTE bHex);

	BYTE UPSHexToChar(BYTE bChar);

	BOOL UPSParseStatePack(BYTE *buf, int len);

	BOOL UPSParseProtocolVersion(BYTE *buf, int len);

	BOOL UPSParseUserDefinedAnalogData1(BYTE *buf, int len);//解析设备回复自定义模拟量化信息报文1

	BOOL UPSParseUserDefinedAnalogData2(BYTE *buf, int len);//解析设备回复自定义模拟量化信息报文2

	BOOL UPSParseUserDefinedAnalogData3(BYTE *buf, int len);//解析设备回复自定义模拟量化信息报文3

	BOOL UPSParseTotalStandardAnalogData(BYTE *buf, int len);//解析电总标准模拟量

	BOOL UPSParseAlarmInfo(BYTE *buf, int len);

	float UPSParseFloat(BYTE *buf);

	int UPSFactorial(int iNum);

	BYTE UPSParseByte(BYTE *buf);

	BOOL UPSPowerSupplyMode(BYTE YxVal);

	BOOL UPSPowerSelfCheck(BYTE YxVal);

	BOOL UPSPowerAllFloat(BYTE YxVal);

	BOOL UPSPowerStartUpShutDown(BYTE YxVal);

	BOOL UPSPowerSupply(BYTE YxVal);

	BOOL UPSPowerGeneratorAccess(BYTE YxVal);

	BOOL UPSPowerInputSwitchState(BYTE YxVal);

	BOOL UPSPowerRepairBypassSwitchState(BYTE YxVal);

	BOOL UPSPowerBypassSwitchState(BYTE YxVal);

	BOOL UPSPowerOutputSwitchState(BYTE YxVal);

	BOOL UPSPowerSupplyStateOfParallelSystem(BYTE YxVal);

	BOOL UPSPowerRotarySwitchState(BYTE YxVal);

	BOOL UPSParseInverter(BYTE YxVal);

	BOOL UPSPowerFilterState(BYTE YxVal);

	BOOL UPSDormantState(BYTE YxVal);

	BOOL UPSRectifierOperationState(BYTE YxVal);

	BOOL UPSECOState(BYTE YxVal);

	BOOL UPSParseMainVoltageAbnormal(BYTE YxVal);

	BOOL UPSParseRectifierLock(BYTE YxVal);

	BOOL UPSParseInverterOutputVoltage(BYTE YxVal);

	BOOL UPSParseBypassSituation(BYTE YxVal);

	BOOL UPSParseTotalVoltageState(BYTE YxVal);

	BOOL UPSParseMainFirequency(BYTE YxVal);

	BOOL UPSParseMainFuseBroken(BYTE YxVal);

	BOOL UPSParseMainReverseOrder(BYTE YxVal);

	BOOL UPSParseMainPhaseFault(BYTE YxVal);

	BOOL UPSParseAuxiliaryPowerSupplyOne(BYTE YxVal);

	BOOL UPSParseAuxiliaryPowerSupplyTow(BYTE YxVal);

	BOOL UPSParseRectifierLimit(BYTE YxVal);

	BOOL UPSParseSoftBoot(BYTE YxVal);

	BOOL UPSParseRectifierOverTemperature(BYTE YxVal);

	BOOL UPSParseInputFilterFail(BYTE YxVal);

	BOOL UPSParseFilterOverFlow(BYTE YxVal);

	BOOL UPSParseFilterFail(BYTE YxVal);

	BOOL UPSParseFilterDriveCableFail(BYTE YxVal);

	BOOL UPSParseRectifierComFail(BYTE YxVal);

	BOOL UPSParseInverterOverTemperature(BYTE YxVal);

	BOOL UPSParseFanFail(BYTE YxVal);

	BOOL UPSParseInverterThyristorFail(BYTE YxVal);

	BOOL UPSParseBypassThyristorFail(BYTE YxVal);

	BOOL UPSParseUserOperationFail(BYTE YxVal);

	BOOL UPSParseSingleOutputOverload(BYTE YxVal);

	BOOL UPSParseParallelSystemOverload(BYTE YxVal);

	BOOL UPSParseSingleOverloadTimeOut(BYTE YxVal);

	BOOL UPSParseBypassAbNormalShutdown(BYTE YxVal);

	BOOL UPSParseAcOutputOverpressure(BYTE YxVal);

	BOOL UPSParseInverterOverflow(BYTE YxVal);

	BOOL UPSParseBypassReverse(BYTE YxVal);

	BOOL UPSParseLoadShock(BYTE YxVal);

	BOOL UPSParseBypassSwitchLimit(BYTE YxVal);

	BOOL UPSParseParallelEqualFail(BYTE YxVal);

	BOOL UPSParseBusAbnormalShutdown(BYTE YxVal);

	BOOL UPSParseNeighborBypass(BYTE YxVal);

	BOOL UPSParseParallelPlateFail(BYTE YxVal);

	BOOL UPSParseParallelConnectFail(BYTE YxVal);

	BOOL UPSParseParallelComFail(BYTE YxVal);

	BOOL UPSParseBypassOverFlowFail(BYTE YxVal);

	BOOL UPSParseLBSActivation(BYTE YxVal);

	BOOL UPSParseBypassInductorOverTemperature(BYTE YxVal);

	BOOL UPSParseStaticSwitchOverTemperature(BYTE YxVal);

	BOOL UPSParseBypassReverseFail(BYTE YxVal);

	BOOL UPSParseInverterDriveCableFail(BYTE YxVal);

	BOOL UPSParseInverterComFail(BYTE YxVal);

	BOOL UPSParseParallelSystemBatteryFail(BYTE YxVal);

	BOOL UPSParseEmergencyShutdown(BYTE YxVal);

	BOOL UPSParseAmbientTemperatureHigh(BYTE YxVal);

	BOOL UPSParseBatteryLife(BYTE YxVal);

	BOOL UPSParseBatteryTemperatureHigh(BYTE YxVal);

	BOOL UPSParseBatteryGroundFail(BYTE YxVal);

	BOOL UPSParseBatteryFuse(BYTE YxVal);

	BOOL UPSParseBCBInput(BYTE YxVal);

	BOOL UPSParseOutputFuse(BYTE YxVal);

	BOOL UPSParseBusCapOvervoltage(BYTE YxVal);

	BOOL UPSParseBusOvervoltage(BYTE YxVal);

	BOOL UPSParseBusShortCircultFail(BYTE YxVal);

	BOOL UPSParseInputFlowUnbalance(BYTE YxVal);

	BOOL UPSParseOutputCapMaintain(BYTE YxVal);

	BOOL UPSParseFilterCutoffTimes(BYTE YxVal);

	BOOL UPSBatterySRCFailure(BYTE YxVal);

	BOOL UPSPFCModeException(BYTE YxVal);

	BOOL UPSEqualizingTimeout(BYTE YxVal);

	BOOL UPSYxRunStateValTwo(BYTE YxVal);

	BOOL UPSYxRunStateValThree(BYTE YxVal);

	BOOL UPSYxRunStateValFour(BYTE YxVal);

	BOOL UPSYxRunStateValSix(BYTE YxVal);

	BOOL UPSYxWarnStateValTwoSync(BYTE YxVal);

	BOOL UPSYxWarnStateValTwoUnusual(BYTE YxVal);

	BOOL UPSYxWarnStateValTwoOverrun(BYTE YxVal);

	BOOL UPSYxWarnStateValThreeUnusual(BYTE YxVal);

	BOOL UPSYxWarnStateValSix(BYTE YxVal);
};


#endif
