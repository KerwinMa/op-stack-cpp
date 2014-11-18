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
#include <openpeer/services/ITCPMessaging.h>

namespace openpeer
{
  namespace stack
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IServerMessaging
    #pragma mark

    interaction IServerMessaging
    {
      ZS_DECLARE_TYPEDEF_PTR(services::ITransportStream, ITransportStream)

      enum SessionStates
      {
        SessionState_Pending,
        SessionState_Connected,
        SessionState_ShuttingDown,
        SessionState_Shutdown,
      };

      static const char *toString(SessionStates state);

      static ElementPtr toDebug(IServerMessagingPtr channel);

      //-----------------------------------------------------------------------
      // PURPOSE: create a new channel to a remote connection
      static IServerMessagingPtr connect(
                                         IServerMessagingDelegatePtr delegate,
                                         IPeerFilesPtr peerFiles,
                                         ITransportStreamPtr receiveStream,
                                         ITransportStreamPtr sendStream,
                                         bool messagesHaveChannelNumber,
                                         const ServerList &possibleServers,
                                         size_t maxMessageSizeInBytes = OPENPEER_SERVICES_ITCPMESSAGING_MAX_MESSAGE_SIZE_IN_BYTES
                                         );

      //-----------------------------------------------------------------------
      // PURPOSE: get process unique ID for object
      virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: subscribe to class events
      virtual IServerMessagingSubscriptionPtr subscribe(IServerMessagingDelegatePtr delegate) = 0;  // passing in IMessageLayerSecurityChannelDelegatePtr() will return the default subscription

      //-----------------------------------------------------------------------
      // PURPOSE: disconnect from the server
      virtual void shutdown(Seconds lingerTime = Seconds(OPENPEER_SERVICES_CLOSE_LINGER_TIMER_IN_SECONDS)) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: return the current state of the connection
      virtual SessionStates getState(
                                     WORD *outLastErrorCode = NULL,
                                     String *outLastErrorReason = NULL
                                     ) const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: enable TCP keep alives
      virtual void enableKeepAlive(bool enable = true) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Gets the IP address of the remotely connected socket
      virtual IPAddress getRemoteIP() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Set the maximum size of a message expecting to receive
      virtual void setMaxMessageSizeInBytes(size_t maxMessageSizeInBytes) = 0;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IServerMessagingDelegate
    #pragma mark

    interaction IServerMessagingDelegate
    {
      typedef IServerMessaging::SessionStates SessionStates;

      //-----------------------------------------------------------------------
      // PURPOSE: Notifies the delegate that the state of the connection
      //          has changed.
      // NOTE:    If the cryptographic keying is discovered to be incorrect
      //          the channel will shutdown and getState() will return an
      //          error code.
      virtual void onServerMessagingStateChanged(
                                                 IServerMessagingPtr channel,
                                                 SessionStates state
                                                 ) = 0;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IServerMessagingSubscription
    #pragma mark

    interaction IServerMessagingSubscription
    {
      virtual PUID getID() const = 0;

      virtual void cancel() = 0;

      virtual void background() = 0;
    };

  }
}

ZS_DECLARE_PROXY_BEGIN(openpeer::stack::IServerMessagingDelegate)
ZS_DECLARE_PROXY_TYPEDEF(openpeer::stack::IServerMessagingPtr, IServerMessagingPtr)
ZS_DECLARE_PROXY_TYPEDEF(openpeer::stack::IServerMessagingDelegate::SessionStates, SessionStates)
ZS_DECLARE_PROXY_METHOD_2(onServerMessagingStateChanged, IServerMessagingPtr, SessionStates)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(openpeer::stack::IServerMessagingDelegate, openpeer::stack::IServerMessagingSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(openpeer::stack::IServerMessagingPtr, IServerMessagingPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(openpeer::stack::IServerMessagingDelegate::SessionStates, SessionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onServerMessagingStateChanged, IServerMessagingPtr, SessionStates)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
