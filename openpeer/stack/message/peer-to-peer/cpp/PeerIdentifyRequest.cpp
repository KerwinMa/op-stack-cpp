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

#include <openpeer/stack/message/peer-to-peer/PeerIdentifyRequest.h>
#include <openpeer/stack/message/internal/stack_message_MessageHelper.h>
#include <openpeer/stack/IPeerFilePublic.h>
#include <openpeer/stack/IPeerFilePrivate.h>
#include <openpeer/stack/IPeerFiles.h>
#include <openpeer/stack/internal/stack_Account.h>
#include <openpeer/stack/internal/stack_Location.h>
#include <openpeer/stack/internal/stack_Peer.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Stringize.h>
#include <zsLib/helpers.h>
#include <zsLib/XML.h>

#define OPENPEER_STACK_MESSAGE_PEER_IDENTIFY_REQUEST_LIFETIME_IN_SECONDS ((60*60)*24)

namespace openpeer { namespace stack { namespace message { ZS_DECLARE_SUBSYSTEM(openpeer_stack_message) } } }

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      using services::IHelper;

      namespace peer_to_peer
      {
        typedef stack::internal::ILocationForMessages UseLocation;
        typedef shared_ptr<UseLocation> UseLocationPtr;
        typedef weak_ptr<UseLocation> UseLocationWeakPtr;

        typedef stack::internal::IPeerForMessages UsePeer;
        typedef shared_ptr<UsePeer> UsePeerPtr;
        typedef weak_ptr<UsePeer> UsePeerWeakPtr;

        using zsLib::Seconds;
        using namespace stack::internal;
        using namespace message::internal;

        typedef zsLib::XML::Exceptions::CheckFailed CheckFailed;

        //---------------------------------------------------------------------
        static Log::Params slog(const char *message)
        {
          return Log::Params(message, "PeerIdentifyRequest");
        }

        //---------------------------------------------------------------------
        PeerIdentifyRequestPtr PeerIdentifyRequest::convert(MessagePtr message)
        {
          return boost::dynamic_pointer_cast<PeerIdentifyRequest>(message);
        }

        //---------------------------------------------------------------------
        PeerIdentifyRequest::PeerIdentifyRequest()
        {
          mAppID.clear();
        }

        //---------------------------------------------------------------------
        PeerIdentifyRequestPtr PeerIdentifyRequest::create()
        {
          PeerIdentifyRequestPtr ret(new PeerIdentifyRequest);
          return ret;
        }

        //---------------------------------------------------------------------
        PeerIdentifyRequestPtr PeerIdentifyRequest::create(
                                                           ElementPtr root,
                                                           IMessageSourcePtr messageSource
                                                           )
        {
          PeerIdentifyRequestPtr ret(new PeerIdentifyRequest);

          try
          {
            ret->mID = IMessageHelper::getAttributeID(root);

            ElementPtr peerIdentityProofBundleEl = root->findFirstChildElementChecked("peerIdentityProofBundle");
            ElementPtr peerIdentityProofEl = peerIdentityProofBundleEl->findFirstChildElementChecked("peerIdentityProof");

            ElementPtr peerEl = peerIdentityProofEl->findFirstChildElementChecked("peer");
            ret->mPeerFilePublic = IPeerFilePublic::loadFromElement(peerEl);
            if (!ret->mPeerFilePublic) {
              ZS_LOG_WARNING(Detail, slog("missing remote peer information"))
              return PeerIdentifyRequestPtr();
            }

            UseLocationPtr locationSource = UseLocation::convert(messageSource);
            if (!locationSource) {
              ZS_LOG_WARNING(Detail, slog("message source could not be identified"))
              return PeerIdentifyRequestPtr();
            }

            AccountPtr account = locationSource->getAccount();
            if (!account) {
              ZS_LOG_WARNING(Detail, slog("account gone so peer identify request cannot be verified"))
              return PeerIdentifyRequestPtr();
            }

            ILocationPtr localLocation = ILocation::getForLocal(account);
            if (!account) {
              ZS_LOG_ERROR(Detail, slog("local location is null"))
              return PeerIdentifyRequestPtr();
            }

            Time expires = IHelper::stringToTime(peerIdentityProofEl->findFirstChildElementChecked("expires")->getText());
            if (zsLib::now() > expires) {
              ZS_LOG_WARNING(Detail, slog("request expired") + ZS_PARAM("expires", expires) + ZS_PARAM("now", ::zsLib::now()))
              return PeerIdentifyRequestPtr();
            }

            ret->mFindSecret = IMessageHelper::getElementTextAndDecode(peerIdentityProofEl->findFirstChildElement("findSecret"));
            ret->mLocationInfo = MessageHelper::createLocation(peerIdentityProofEl->findFirstChildElement("location"), messageSource);

            UsePeerPtr remotePeer = IPeerForMessages::create(account, ret->mPeerFilePublic);
            if (!remotePeer) {
              ZS_LOG_ERROR(Detail, slog("remote peer object could not be created"))
              return PeerIdentifyRequestPtr();
            }

            UseLocationPtr location;
            if (ret->mLocationInfo) {
              location = Location::convert(ret->mLocationInfo->mLocation);
            }

            if (!location) {
              ZS_LOG_WARNING(Detail, slog("remote location object could not be created"))
              return PeerIdentifyRequestPtr();
            }

            if (location->getPeerURI() != remotePeer->getPeerURI()) {
              ZS_LOG_WARNING(Detail, slog("location peer does not match public peer file") +  ILocation::toDebug(Location::convert(location)) + IPeer::toDebug(Peer::convert(remotePeer)))
              return PeerIdentifyRequestPtr();
            }

            UsePeerPtr signaturePeer = UsePeer::getFromSignature(account, peerIdentityProofEl);
            if (!signaturePeer) {
              ZS_LOG_WARNING(Detail, slog("signature peer is null"))
              return PeerIdentifyRequestPtr();
            }

            if (signaturePeer->getID() != remotePeer->getID()) {
              ZS_LOG_WARNING(Detail, slog("signature peer does not match identity peer") + ZS_PARAM("signature peer", IPeer::toDebug(Peer::convert(signaturePeer))) + ZS_PARAM("request peer", IPeer::toDebug(Peer::convert(remotePeer))))
              return PeerIdentifyRequestPtr();
            }

            if (!signaturePeer->verifySignature(peerIdentityProofEl)) {
              ZS_LOG_WARNING(Detail, slog("signature does not validate"))
              return PeerIdentifyRequestPtr();
            }

          } catch (CheckFailed &) {
            ZS_LOG_WARNING(Detail, slog("failed to obtain information"))
            return PeerIdentifyRequestPtr();
          }
          return ret;
        }

        //---------------------------------------------------------------------
        bool PeerIdentifyRequest::hasAttribute(AttributeTypes type) const
        {
          switch (type)
          {
            case AttributeType_LocationInfo:    return mLocationInfo ? mLocationInfo->hasData() : false;
            case AttributeType_FindSecret:      return (!mFindSecret.isEmpty());
            case AttributeType_PeerFilePublic:  return (bool)mPeerFilePublic;
            case AttributeType_PeerFiles:       return (bool)mPeerFiles;
            default:                            break;
          }
          return false;
        }

        //---------------------------------------------------------------------
        DocumentPtr PeerIdentifyRequest::encode()
        {
          if (!mPeerFiles) {
            ZS_LOG_ERROR(Detail, slog("peer files was null"))
            return DocumentPtr();
          }

          IPeerFilePrivatePtr peerFilePrivate = mPeerFiles->getPeerFilePrivate();
          if (!peerFilePrivate) {
            ZS_LOG_ERROR(Detail, slog("peer file private was null"))
            return DocumentPtr();
          }
          IPeerFilePublicPtr peerFilePublic = mPeerFiles->getPeerFilePublic();
          if (!peerFilePublic) {
            ZS_LOG_ERROR(Detail, slog("peer file public was null"))
            return DocumentPtr();
          }

          DocumentPtr ret = IMessageHelper::createDocumentWithRoot(*this);
          ElementPtr root = ret->getFirstChildElement();

          ElementPtr peerIdentityProofBundleEl = Element::create("peerIdentityProofBundle");
          ElementPtr peerIdentityProofEl = Element::create("peerIdentityProof");

          peerIdentityProofEl->adoptAsLastChild(IMessageHelper::createElementWithText("nonce", IHelper::randomString(32)));

          Time expires = zsLib::now() + Duration(Seconds(OPENPEER_STACK_MESSAGE_PEER_IDENTIFY_REQUEST_LIFETIME_IN_SECONDS));
          peerIdentityProofEl->adoptAsLastChild(IMessageHelper::createElementWithText("expires", IHelper::timeToString(expires)));

          if (hasAttribute(AttributeType_FindSecret)) {
            peerIdentityProofEl->adoptAsLastChild(IMessageHelper::createElementWithText("findSecret", mFindSecret));
          }

          if (hasAttribute(AttributeType_LocationInfo)) {
            peerIdentityProofEl->adoptAsLastChild(MessageHelper::createElement(*mLocationInfo));
          }

          peerIdentityProofEl->adoptAsLastChild(peerFilePublic->saveToElement());

          peerIdentityProofBundleEl->adoptAsLastChild(peerIdentityProofEl);
          peerFilePrivate->signElement(peerIdentityProofEl);
          root->adoptAsLastChild(peerIdentityProofBundleEl);
          return ret;
        }


      }
    }
  }
}
