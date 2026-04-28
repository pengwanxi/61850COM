/*
 *  iso_connection_parameters.c
 *
 *  IsoConnectionParameters abstract data type to represent the configurable parameters of the ISO protocol stack.
 *
 *  Copyright 2013, 2014 Michael Zillgith
 *
 *  This file is part of libIEC61850.
 *
 *  libIEC61850 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libIEC61850 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libIEC61850.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See COPYING file for the complete license text.
 */

#include "libiec61850_platform_includes.h"

#include "stack_config.h"

#include "iso_connection_parameters.h"

#include "ber_encoder.h"

	AcseAuthenticationParameter
AcseAuthenticationParameter_create()
{/*{{{*/
	AcseAuthenticationParameter self = (AcseAuthenticationParameter)
		GLOBAL_CALLOC(1, sizeof(struct sAcseAuthenticationParameter));

	return self;
}/*}}}*/

	void
AcseAuthenticationParameter_destroy(AcseAuthenticationParameter self)
{/*{{{*/
	if (self->mechanism == ACSE_AUTH_PASSWORD)
		if (self->value.password.octetString != NULL)
			GLOBAL_FREEMEM(self->value.password.octetString);

	GLOBAL_FREEMEM(self);
}/*}}}*/

	void
AcseAuthenticationParameter_setPassword(AcseAuthenticationParameter self, char* password)
{/*{{{*/
	self->value.password.octetString = (uint8_t*) StringUtils_copyString(password);
	self->value.password.passwordLength = strlen(password);
}/*}}}*/

	void
AcseAuthenticationParameter_setAuthMechanism(AcseAuthenticationParameter self, AcseAuthenticationMechanism mechanism)
{/*{{{*/
	self->mechanism = mechanism;
}/*}}}*/

/*
 * description:
 * 		create a new IsoConnectionParameters instance (FOR LIBRARY INTERNAL USE)
 * 		NOTE:This function used internally by the MMS client library.When using
 * 		the MMS or IEC 61850 API there shoule be no reason for the user to call
 * 		this function.
 * return:
 * 		new IsoConnectionParameters instanceextern "C"{
 */
	IsoConnectionParameters
IsoConnectionParameters_create()
{/*{{{*/
	IsoConnectionParameters self = (IsoConnectionParameters) GLOBAL_CALLOC(1, sizeof(struct sIsoConnectionParameters));

	return self;
}/*}}}*/

	void
IsoConnectionParameters_destroy(IsoConnectionParameters self)
{/*{{{*/
	GLOBAL_FREEMEM(self);
}/*}}}*/

	void
IsoConnectionParameters_setAcseAuthenticationParameter(IsoConnectionParameters self,
		AcseAuthenticationParameter acseAuthParameter)
{/*{{{*/
	self->acseAuthParameter = acseAuthParameter;
}/*}}}*/

	void
IsoConnectionParameters_setTcpParameters(IsoConnectionParameters self, const char* hostname, int tcpPort)
{/*{{{*/
	self->hostname = hostname;
	self->tcpPort = tcpPort;
}/*}}}*/

/*
 * description:
 * 		set the remote AP-Title and AE-Qualifier
 * 		Calling this function is optional and not recommended.If not called the
 * 		default parameters are used.If apTitle is NULL the parameter the
 * 		AP-Title and AE-Qualifier will not be transmitted.This seems to be
 * 		required by some server devices.
 * parameters:
 * 		self:	the IsoConnectionParameters instance.
 * 		apTitle:	the AP-Title OID as string.
 * 		aeQualifier:	the AP-qualifier
 */
	void
IsoConnectionParameters_setRemoteApTitle(IsoConnectionParameters self, const char* apTitle, int aeQualifier)
{/*{{{*/
	if (apTitle == NULL)
		self->remoteApTitleLen = 0;
	else {
		self->remoteApTitleLen = BerEncoder_encodeOIDToBuffer(apTitle, self->remoteApTitle, 10);
		self->remoteAEQualifier = aeQualifier;
	}
}/*}}}*/

/*
 * description:
 * 		set remote address for the lower layers
 * 		This function can be used to set the addresses for the lower layer
 * 		protocols (presentation,session,and transport layer).Calling this
 * 		function is optional and not recommended.If not called the default
 * 		parameters are used.
 * parameters:
 * 		self:	the IsoConnectionParameters instance.
 * 		pSelector:	the P-Selector (presentation layer address)
 * 		sSelector:	the S-Selector (session layer address)
 * 		tSelector:	the T-Selector (ISO transport layer address)
 */
	void
IsoConnectionParameters_setRemoteAddresses(IsoConnectionParameters self, uint32_t pSelector, SSelector sSelector, TSelector tSelector)
{/*{{{*/
	self->remotePSelector = pSelector;
	self->remoteSSelector = sSelector;
	self->remoteTSelector = tSelector;
}/*}}}*/

/*
 * description:
 * 		set local AP-Title and AE-Qualifier
 * 		Calling this function is optional and not recommended.If not called the
 * 		default parameters are used.If apTitle is NULL the parameter the
 * 		AP-Title and AE-Qualifier will not be transmitted.This seems to be
 * 		required by some server devices.
 * parameters:
 * 		self:	the IsoConnectionParameters instance.
 * 		apTitle:	the AP-Title OID as string.
 * 		aeQualifier:the AP-qualifier
 */
	void
IsoConnectionParameters_setLocalApTitle(IsoConnectionParameters self, char* apTitle, int aeQualifier)
{/*{{{*/
	if (apTitle == NULL)
		self->localApTitleLen = 0;
	else {
		self->localApTitleLen = BerEncoder_encodeOIDToBuffer(apTitle, self->localApTitle, 10);
		self->localAEQualifier = aeQualifier;
	}
}/*}}}*/

/*
 * description:
 * 		set local addresses for the lower layers
 * 		This function can be used to set the addresses for the lower layer
 * 		protocols (presentation,session,and transport layer).Calling this
 * 		function is optional and not recommended.If not called the default
 * 		parameters are used.
 * parameters:
 * 		self:	the IsoConnectionParameters instance
 * 		pSelector:	the P-Selector (presentation layer address)
 * 		sSelector:	the S-Selector (presentation layer address)
 * 		tSelector:	the T-Selector (ISO transport layer address)
 */
	void
IsoConnectionParameters_setLocalAddresses(IsoConnectionParameters self, uint32_t pSelector, SSelector sSelector, TSelector tSelector)
{/*{{{*/
	self->localPSelector = pSelector;
	self->localSSelector = sSelector;
	self->localTSelector = tSelector;
}/*}}}*/
