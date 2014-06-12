/*

 Copyright (c) 2014, Hookflash Inc.
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

#include <map>

namespace openpeer
{
  namespace stack
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IServicePeerFileLookupQuery
    #pragma mark

    interaction IServicePeerFileLookupQuery
    {
      typedef String PeerURI;

      typedef std::list<PeerURI> PeerFileLookupList;

      static ElementPtr toDebug(IServicePeerFileLookupQueryPtr query);

      static IServicePeerFileLookupQueryPtr fetchPeerFiles(
                                                           IServicePeerFileLookupQueryDelegatePtr delegate,
                                                           const PeerFileLookupList &peerFilesToLookup
                                                           );

      virtual PUID getID() const = 0;

      virtual bool isComplete(
                              WORD *outErrorCode = NULL,
                              String *outErrorReason = NULL
                              ) const = 0;

      virtual void cancel() = 0;

      virtual IPeerFilePublicPtr getPeerFileByPeerURI(const String &peerURI) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IServicePeerFileLookupQueryDelegate
    #pragma mark

    interaction IServicePeerFileLookupQueryDelegate
    {
      virtual void onServicePeerFileLookupQueryCompleted(IServicePeerFileLookupQueryPtr query) = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(openpeer::stack::IServicePeerFileLookupQueryDelegate)
ZS_DECLARE_PROXY_TYPEDEF(openpeer::stack::IServicePeerFileLookupQueryPtr, IServicePeerFileLookupQueryPtr)
ZS_DECLARE_PROXY_METHOD_1(onServicePeerFileLookupQueryCompleted, IServicePeerFileLookupQueryPtr)
ZS_DECLARE_PROXY_END()
