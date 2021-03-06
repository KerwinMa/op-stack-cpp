/*

 Copyright (c) 2013, SMB Phone Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#pragma once

#include <openpeer/stack/types.h>
#include <openpeer/stack/message/types.h>

#include <openpeer/services/IICESocket.h>
#include <openpeer/services/IHTTP.h>


namespace openpeer
{
  namespace stack
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IServiceCertificates
    #pragma mark

    interaction IAccount
    {
      enum AccountStates
      {
        AccountState_Pending,
        AccountState_Ready,
        AccountState_ShuttingDown,
        AccountState_Shutdown,
      };

      static const char *toString(AccountStates state);

      static ElementPtr toDebug(IAccountPtr account);

      static IAccountPtr create(
                                IAccountDelegatePtr delegate,
                                IServiceLockboxSessionPtr peerContactSession
                                );

      virtual PUID getID() const = 0;

      virtual AccountStates getState(
                                     WORD *outLastErrorCode = NULL,
                                     String *outLastErrorReason = NULL
                                     ) const = 0;

      virtual IServiceLockboxSessionPtr getLockboxSession() const = 0;

      virtual void getNATServers(
                                 IICESocket::TURNServerInfoList &outTURNServers,
                                 IICESocket::STUNServerInfoList &outSTUNServers
                                 ) const = 0;

      virtual void shutdown() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IAccountDelegate
    #pragma mark

    interaction IAccountDelegate
    {
      typedef IAccount::AccountStates AccountStates;

      virtual void onAccountStateChanged(
                                         IAccountPtr account,
                                         AccountStates state
                                         ) = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(openpeer::stack::IAccountDelegate)
ZS_DECLARE_PROXY_METHOD_2(onAccountStateChanged, openpeer::stack::IAccountPtr, openpeer::stack::IAccount::AccountStates)
ZS_DECLARE_PROXY_END()
