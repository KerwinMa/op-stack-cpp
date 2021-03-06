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
#include <openpeer/services/IICESocket.h>

#include <list>
#include <map>

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      using zsLib::string;
      using zsLib::DWORD;
      using zsLib::XML::Document;
      using zsLib::XML::Element;
      using zsLib::XML::Text;
      using zsLib::XML::TextPtr;
      using zsLib::XML::Attribute;
      using zsLib::XML::AttributePtr;
      using zsLib::Singleton;
      using zsLib::SingletonLazySharedPtr;

      using boost::dynamic_pointer_cast;

      using services::IICESocket;
      using services::IRSAPrivateKey;
      using services::IRSAPrivateKeyPtr;
      using services::IRSAPublicKey;
      using services::IRSAPublicKeyPtr;
      using services::IDHKeyDomain;
      using services::IDHKeyDomainPtr;
      using services::IDHPrivateKey;
      using services::IDHPrivateKeyPtr;
      using services::IDHPublicKey;
      using services::IDHPublicKeyPtr;

      typedef std::list<String> RouteList;
      typedef std::list<String> StringList;

      typedef stack::LocationInfo LocationInfo;
      typedef stack::LocationInfoList LocationInfoList;

      ZS_DECLARE_STRUCT_PTR(Service)
      ZS_DECLARE_STRUCT_PTR(IdentityInfo)

      ZS_DECLARE_TYPEDEF_PTR(std::list<IdentityInfo>, IdentityInfoList)


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Service
      #pragma mark

      struct Service
      {
        ZS_DECLARE_STRUCT_PTR(Method)
        ZS_DECLARE_TYPEDEF_PTR(std::list<MethodPtr>, MethodList)

        typedef String ServiceID;
        typedef String Type;

        struct Method
        {
          typedef String MethodName;

          MethodName mName;
          String mURI;
          String mUsername;
          String mPassword;

          bool hasData() const;
          ElementPtr toDebug() const;
        };
        typedef std::map<Method::MethodName, Method> MethodMap;

        ServiceID mID;
        String mType;
        String mVersion;

        MethodMap mMethods;

        bool hasData() const;
        ElementPtr toDebug() const;
      };
      typedef std::map<Service::ServiceID, Service> ServiceMap;
      typedef std::map<Service::Type, ServiceMap> ServiceTypeMap;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Certificate
      #pragma mark

      struct Certificate
      {
        typedef String CertificateID;
        String mID;
        String mService;
        Time mExpires;
        IRSAPublicKeyPtr mPublicKey;

        bool hasData() const;
        ElementPtr toDebug() const;
      };
      typedef std::map<Certificate::CertificateID, Certificate> CertificateMap;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Finder
      #pragma mark

      struct Finder
      {
        struct Protocol
        {
          String mTransport;
          String mHost;
        };
        typedef std::list<Protocol> ProtocolList;

        String mID;
        ProtocolList mProtocols;
        IRSAPublicKeyPtr mPublicKey;
        WORD  mPriority;
        WORD  mWeight;
        String mRegion;
        Time mCreated;
        Time mExpires;

        Finder() :
          mPriority(0),
          mWeight(0)
        {}

        bool hasData() const;
        ElementPtr toDebug() const;
      };
      typedef std::list<Finder> FinderList;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IdentityInfo
      #pragma mark

      struct IdentityInfo
      {
        enum Dispositions
        {
          Disposition_NA, // not applicable
          Disposition_Update,
          Disposition_Remove,
        };

        struct Avatar
        {
          String mName;
          String mURL;
          int mWidth;
          int mHeight;

          Avatar() : mWidth(0), mHeight(0) {}
          bool hasData() const;
          ElementPtr toDebug() const;
        };
        typedef std::list<Avatar> AvatarList;

        static const char *toString(Dispositions diposition);
        static Dispositions toDisposition(const char *str);

        Dispositions mDisposition;

        String mAccessToken;
        String mAccessSecret;
        Time mAccessSecretExpires;
        String mAccessSecretProof;
        Time mAccessSecretProofExpires;

        String mReloginKey;

        String mBase;
        String mURI;
        String mProvider;

        String mStableID;
        IPeerFilePublicPtr mPeerFilePublic;

        WORD mPriority;
        WORD mWeight;

        Time mCreated;
        Time mUpdated;
        Time mExpires;

        String mName;
        String mProfile;
        String mVProfile;

        AvatarList mAvatars;

        ElementPtr mContactProofBundle;
        ElementPtr mIdentityProofBundle;

        IdentityInfo() : mDisposition(Disposition_NA), mPriority(0), mWeight(0) {}
        bool hasData() const;
        ElementPtr toDebug() const;

        void mergeFrom(
                       const IdentityInfo &source,
                       bool overwriteExisting = true
                       );
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LockboxInfo
      #pragma mark

      struct LockboxInfo
      {
        String mDomain;
        String mAccountID;

        String mAccessToken;
        String mAccessSecret;
        Time mAccessSecretExpires;
        String mAccessSecretProof;
        Time mAccessSecretProofExpires;

        SecureByteBlockPtr mKey;
        String mHash;

        bool mResetFlag;

        LockboxInfo() : mResetFlag(false) {}
        bool hasData() const;
        ElementPtr toDebug() const;

        void mergeFrom(
                       const LockboxInfo &source,
                       bool overwriteExisting = true
                       );
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark AgentInfo
      #pragma mark

      struct AgentInfo
      {
        String mUserAgent;
        String mName;
        String mImageURL;
        String mAgentURL;

        AgentInfo() {}
        bool hasData() const;
        ElementPtr toDebug() const;

        void mergeFrom(
                       const AgentInfo &source,
                       bool overwriteExisting = true
                       );
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark NamespaceGrantChallengeInfo
      #pragma mark

      struct NamespaceGrantChallengeInfo
      {
        String mID;
        String mName;
        String mImageURL;
        String mServiceURL;
        String mDomains;

        NamespaceGrantChallengeInfo() {}
        bool hasData() const;
        ElementPtr toDebug() const;

        void mergeFrom(
                       const NamespaceGrantChallengeInfo &source,
                       bool overwriteExisting = true
                       );
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark NamespaceInfo
      #pragma mark

      struct NamespaceInfo
      {
        typedef String NamespaceURL;

        NamespaceURL mURL;
        Time mLastUpdated;

        NamespaceInfo() {}
        bool hasData() const;
        ElementPtr toDebug() const;

        void mergeFrom(
                       const NamespaceInfo &source,
                       bool overwriteExisting = true
                       );
      };

      typedef std::map<NamespaceInfo::NamespaceURL, NamespaceInfo> NamespaceInfoMap;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RolodexInfo
      #pragma mark

      struct RolodexInfo
      {
        String mServerToken;

        String mAccessToken;
        String mAccessSecret;
        Time mAccessSecretExpires;
        String mAccessSecretProof;
        Time mAccessSecretProofExpires;

        String mVersion;
        Time mUpdateNext;

        bool mRefreshFlag;

        RolodexInfo() : mRefreshFlag(false) {}
        bool hasData() const;
        ElementPtr toDebug() const;

        void mergeFrom(
                       const RolodexInfo &source,
                       bool overwriteExisting = true
                       );
      };
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (forwards)
      #pragma mark

      ZS_DECLARE_CLASS_PTR(Message)
      ZS_DECLARE_CLASS_PTR(MessageRequest)
      ZS_DECLARE_CLASS_PTR(MessageResult)
      ZS_DECLARE_CLASS_PTR(MessageNotify)

      ZS_DECLARE_INTERACTION_PTR(IMessageHelper)
      ZS_DECLARE_INTERACTION_PTR(IMessageFactory)


      namespace bootstrapper
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryBootstrapper)
        ZS_DECLARE_CLASS_PTR(ServicesGetRequest)
        ZS_DECLARE_CLASS_PTR(ServicesGetResult)
      }

      namespace bootstrapped_finder
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryBootstrappedFinder)
        ZS_DECLARE_CLASS_PTR(FindersGetRequest)
        ZS_DECLARE_CLASS_PTR(FindersGetResult)
      }

      namespace certificates
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryCertificates)
        ZS_DECLARE_CLASS_PTR(CertificatesGetRequest)
        ZS_DECLARE_CLASS_PTR(CertificatesGetResult)
      }

      namespace identity
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryIdentity)
        ZS_DECLARE_CLASS_PTR(IdentityAccessWindowRequest)
        ZS_DECLARE_CLASS_PTR(IdentityAccessWindowResult)
        ZS_DECLARE_CLASS_PTR(IdentityAccessStartNotify)
        ZS_DECLARE_CLASS_PTR(IdentityAccessCompleteNotify)
        ZS_DECLARE_CLASS_PTR(IdentityAccessLockboxUpdateRequest)
        ZS_DECLARE_CLASS_PTR(IdentityAccessLockboxUpdateResult)
        ZS_DECLARE_CLASS_PTR(IdentityAccessRolodexCredentialsGetRequest)
        ZS_DECLARE_CLASS_PTR(IdentityAccessRolodexCredentialsGetResult)
        ZS_DECLARE_CLASS_PTR(IdentityAccessValidateRequest)
        ZS_DECLARE_CLASS_PTR(IdentityAccessValidateResult)
        ZS_DECLARE_CLASS_PTR(IdentityLookupUpdateRequest)
        ZS_DECLARE_CLASS_PTR(IdentityLookupUpdateResult)
      }

      namespace identity_lockbox
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryIdentityLockbox)
        ZS_DECLARE_CLASS_PTR(LockboxAccessRequest)
        ZS_DECLARE_CLASS_PTR(LockboxAccessResult)
        ZS_DECLARE_CLASS_PTR(LockboxAccessValidateRequest)
        ZS_DECLARE_CLASS_PTR(LockboxAccessValidateResult)
        ZS_DECLARE_CLASS_PTR(LockboxNamespaceGrantChallengeValidateRequest)
        ZS_DECLARE_CLASS_PTR(LockboxNamespaceGrantChallengeValidateResult)
        ZS_DECLARE_CLASS_PTR(LockboxIdentitiesUpdateRequest)
        ZS_DECLARE_CLASS_PTR(LockboxIdentitiesUpdateResult)
        ZS_DECLARE_CLASS_PTR(LockboxContentGetRequest)
        ZS_DECLARE_CLASS_PTR(LockboxContentGetResult)
        ZS_DECLARE_CLASS_PTR(LockboxContentSetRequest)
        ZS_DECLARE_CLASS_PTR(LockboxContentSetResult)
      }

      namespace identity_lookup
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryIdentityLookup)
        ZS_DECLARE_CLASS_PTR(IdentityLookupCheckRequest)
        ZS_DECLARE_CLASS_PTR(IdentityLookupCheckResult)
        ZS_DECLARE_CLASS_PTR(IdentityLookupRequest)
        ZS_DECLARE_CLASS_PTR(IdentityLookupResult)
      }

      namespace namespace_grant
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryNamespaceGrant)
        ZS_DECLARE_CLASS_PTR(NamespaceGrantWindowRequest)
        ZS_DECLARE_CLASS_PTR(NamespaceGrantWindowResult)
        ZS_DECLARE_CLASS_PTR(NamespaceGrantStartNotify)
        ZS_DECLARE_CLASS_PTR(NamespaceGrantCompleteNotify)
      }
      
      namespace rolodex
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryRolodex)
        ZS_DECLARE_CLASS_PTR(RolodexAccessRequest)
        ZS_DECLARE_CLASS_PTR(RolodexAccessResult)
        ZS_DECLARE_CLASS_PTR(RolodexNamespaceGrantChallengeValidateRequest)
        ZS_DECLARE_CLASS_PTR(RolodexNamespaceGrantChallengeValidateResult)
        ZS_DECLARE_CLASS_PTR(RolodexContactsGetRequest)
        ZS_DECLARE_CLASS_PTR(RolodexContactsGetResult)
      }

      namespace peer
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryPeer)
        ZS_DECLARE_CLASS_PTR(PeerServicesGetRequest)
        ZS_DECLARE_CLASS_PTR(PeerServicesGetResult)
      }

      namespace peer_common
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryPeerCommon)
        ZS_DECLARE_CLASS_PTR(PeerPublishRequest)
        ZS_DECLARE_CLASS_PTR(PeerPublishResult)
        ZS_DECLARE_CLASS_PTR(PeerGetRequest)
        ZS_DECLARE_CLASS_PTR(PeerGetResult)
        ZS_DECLARE_CLASS_PTR(PeerDeleteRequest)
        ZS_DECLARE_CLASS_PTR(PeerDeleteResult)
        ZS_DECLARE_CLASS_PTR(PeerSubscribeRequest)
        ZS_DECLARE_CLASS_PTR(PeerSubscribeResult)
        ZS_DECLARE_CLASS_PTR(PeerPublishNotify)
      }

      namespace peer_finder
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryPeerFinder)
        ZS_DECLARE_CLASS_PTR(ChannelMapRequest)
        ZS_DECLARE_CLASS_PTR(ChannelMapResult)
        ZS_DECLARE_CLASS_PTR(ChannelMapNotify)
        ZS_DECLARE_CLASS_PTR(PeerLocationFindRequest)
        ZS_DECLARE_CLASS_PTR(PeerLocationFindResult)
        ZS_DECLARE_CLASS_PTR(PeerLocationFindNotify)
        ZS_DECLARE_CLASS_PTR(SessionCreateRequest)
        ZS_DECLARE_CLASS_PTR(SessionCreateResult)
        ZS_DECLARE_CLASS_PTR(SessionDeleteRequest)
        ZS_DECLARE_CLASS_PTR(SessionDeleteResult)
        ZS_DECLARE_CLASS_PTR(SessionKeepAliveRequest)
        ZS_DECLARE_CLASS_PTR(SessionKeepAliveResult)
      }

      namespace peer_salt
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryPeerSalt)
        ZS_DECLARE_CLASS_PTR(SignedSaltGetRequest)
        ZS_DECLARE_CLASS_PTR(SignedSaltGetResult)

        typedef ElementPtr SignedSaltElementPtr;
        typedef std::list<SignedSaltElementPtr> SaltBundleList;
      }

      namespace peer_to_peer
      {
        ZS_DECLARE_CLASS_PTR(MessageFactoryPeerToPeer)
        ZS_DECLARE_CLASS_PTR(PeerIdentifyRequest)
        ZS_DECLARE_CLASS_PTR(PeerIdentifyResult)
        ZS_DECLARE_CLASS_PTR(PeerKeepAliveRequest)
        ZS_DECLARE_CLASS_PTR(PeerKeepAliveResult)
      }
    }
  }
}
