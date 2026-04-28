<?xml version="1.0" encoding="utf-8"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL"
xmlns:sac="http://www.sac-china.com"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.iec.ch/61850/2003/SCL SCL.xsd">
  <Header nameStructure="IEDName" revision="0.2" version="0.1">
    <History>
      <HItem revision="0.1" what="初始化配置文件"
      when="2026-02-04 14:10:10" who="mengqp" />
    </History>
  </Header>
  <Communication desc="Local">
    <SubNetwork desc="Local" name="MMS" type="8-MMS">
      <BitRate multiplier="M" unit="b/s" />
      <ConnectedAP apName="AP1" iedName="PXR850">
        <Address>
          <P type="OSI-AP-Title">1,1,9999,1</P>
          <P type="OSI-AE-Qualifier">12</P>
          <P type="OSI-PSEL">00000001</P>
          <P type="OSI-SSEL">0001</P>
          <P type="OSI-TSEL">0001</P>
          <P type="IP">192.189.7.132</P>
          <P type="IP-SUBNET">255.255.252.0</P>
          <P type="IP-GATEWAY">192.1.1.2</P>
        </Address>
        <GSE cbName="gsecb0" ldInst="PXR25_1">
          <Address>
            <P type="VLAN-ID">02</P>
            <P type="VLAN-PRIORITY">4</P>
            <P type="MAC-Address">01-0c-cd-01-00-01</P>
            <P type="APPID">1000</P>
          </Address>
        </GSE>
        <SMV cbName="smvcb0" ldInst="PXR25_1">
          <Address>
            <P type="VLAN-ID">01</P>
            <P type="VLAN-PRIORITY">4</P>
            <P type="MAC-Address">01-0c-cd-04-10-01</P>
            <P type="APPID">1001</P>
          </Address>
        </SMV>
      </ConnectedAP>
    </SubNetwork>
  </Communication>
  <IED desc="此处现场name可以修改" name="PXR850">
    <Services>
      <DynAssociation />
      <SettingGroups>
        <SGEdit />
        <ConfSG />
      </SettingGroups>
      <GetDirectory />
      <GetDataObjectDefinition />
      <GetDataSetValue />
      <ConfDataSet max="64" maxAttributes="256" />
      <!-- <ConfReportControl max="20" bufConf="false" /> -->
      <DataSetDirectory />
      <ReadWrite />
      <GetCBValues />
      <ConfLNs fixLnInst="true" fixPrefix="true" />
      <!-- <GOOSE max="10" /> -->
      <GSSE max="10" />
      <FileHandling />
      <GSEDir />
      <TimerActivatedControl />
    </Services>
    <AccessPoint name="AP1">
      <Server>
        <Authentication />
        <LDevice inst="PXR25">
          <LN0 lnClass="LLN0" lnType="LLN01" inst="">
            <DataSet name="realtime">
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="A" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="In" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="Ig" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="PhV" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="PPV" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="TotW" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="TotVAr" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="TotVA" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="TotPF" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="Hz" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="DmdPeakTotW" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="DmdPeakTotVAr" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="DmdPeakTotVA" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="DmdTotW" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="DmdTotVAr" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="DmdTotVA" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="Tmp" fc="MX" />
              <FCDA  prefix="" lnClass="MMXU"
              lnInst="1" doName="MaxTmp" fc="MX" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="ForwardTotWh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="ReverseTotWh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="TotWh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="LeadVArh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="LagVArh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="NetVArh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="TotVAh" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="INSTorSDT" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="LFTorGFT" fc="ST" />
              <FCDA  prefix="" lnClass="MMTR"
              lnInst="1" doName="OpeCount" fc="ST" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxAmps" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MinAmps" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxIn" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MinIn" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxIg" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MinIg" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxPhVolts" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MinPhVolts" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxPPVolts" fc="MX" />
              <FCDA  prefix="" lnClass="MSTA"
              lnInst="1" doName="MinPPVolts" fc="MX" />
              <FCDA  prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdA" fc="MX" />
              <FCDA  prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdIn" fc="MX" />
              <FCDA  prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdPhV" fc="MX" />
              <FCDA  prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdPPV" fc="MX" />
            </DataSet>
            <DataSet name="InputState">
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="BreakerInClosePos" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="Un-acknowiedgedTrip" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="ActiveOrunacknowiedgedAlarm" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="MaintenanceActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="TestActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="PhaseRotationABC1" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="LongDelayActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="ZoneInterLockActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="BreakerInConenctedPos" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="SpringInChargeState" fc="ST" />
            </DataSet>
            <DataSet name="alarm">
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="OpenCloseState" fc="ST" />
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="TrippedState" fc="ST" />
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="Alarmed" fc="ST" />
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="Ipckedup" fc="ST" />
            </DataSet>
            <ReportControl name="alarm" rptID="LLN0$RP$alarm"
            datSet="alarm" buffered="true" bufTime="1000"
            intgPd="900000" PurgeBuf="true"
            confRev="1">
              <TrgOps dchg="true" qchg="true" dupd="true"
              period="true" gi="true" />
              <OptFields seqNum="true" timeStamp="true"
              dataSet="true" reasonCode="true" dataRef="false"
              entryID="true" configRef="true" />
              <RptEnabled max="1" />
            </ReportControl>
          </LN0>
          <LN inst="1" lnClass="MMXU" lnType="MMXUPXR25" />
          <LN inst="1" lnClass="MSTA" lnType="MSTAPXR25" />
          <LN inst="1" lnClass="MMTR" lnType="MMTRPXR25" />
          <LN inst="1" lnClass="MHAI" lnType="MHAIPXR25" />
          <LN inst="1" lnClass="GGIO" lnType="GGIO1" />
          <LN inst="1" lnClass="CALH" lnType="CALH1" />
        </LDevice>
        <LDevice inst="PXR20">
          <LN0 lnClass="LLN0" lnType="LLN01" inst="">
            <DataSet name="realtime">
              <FCDA   prefix="" lnClass="MMXU"
              lnInst="1" doName="A" fc="MX" />
              <FCDA   prefix="" lnClass="MMXU"
              lnInst="1" doName="In" fc="MX" />
              <FCDA   prefix="" lnClass="MMXU"
              lnInst="1" doName="Ig" fc="MX" />
              <FCDA   prefix="" lnClass="MMXU"
              lnInst="1" doName="Hz" fc="MX" />
              <FCDA   prefix="" lnClass="MMXU"
              lnInst="1" doName="Tmp" fc="MX" />
              <FCDA   prefix="" lnClass="MMXU"
              lnInst="1" doName="MaxTmp" fc="MX" />
              <FCDA   prefix="" lnClass="MMTR"
              lnInst="1" doName="INSTorSDT" fc="ST" />
              <FCDA   prefix="" lnClass="MMTR"
              lnInst="1" doName="LFTorGFT" fc="ST" />
              <FCDA   prefix="" lnClass="MMTR"
              lnInst="1" doName="OpeCount" fc="ST" />
              <FCDA   prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxAmps" fc="MX" />
              <FCDA   prefix="" lnClass="MSTA"
              lnInst="1" doName="MinAmps" fc="MX" />
              <FCDA   prefix="" lnClass="MSTA"
              lnInst="1" doName="MaxIn" fc="MX" />
              <FCDA   prefix="" lnClass="MSTA"
              lnInst="1" doName="MinIn" fc="MX" />
              <FCDA   prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdA" fc="MX" />
              <FCDA   prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdIn" fc="MX" />
              <FCDA   prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdPhV" fc="MX" />
              <FCDA   prefix="" lnClass="MHAI"
              lnInst="1" doName="ThdPPV" fc="MX" />
            </DataSet>
            <DataSet name="InputState">
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="BreakerInClosePos" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="Un-acknowiedgedTrip" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="ActiveOrunacknowiedgedAlarm" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="MaintenanceActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="TestActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="PhaseRotationABC1" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="LongDelayActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="ZoneInterLockActive" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="BreakerInConenctedPos" fc="ST" />
              <FCDA   prefix="" lnClass="GGIO"
              lnInst="1" doName="SpringInChargeState" fc="ST" />
            </DataSet>
            <DataSet name="alarm">
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="OpenCloseState" fc="ST" />
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="TrippedState" fc="ST" />
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="Alarmed" fc="ST" />
              <FCDA   prefix="" lnClass="CALH"
              lnInst="1" doName="Ipckedup" fc="ST" />
            </DataSet>
            <ReportControl name="alarm" rptID="LLN0$RP$alarm"
            datSet="alarm" buffered="true" bufTime="1000"
            intgPd="900000" PurgeBuf="true"
            confRev="1">
              <TrgOps dchg="true" qchg="true" dupd="true"
              period="true" gi="true" />
              <OptFields seqNum="true" timeStamp="true"
              dataSet="true" reasonCode="true" dataRef="false"
              entryID="true" configRef="true" />
              <RptEnabled max="32" />
            </ReportControl>
          </LN0>
          <LN inst="1" lnClass="MMXU" lnType="MMXUPXR20" />
          <LN inst="1" lnClass="MSTA" lnType="MSTAPXR20" />
          <LN inst="1" lnClass="MMTR" lnType="MMTRPXR20" />
          <LN inst="1" lnClass="MHAI" lnType="MHAIPXR20" />
          <LN inst="1" lnClass="GGIO" lnType="GGIO1" />
          <LN inst="1" lnClass="CALH" lnType="CALH1" />
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  <DataTypeTemplates>
    <LNodeType id="LLN01" lnClass="LLN0">
      <DO name="productID" type="MV_2_POWER" />
    </LNodeType>
    <LNodeType id="MMXUPXR25" lnClass="MMXU">
      <DO desc="三相电流" name="A" type="WYE_1_phs" />
      <DO desc="零序电流" name="In" type="MV_1_HZ" />
      <DO desc="漏电流" name="Ig" type="MV_1_HZ" />
      <DO desc="三相电压" name="PhV" type="WYE_1_phs" />
      <DO desc="三相线电压" name="PPV" type="DEL_1_ppv" />
      <DO desc="总有功" name="TotW" type="MV_1_POWER" />
      <DO desc="总无功" name="TotVAr" type="MV_1_POWER" />
      <DO desc="总视在功率" name="TotVA" type="MV_1_POWER" />
      <DO desc="总功率因数" name="TotPF" type="MV_1_POWER" />
      <DO desc="hz" name="Hz" type="MV_1_HZ" />
      <DO desc="有功功率蜂值需量" name="DmdPeakTotW" type="MV_1_POWER" />
      <DO desc="无功功率蜂值需量" name="DmdPeakTotVAr" type="MV_1_POWER" />
      <DO desc="视在功率蜂值需量" name="DmdPeakTotVA" type="MV_1_POWER" />
      <DO desc="有功需量" name="DmdTotW" type="MV_1_POWER" />
      <DO desc="无功需量" name="DmdTotVAr" type="MV_1_POWER" />
      <DO desc="视在需量" name="DmdTotVA" type="MV_1_POWER" />
      <DO desc="装置温度" name="Tmp" type="MV_1_POWER" />
      <DO desc="最大温度" name="MaxTmp" type="MV_1_POWER" />
      <DO desc="主要原因" name="Primary" type="MV_2_POWER" />
      <DO desc="次要原因" name="Secondary" type="MV_2_POWER" />
      <DO desc="具体原因" name="Cause" type="MV_2_POWER" />
    </LNodeType>
    <LNodeType id="MMXUPXR20" lnClass="MMXU">
      <DO desc="三相电流" name="A" type="WYE_1_phs" />
      <DO desc="零序电流" name="In" type="MV_1_HZ" />
      <DO desc="漏电流" name="Ig" type="MV_1_HZ" />
      <DO desc="hz" name="Hz" type="MV_1_HZ" />
      <DO desc="装置温度" name="Tmp" type="MV_1_POWER" />
      <DO desc="最大温度" name="MaxTmp" type="MV_1_POWER" />
      <DO desc="主要原因" name="Primary" type="MV_2_POWER" />
      <DO desc="次要原因" name="Secondary" type="MV_2_POWER" />
      <DO desc="具体原因" name="Cause" type="MV_2_POWER" />
    </LNodeType>
    <LNodeType desc="统计电能" id="MMTRPXR25" lnClass="MMTR">
      <DO name="ForwardTotWh" type="BCR_1_Tot" />
      <DO name="ReverseTotWh" type="BCR_1_Tot" />
      <DO name="TotWh" type="BCR_1_Tot" />
      <DO desc="容性（超前）无功电能" name="LeadVArh" type="BCR_1_Tot" />
      <DO desc="是感性（滞后）无功电能" name="LagVArh" type="BCR_1_Tot" />
      <DO desc="表示感性-容性无功电能的净无功电能" name="NetVArh" type="BCR_1_Tot" />
      <DO name="TotVAh" type="BCR_1_Tot" />
      <DO name="INSTorSDT" type="BCR_1_Tot" />
      <DO name="LFTorGFT" type="BCR_1_Tot" />
      <DO name="OpeCount" type="BCR_1_Tot" />
    </LNodeType>
    <LNodeType desc="统计电能" id="MMTRPXR20" lnClass="MMTR">
      <DO name="INSTorSDT" type="BCR_1_Tot" />
      <DO name="LFTorGFT" type="BCR_1_Tot" />
      <DO name="OpeCount" type="BCR_1_Tot" />
    </LNodeType>
    <LNodeType id="MSTAPXR25" lnClass="MSTA">
      <DO desc="最大电流" name="MaxAmps" type="WYE_1_phs" />
      <DO desc="最小电流" name="MinAmps" type="WYE_1_phs" />
      <DO desc="最大零序" name="MaxIn" type="MV_1_Av" />
      <DO desc="最小零序" name="MinIn" type="MV_1_Av" />
      <DO desc="最大接地" name="MaxIg" type="MV_1_Av" />
      <DO desc="最小接地" name="MinIg" type="MV_1_Av" />
      <DO desc="最大相电压" name="MaxPhVolts" type="WYE_1_phs" />
      <DO desc="最小相电压" name="MinPhVolts" type="WYE_1_phs" />
      <DO desc="最大线电压" name="MaxPPVolts" type="DEL_1_ppv" />
      <DO desc="最小线电压" name="MinPPVolts" type="DEL_1_ppv" />
    </LNodeType>
    <LNodeType id="MSTAPXR20" lnClass="MSTA">
      <DO desc="最大电流" name="MaxAmps" type="WYE_1_phs" />
      <DO desc="最小电流" name="MinAmps" type="WYE_1_phs" />
      <DO desc="最大零序" name="MaxIn" type="MV_1_Av" />
      <DO desc="最小零序" name="MinIn" type="MV_1_Av" />
      <DO desc="最大接地" name="MaxIg" type="MV_1_Av" />
      <DO desc="最小接地" name="MinIg" type="MV_1_Av" />
    </LNodeType>
    <LNodeType desc="谐波含有率" id="MHAIPXR25" lnClass="MHAI">
      <DO name="ThdA" type="WYE_1_Thd" />
      <DO name="ThdIn" type="CMV_1_phs" />
      <DO name="ThdPhV" type="WYE_1_Thd" />
      <DO name="ThdPPV" type="DEL_1_ppv" />
    </LNodeType>
    <LNodeType desc="谐波含有率" id="MHAIPXR20" lnClass="MHAI">
      <DO name="ThdA" type="WYE_1_Thd" />
      <DO name="ThdIn" type="CMV_1_phs" />
      <DO name="ThdPhV" type="WYE_1_Thd" />
      <DO name="ThdPPV" type="DEL_1_ppv" />
    </LNodeType>
    <LNodeType id="GGIO1" lnClass="GGIO">
      <DO desc="断路器处于闭合位置" name="BreakerInClosePos"
      type="SPS_1_Ind" />
      <DO desc="未确认的脱扣状态" name="Un-acknowiedgedTrip"
      type="SPS_1_Ind" />
      <DO desc="激活或未确认的报警" name="ActiveOrunacknowiedgedAlarm"
      type="SPS_1_Ind" />
      <DO desc="维护模式被激活" name="MaintenanceActive"
      type="SPS_1_Ind" />
      <DO desc="测试模式被激活" name="TestActive" type="SPS_1_Ind" />
      <DO desc="三相电源相序" name="PhaseRotationABC1"
      type="SPS_1_Ind" />
      <DO desc="长延时整定被激活" name="LongDelayActive"
      type="SPS_1_Ind" />
      <DO desc="区域选择性联锁(ZSI)被激活" name="ZoneInterLockActive"
      type="SPS_1_Ind" />
      <DO desc="接地为返回电源接地" name="SourceGround"
      type="SPS_1_Ind" />
      <DO desc="断路器处于连接状态" name="BreakerInConenctedPos"
      type="SPS_1_Ind" />
      <DO desc="弹簧处于储能状态" name="SpringInChargeState"
      type="SPS_1_Ind" />
    </LNodeType>
    <LNodeType desc="告警状态" id="CALH1" lnClass="CALH">
      <DO desc="分合状态" name="OpenCloseState"
      type="SPS_1_GrAlm" />
      <DO desc="触发状态" name="TrippedState"
      type="SPS_1_GrAlm" />
      <DO desc="告警状态" name="Alarmed"
      type="SPS_1_GrAlm" />
      <DO desc="Pickedup" name="Ipckedup"
      type="SPS_1_GrAlm" />
    </LNodeType>
    <DOType cdc="INC" id="INC_1_Mod">
      <DA bType="Quality" fc="ST" name="q" qchg="true" />
      <DA bType="Timestamp" fc="ST" name="t" />
      <DA bType="Enum" fc="CF" name="ctlModel" type="CtlModels" />
    </DOType>
    <DOType cdc="INS" id="INS_1_Beh">
      <DA bType="INT32" dchg="true" fc="ST" name="stVal" />
      <DA bType="Quality" fc="ST" name="q" qchg="true" />
      <DA bType="Timestamp" fc="ST" name="t" />
    </DOType>
    <DOType cdc="LPL" id="LPL_1_NamPlt">
      <DA bType="VisString255" fc="DC" name="vendor" />
      <DA bType="VisString255" fc="DC" name="swRev" />
      <DA bType="VisString255" fc="DC" name="d" />
      <DA bType="VisString255" fc="DC" name="configRev" />
      <DA bType="VisString255" fc="EX" name="ldNs" />
    </DOType>
    <DOType cdc="SPS" id="SPS_1_GrAlm">
      <DA bType="BOOLEAN" dchg="true" fc="ST" name="stVal" />
      <DA bType="Timestamp" fc="ST" name="t" />
    </DOType>
    <DOType cdc="WYE" desc="phase voltage" id="WYE_1_phs">
      <SDO name="phsA" type="CMV_1_phs" />
      <SDO name="phsB" type="CMV_1_phs" />
      <SDO name="phsC" type="CMV_1_phs" />
    </DOType>
    <DOType cdc="WYE" desc="phase voltage" id="WYE_2_phs">
      <SDO name="phsA" type="CMV_2_phs" />
      <SDO name="phsB" type="CMV_2_phs" />
      <SDO name="phsC" type="CMV_2_phs" />
    </DOType>
    <DOType cdc="WYE" desc="phase voltage" id="WYE_1_Thd">
      <SDO name="phsA" type="CMV_1_phs" />
      <SDO name="phsB" type="CMV_1_phs" />
      <SDO name="phsC" type="CMV_1_phs" />
    </DOType>
    <DOType cdc="BCR" id="BCR_1_Tot">
      <DA bType="INT64" dchg="true" fc="ST" name="actVal" />
      <!-- <DA bType="Timestamp" fc="ST" name="t" /> -->
    </DOType>
    <DOType cdc="WYE" desc="phase voltage" id="DEL_1_ppv">
      <SDO name="ppvAB" type="CMV_1_phs" />
      <SDO name="ppvBC" type="CMV_1_phs" />
      <SDO name="ppvCA" type="CMV_1_phs" />
    </DOType>
    <DOType cdc="MV" id="MV_1_HZ">
      <DA bType="Struct" dchg="true" fc="MX" name="mag"
      type="AnalogueValue_1" />
    </DOType>
    <DOType cdc="MV" id="MV_1_Av">
      <DA bType="Struct" dchg="true" fc="MX" name="mag"
      type="AnalogueValue_1" />
    </DOType>
    <DOType cdc="MV" id="MV_1_POWER">
      <DA bType="Struct" dchg="true" fc="MX" name="mag"
      type="AnalogueValue_1" />
    </DOType>
    <DOType cdc="MV" id="MV_2_POWER">
      <DA bType="Struct" dchg="true" fc="MX" name="mag"
      type="AnalogueValue_2" />
    </DOType>
    <DOType cdc="MV" id="MV_1_imb">
      <DA bType="Struct" dchg="true" fc="MX" name="mag"
      type="AnalogueValue_1" />
      <DA bType="Quality" fc="MX" name="q" qchg="true" />
      <DA bType="Timestamp" fc="MX" name="t" />
    </DOType>
    <DOType cdc="CMV" id="CMV_1_phs">
      <DA bType="Struct" dchg="true" fc="MX" name="cVal"
      type="Vector_1" />
    </DOType>
    <DOType cdc="CMV" id="CMV_2_phs">
      <DA bType="Struct" dchg="true" fc="MX" name="cVal"
      type="Vector_2" />
    </DOType>
    <DOType cdc="SPS" id="SPS_1_Ind">
      <DA bType="BOOLEAN" dchg="true" fc="ST" name="stVal" />
      <!-- <DA bType="Timestamp" fc="ST" name="t" /> -->
    </DOType>
    <DOType cdc="SEQ" id="SEQ_1">
      <SDO name="c1" type="CMV_1_SEQ" />
      <SDO name="c2" type="CMV_1_SEQ" />
      <SDO name="c3" type="CMV_1_SEQ" />
    </DOType>
    <DOType cdc="CMV" id="CMV_1_SEQ">
      <DA bType="Struct" fc="MX" name="cVal"
      type="AnalogueValue_1" />
    </DOType>
    <DOType cdc="ASG" id="ASG_SE_RATE">
      <DA bType="Struct" fc="SE" name="setMag"
      type="AnalogueValue_1" />
    </DOType>
    <DOType cdc="ASG" id="ASG_SE_RATE_2">
      <DA bType="Struct" fc="SE" name="setMag"
      type="AnalogueValue_2" />
    </DOType>
    <DAType id="Vector_1">
      <BDA bType="Struct" name="mag" type="AnalogueValue_1" />
    </DAType>
    <DAType id="Vector_2">
      <BDA bType="Struct" name="mag" type="AnalogueValue_2" />
    </DAType>
    <DAType id="AnalogueValue_1">
      <BDA bType="FLOAT32" name="f" />
    </DAType>
    <DAType id="AnalogueValue_2">
      <BDA bType="INT32" name="i" />
    </DAType>
    <EnumType id="CtlModels">
      <EnumVal ord="0">status-only</EnumVal>
      <EnumVal ord="1">direct-with-normal-security</EnumVal>
      <EnumVal ord="2">sbo-with-normal-security</EnumVal>
      <EnumVal ord="3">direct-with-enhanced-security</EnumVal>
      <EnumVal ord="4">sbo-with-enhanced-security</EnumVal>
    </EnumType>
  </DataTypeTemplates>
</SCL>
