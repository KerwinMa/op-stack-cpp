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

#include <openpeer/stack/internal/types.h>
#include <openpeer/stack/IBootstrappedNetwork.h>
#include <openpeer/stack/IServiceSalt.h>
#include <openpeer/stack/IMessageMonitor.h>
#include <openpeer/stack/message/peer-salt/SignedSaltGetResult.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/MessageQueueAssociator.h>

namespace openpeer
{
  namespace stack
  {
    namespace internal
    {
      interaction IBootstrappedNetworkForServices;

      ZS_DECLARE_USING_PTR(message::peer_salt, SignedSaltGetResult)

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ServiceSaltFetchSignedSaltQuery
      #pragma mark

      class ServiceSaltFetchSignedSaltQuery : public Noop,
                                              public zsLib::MessageQueueAssociator,
                                              public IServiceSaltFetchSignedSaltQuery,
                                              public IBootstrappedNetworkDelegate,
                                              public IMessageMonitorResultDelegate<SignedSaltGetResult>
      {
      public:
        friend interaction IServiceSaltFetchSignedSaltQueryFactory;
        friend interaction IServiceSaltFetchSignedSaltQuery;

        ZS_DECLARE_TYPEDEF_PTR(IBootstrappedNetworkForServices, UseBootstrappedNetwork)

        typedef message::peer_salt::SaltBundleList SaltBundleList;

      protected:
        ServiceSaltFetchSignedSaltQuery(
                                        IMessageQueuePtr queue,
                                        IServiceSaltFetchSignedSaltQueryDelegatePtr delegate,
                                        IServiceSaltPtr serviceSalt,
                                        UINT totalToFetch
                                        );
        
        ServiceSaltFetchSignedSaltQuery(Noop) : Noop(true), MessageQueueAssociator(IMessageQueuePtr()) {};
        
        void init();

      public:
        ~ServiceSaltFetchSignedSaltQuery();

        static ServiceSaltFetchSignedSaltQueryPtr convert(IServiceSaltFetchSignedSaltQueryPtr query);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceSaltFetchSignedSaltQuery => IServiceSaltFetchSignedSaltQuery
        #pragma mark

        static ElementPtr toDebug(IServiceSaltFetchSignedSaltQueryPtr query);

        static ServiceSaltFetchSignedSaltQueryPtr fetchSignedSalt(
                                                                  IServiceSaltFetchSignedSaltQueryDelegatePtr delegate,
                                                                  IServiceSaltPtr serviceSalt,
                                                                  UINT totalToFetch = 1
                                                                  );

        virtual PUID getID() const {return mID;}

        virtual IServiceSaltPtr getService() const;

        virtual bool isComplete() const;
        virtual bool wasSuccessful(
                                   WORD *outErrorCode = NULL,
                                   String *outErrorReason = NULL
                                   ) const;

        virtual UINT getTotalSignedSaltsAvailable() const;
        virtual ElementPtr getNextSignedSalt();

        virtual void cancel();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceSaltFetchSignedSaltQuery => IBootstrappedNetworkDelegate
        #pragma mark

        virtual void onBootstrappedNetworkPreparationCompleted(IBootstrappedNetworkPtr bootstrappedNetwork);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceSaltFetchSignedSaltQuery => IMessageMonitorResultDelegate<SignedSaltGetResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        SignedSaltGetResultPtr response
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             SignedSaltGetResultPtr response, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceSaltFetchSignedSaltQuery => (internal)
        #pragma mark

        RecursiveLock &getLock() const;

        Log::Params log(const char *message) const;
        Log::Params debug(const char *message) const;

        virtual ElementPtr toDebug() const;

        void step();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceSaltFetchSignedSaltQuery => (data)
        #pragma mark

        AutoPUID mID;
        mutable RecursiveLock mLock;
        ServiceSaltFetchSignedSaltQueryWeakPtr mThisWeak;

        IServiceSaltFetchSignedSaltQueryDelegatePtr mDelegate;

        UseBootstrappedNetworkPtr mBootstrappedNetwork;

        IMessageMonitorPtr mSaltMonitor;

        SaltBundleList mSaltBundles;

        UINT mTotalToFetch;

        WORD mLastError;
        String mLastErrorReason;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IServiceSaltFetchSignedSaltQueryFactory
      #pragma mark

      interaction IServiceSaltFetchSignedSaltQueryFactory
      {
        static IServiceSaltFetchSignedSaltQueryFactory &singleton();

        virtual ServiceSaltFetchSignedSaltQueryPtr fetchSignedSalt(
                                                                   IServiceSaltFetchSignedSaltQueryDelegatePtr delegate,
                                                                   IServiceSaltPtr serviceSalt,
                                                                   UINT totalToFetch = 1
                                                                   );
      };

    }
  }
}
