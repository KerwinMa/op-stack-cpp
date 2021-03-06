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

#include <openpeer/stack/internal/types.h>
#include <openpeer/stack/IBootstrappedNetwork.h>
#include <openpeer/stack/IMessageMonitor.h>
#include <openpeer/stack/IMessageSource.h>
#include <openpeer/stack/IServiceIdentity.h>

#include <openpeer/stack/message/identity/IdentityAccessLockboxUpdateResult.h>
#include <openpeer/stack/message/identity/IdentityLookupUpdateResult.h>
#include <openpeer/stack/message/identity/IdentityAccessRolodexCredentialsGetResult.h>
#include <openpeer/stack/message/identity-lookup/IdentityLookupResult.h>
#include <openpeer/stack/message/rolodex/RolodexAccessResult.h>
#include <openpeer/stack/message/rolodex/RolodexNamespaceGrantChallengeValidateResult.h>
#include <openpeer/stack/message/rolodex/RolodexContactsGetResult.h>

#include <openpeer/stack/internal/stack_ServiceNamespaceGrantSession.h>

#include <openpeer/services/IWakeDelegate.h>

#include <zsLib/Timer.h>
#include <zsLib/MessageQueueAssociator.h>

#include <list>

namespace openpeer
{
  namespace stack
  {
    namespace internal
    {
      interaction IBootstrappedNetworkForServices;
      interaction IServiceLockboxSessionForServiceIdentity;

      ZS_DECLARE_USING_PTR(message::identity, IdentityAccessLockboxUpdateResult)
      ZS_DECLARE_USING_PTR(message::identity, IdentityLookupUpdateResult)
      ZS_DECLARE_USING_PTR(message::identity, IdentityAccessRolodexCredentialsGetResult)

      ZS_DECLARE_USING_PTR(message::identity_lookup, IdentityLookupResult)
      
      ZS_DECLARE_USING_PTR(message::rolodex, RolodexAccessResult)
      ZS_DECLARE_USING_PTR(message::rolodex, RolodexNamespaceGrantChallengeValidateResult)
      ZS_DECLARE_USING_PTR(message::rolodex, RolodexContactsGetResult)

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IServiceIdentitySessionForServiceLockbox
      #pragma mark

      interaction IServiceIdentitySessionForServiceLockbox
      {
        ZS_DECLARE_TYPEDEF_PTR(IServiceIdentitySessionForServiceLockbox, ForLockbox)

        typedef IServiceIdentitySession::SessionStates SessionStates;

        static ElementPtr toDebug(ForLockboxPtr session);

        static ForLockboxPtr reload(
                                    BootstrappedNetworkPtr provider,
                                    IServiceNamespaceGrantSessionPtr grantSession,
                                    IServiceLockboxSessionPtr existingLockbox,
                                    const char *identityURI,
                                    const char *reloginKey
                                    );

        virtual PUID getID() const = 0;

        virtual SessionStates getState(
                                       WORD *outLastErrorCode = NULL,
                                       String *outLastErrorReason = NULL
                                       ) const = 0;

        virtual void associate(ServiceLockboxSessionPtr lockbox) = 0;
        virtual void killAssociation(ServiceLockboxSessionPtr lockbox) = 0;

        virtual bool isAssociated() const = 0;

        virtual void notifyStateChanged() = 0;

        virtual bool isLoginComplete() const = 0;
        virtual bool isShutdown() const = 0;

        virtual IdentityInfo getIdentityInfo() const = 0;
        virtual LockboxInfo getLockboxInfo() const = 0;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ServiceIdentitySession
      #pragma mark

      class ServiceIdentitySession : public Noop,
                                     public zsLib::MessageQueueAssociator,
                                     public IServiceIdentitySession,
                                     public IMessageSource,
                                     public IServiceIdentitySessionForServiceLockbox,
                                     public IWakeDelegate,
                                     public IBootstrappedNetworkDelegate,
                                     public zsLib::ITimerDelegate,
                                     public IServiceNamespaceGrantSessionWaitDelegate,
                                     public IServiceNamespaceGrantSessionQueryDelegate,
                                     public IMessageMonitorResultDelegate<IdentityAccessLockboxUpdateResult>,
                                     public IMessageMonitorResultDelegate<IdentityLookupUpdateResult>,
                                     public IMessageMonitorResultDelegate<IdentityAccessRolodexCredentialsGetResult>,
                                     public IMessageMonitorResultDelegate<IdentityLookupResult>,
                                     public IMessageMonitorResultDelegate<RolodexAccessResult>,
                                     public IMessageMonitorResultDelegate<RolodexNamespaceGrantChallengeValidateResult>,
                                     public IMessageMonitorResultDelegate<RolodexContactsGetResult>
      {
      public:
        friend interaction IServiceIdentitySessionFactory;
        friend interaction IServiceIdentitySession;

        ZS_DECLARE_TYPEDEF_PTR(RecursiveLock, UseRecursiveLock)

        ZS_DECLARE_TYPEDEF_PTR(IBootstrappedNetworkForServices, UseBootstrappedNetwork)
        ZS_DECLARE_TYPEDEF_PTR(IServiceLockboxSessionForServiceIdentity, UseServiceLockboxSession)
        ZS_DECLARE_TYPEDEF_PTR(IServiceNamespaceGrantSessionForServices, UseServiceNamespaceGrantSession)

        typedef IServiceNamespaceGrantSession::SessionStates GrantSessionStates;
        typedef IServiceIdentitySession::SessionStates SessionStates;

        typedef std::pair<DocumentPtr, MessagePtr> DocumentMessagePair;
        typedef std::list<DocumentMessagePair> DocumentList;

      protected:
        ServiceIdentitySession(
                               IMessageQueuePtr queue,
                               IServiceIdentitySessionDelegatePtr delegate,
                               UseBootstrappedNetworkPtr providerNetwork,
                               UseBootstrappedNetworkPtr identityNetwork,
                               ServiceNamespaceGrantSessionPtr grantSession,
                               UseServiceLockboxSessionPtr existingLockbox,
                               const char *outerFrameURLUponReload
                               );

        ServiceIdentitySession(Noop) :
          Noop(true),
          MessageQueueAssociator(IMessageQueuePtr()),
          mLockPtr(new RecursiveLock),
          mLock(*mLockPtr) {}

        void init();

      public:
        ~ServiceIdentitySession();

        static ServiceIdentitySessionPtr convert(IServiceIdentitySessionPtr object);
        static ServiceIdentitySessionPtr convert(ForLockboxPtr object);


      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IServiceIdentitySession
        #pragma mark

        static ElementPtr toDebug(IServiceIdentitySessionPtr session);

        static ServiceIdentitySessionPtr loginWithIdentity(
                                                           IServiceIdentitySessionDelegatePtr delegate,
                                                           IServiceIdentityPtr provider,
                                                           IServiceNamespaceGrantSessionPtr grantSession,
                                                           IServiceLockboxSessionPtr existingLockbox,
                                                           const char *outerFrameURLUponReload,
                                                           const char *identityURI_or_identityBaseURI
                                                           );

        static ServiceIdentitySessionPtr loginWithIdentityPreauthorized(
                                                                        IServiceIdentitySessionDelegatePtr delegate,
                                                                        IServiceIdentityPtr provider,
                                                                        IServiceNamespaceGrantSessionPtr grantSession,
                                                                        IServiceLockboxSessionPtr existingLockbox,  // pass NULL IServiceLockboxSessionPtr() if none exists
                                                                        const char *identityURI,
                                                                        const char *identityAccessToken,
                                                                        const char *identityAccessSecret,
                                                                        Time identityAccessSecretExpires
                                                                        );
        virtual PUID getID() const {return mID;}

        virtual IServiceIdentityPtr getService() const;

        virtual SessionStates getState(
                                       WORD *outLastErrorCode,
                                       String *outLastErrorReason
                                       ) const;

        virtual bool isDelegateAttached() const;
        virtual void attachDelegate(
                                    IServiceIdentitySessionDelegatePtr delegate,
                                    const char *outerFrameURLUponReload
                                    );
        virtual void attachDelegateAndPreauthorizeLogin(
                                                        IServiceIdentitySessionDelegatePtr delegate,
                                                        const char *identityAccessToken,
                                                        const char *identityAccessSecret,
                                                        Time identityAccessSecretExpires
                                                        );

        virtual String getIdentityURI() const;
        virtual String getIdentityProviderDomain() const;

        virtual void getIdentityInfo(IdentityInfo &outIdentityInfo) const;

        virtual String getInnerBrowserWindowFrameURL() const;

        virtual void notifyBrowserWindowVisible();
        virtual void notifyBrowserWindowClosed();

        virtual DocumentPtr getNextMessageForInnerBrowerWindowFrame();
        virtual void handleMessageFromInnerBrowserWindowFrame(DocumentPtr unparsedMessage);

        virtual void startRolodexDownload(const char *inLastDownloadedVersion = NULL);
        virtual void refreshRolodexContacts();
        virtual bool getDownloadedRolodexContacts(
                                                  bool &outFlushAllRolodexContacts,
                                                  String &outVersionDownloaded,
                                                  IdentityInfoListPtr &outRolodexContacts
                                                  );

        virtual void cancel();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageSource
        #pragma mark

        // (duplicate) virtual PUID getID() const;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IServiceIdentitySessionForServiceLockbox
        #pragma mark

        static ServiceIdentitySessionPtr reload(
                                                BootstrappedNetworkPtr provider,
                                                IServiceNamespaceGrantSessionPtr grantSession,
                                                IServiceLockboxSessionPtr existingLockbox,
                                                const char *identityURI,
                                                const char *reloginKey
                                                );

        // (duplicate) virtual PUID getID() const;

        // virtual SessionStates getState(
        //                                WORD *outLastErrorCode,
        //                                String *outLastErrorReason
        //                                ) const;

        virtual void associate(ServiceLockboxSessionPtr lockbox);
        virtual void killAssociation(ServiceLockboxSessionPtr lockbox);

        virtual bool isAssociated() const;

        virtual void notifyStateChanged();

        virtual bool isLoginComplete() const;
        virtual bool isShutdown() const;

        virtual IdentityInfo getIdentityInfo() const;
        virtual LockboxInfo getLockboxInfo() const;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IWakeDelegate
        #pragma mark

        virtual void onWake();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IBootstrappedNetworkDelegate
        #pragma mark

        virtual void onBootstrappedNetworkPreparationCompleted(IBootstrappedNetworkPtr bootstrappedNetwork);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => ITimerDelegate
        #pragma mark

        virtual void onTimer(TimerPtr timer);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IServiceNamespaceGrantSessionWaitDelegate
        #pragma mark

        virtual void onServiceNamespaceGrantSessionForServicesWaitComplete(IServiceNamespaceGrantSessionPtr session);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IServiceNamespaceGrantSessionQueryDelegate
        #pragma mark

        virtual void onServiceNamespaceGrantSessionForServicesQueryComplete(
                                                                            IServiceNamespaceGrantSessionQueryPtr query,
                                                                            ElementPtr namespaceGrantChallengeBundleEl
                                                                            );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<IdentityAccessLockboxUpdateResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        IdentityAccessLockboxUpdateResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             IdentityAccessLockboxUpdateResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<IdentityLookupUpdateResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        IdentityLookupUpdateResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             IdentityLookupUpdateResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<IdentityAccessRolodexCredentialsGetResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        IdentityAccessRolodexCredentialsGetResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             IdentityAccessRolodexCredentialsGetResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<IdentityLookupResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        IdentityLookupResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             IdentityLookupResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<RolodexAccessResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        RolodexAccessResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             RolodexAccessResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<RolodexNamespaceGrantChallengeValidateResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        RolodexNamespaceGrantChallengeValidateResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             RolodexNamespaceGrantChallengeValidateResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => IMessageMonitorResultDelegate<RolodexContactsGetResult>
        #pragma mark

        virtual bool handleMessageMonitorResultReceived(
                                                        IMessageMonitorPtr monitor,
                                                        RolodexContactsGetResultPtr result
                                                        );

        virtual bool handleMessageMonitorErrorResultReceived(
                                                             IMessageMonitorPtr monitor,
                                                             RolodexContactsGetResultPtr ignore, // will always be NULL
                                                             message::MessageResultPtr result
                                                             );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => (internal)
        #pragma mark

        RecursiveLock &getLock() const;

        Log::Params log(const char *message) const;
        Log::Params debug(const char *message) const;

        virtual ElementPtr toDebug() const;

        void step();

        bool stepBootstrapper();
        bool stepGrantCheck();
        bool stepLoadBrowserWindow();
        bool stepMakeBrowserWindowVisible();
        bool stepIdentityAccessStartNotification();
        bool stepIdentityAccessCompleteNotification();
        bool stepRolodexCredentialsGet();
        bool stepRolodexAccess();
        bool stepLockboxAssociation();
        bool stepIdentityLookup();
        bool stepLockboxAccessToken();
        bool stepLockboxUpdate();
        bool stepCloseBrowserWindow();
        bool stepPreGrantChallenge();
        bool stepClearGrantWait();
        bool stepGrantChallenge();
        bool stepLockboxReady();
        bool stepLookupUpdate();
        bool stepDownloadContacts();

        void setState(SessionStates state);
        void setError(WORD errorCode, const char *reason = NULL);
        void notifyLockboxStateChanged();
        void sendInnerWindowMessage(MessagePtr message);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ServiceIdentitySession => (data)
        #pragma mark

        AutoPUID mID;
        mutable UseRecursiveLockPtr mLockPtr;
        UseRecursiveLockPtr mAssociatedLockPtr;
        UseRecursiveLock &mLock;
        ServiceIdentitySessionWeakPtr mThisWeak;
        ServiceIdentitySessionPtr mGraciousShutdownReference;

        SessionStates mCurrentState;
        SessionStates mLastReportedState;

        AutoWORD mLastError;
        String mLastErrorReason;
        
        IServiceIdentitySessionDelegatePtr mDelegate;
        UseServiceLockboxSessionWeakPtr mAssociatedLockbox;
        AutoBool mKillAssociation;

        IdentityInfo mIdentityInfo;

        UseBootstrappedNetworkPtr mProviderBootstrappedNetwork;
        UseBootstrappedNetworkPtr mIdentityBootstrappedNetwork;
        UseBootstrappedNetworkPtr mActiveBootstrappedNetwork;

        UseServiceNamespaceGrantSessionPtr mGrantSession;
        IServiceNamespaceGrantSessionQueryPtr mGrantQuery;
        IServiceNamespaceGrantSessionWaitPtr mGrantWait;

        IMessageMonitorPtr mIdentityAccessLockboxUpdateMonitor;
        IMessageMonitorPtr mIdentityLookupUpdateMonitor;
        IMessageMonitorPtr mIdentityAccessRolodexCredentialsGetMonitor;
        IMessageMonitorPtr mIdentityLookupMonitor;
        IMessageMonitorPtr mRolodexAccessMonitor;
        IMessageMonitorPtr mRolodexNamespaceGrantChallengeValidateMonitor;
        IMessageMonitorPtr mRolodexContactsGetMonitor;

        LockboxInfo mLockboxInfo;

        AutoBool mBrowserWindowReady;
        AutoBool mBrowserWindowVisible;
        AutoBool mBrowserWindowClosed;

        AutoBool mNeedsBrowserWindowVisible;

        AutoBool mIdentityAccessStartNotificationSent;
        AutoBool mLockboxUpdated;
        AutoBool mIdentityLookupUpdated;
        IdentityInfo mPreviousLookupInfo;

        String mOuterFrameURLUponReload;

        DocumentList mPendingMessagesToDeliver;

        // rolodex related
        AutoBool mRolodexNotSupportedForIdentity;
        RolodexInfo mRolodexInfo;

        TimerPtr mTimer;

        String mFrozenVersion;

        String mLastVersionDownloaded;
        Time mForceRefresh;

        Time mFreshDownload;
        IdentityInfoList mIdentities;

        AutoULONG mFailuresInARow;
        Duration mNextRetryAfterFailureTime;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IServiceIdentitySessionFactory
      #pragma mark

      interaction IServiceIdentitySessionFactory
      {
        static IServiceIdentitySessionFactory &singleton();

        virtual ServiceIdentitySessionPtr loginWithIdentity(
                                                            IServiceIdentitySessionDelegatePtr delegate,
                                                            IServiceIdentityPtr provider,
                                                            IServiceNamespaceGrantSessionPtr grantSession,
                                                            IServiceLockboxSessionPtr existingLockbox,
                                                            const char *outerFrameURLUponReload,
                                                            const char *identityURI_or_identityBaseURI
                                                            );

        virtual ServiceIdentitySessionPtr loginWithIdentityPreauthorized(
                                                                         IServiceIdentitySessionDelegatePtr delegate,
                                                                         IServiceIdentityPtr provider,
                                                                         IServiceNamespaceGrantSessionPtr grantSession,
                                                                         IServiceLockboxSessionPtr existingLockbox,  // pass NULL IServiceLockboxSessionPtr() if none exists
                                                                         const char *identityURI,
                                                                         const char *identityAccessToken,
                                                                         const char *identityAccessSecret,
                                                                         Time identityAccessSecretExpires
                                                                         );

        virtual ServiceIdentitySessionPtr reload(
                                                 BootstrappedNetworkPtr provider,
                                                 IServiceNamespaceGrantSessionPtr grantSession,
                                                 IServiceLockboxSessionPtr existingLockbox,
                                                 const char *identityURI,
                                                 const char *reloginKey
                                                 );
      };
    }
  }
}
