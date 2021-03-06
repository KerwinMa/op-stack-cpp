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

#include <openpeer/stack/message/peer-finder/PeerLocationFindNotify.h>
#include <openpeer/stack/message/peer-finder/PeerLocationFindRequest.h>
#include <openpeer/stack/message/internal/stack_message_MessageHelper.h>
#include <openpeer/stack/internal/stack_Location.h>
#include <openpeer/stack/internal/stack_Peer.h>
#include <openpeer/stack/IPeerFiles.h>
#include <openpeer/stack/IPeerFilePublic.h>
#include <openpeer/stack/IPeerFilePrivate.h>

#include <zsLib/XML.h>
#include <zsLib/helpers.h>
#include <zsLib/Numeric.h>

namespace openpeer { namespace stack { namespace message { ZS_DECLARE_SUBSYSTEM(openpeer_stack_message) } } }

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      namespace peer_finder
      {
        ZS_DECLARE_TYPEDEF_PTR(stack::internal::ILocationForMessages, UseLocation)
        ZS_DECLARE_TYPEDEF_PTR(stack::internal::IPeerForMessages, UsePeer)

        typedef zsLib::XML::Exceptions::CheckFailed CheckFailed;

        using zsLib::Numeric;
        using message::internal::MessageHelper;

        using namespace stack::internal;

        //---------------------------------------------------------------------
        static Log::Params slog(const char *message)
        {
          return Log::Params(message, "PeerLocationFindNotify");
        }

        //---------------------------------------------------------------------
        PeerLocationFindNotifyPtr PeerLocationFindNotify::convert(MessagePtr message)
        {
          return dynamic_pointer_cast<PeerLocationFindNotify>(message);
        }

        //---------------------------------------------------------------------
        PeerLocationFindNotify::PeerLocationFindNotify()
        {
        }

        //---------------------------------------------------------------------
        PeerLocationFindNotifyPtr PeerLocationFindNotify::create(
                                                                 ElementPtr root,
                                                                 IMessageSourcePtr messageSource
                                                                 )
        {
          PeerLocationFindNotifyPtr ret(new PeerLocationFindNotify);
          IMessageHelper::fill(*ret, root, messageSource);

          try {

            ElementPtr findProofEl = root->findFirstChildElementChecked("findProofBundle")->findFirstChildElementChecked("findProof");

            ret->mRequestFindProofBundleDigestValue = IMessageHelper::getElementTextAndDecode(findProofEl->findFirstChildElementChecked("requestFindProofBundleDigestValue"));

            ret->mContext = IMessageHelper::getElementTextAndDecode(findProofEl->findFirstChildElementChecked("context"));

            try {
              get(ret->mValidated) = Numeric<bool>(IMessageHelper::getElementTextAndDecode(findProofEl->findFirstChildElementChecked("validated")));
            } catch (Numeric<bool>::ValueOutOfRange &) {
              ZS_LOG_WARNING(Detail, slog("final value missing"))
            }

            ret->mICEUsernameFrag = IMessageHelper::getElementTextAndDecode(findProofEl->findFirstChildElementChecked("iceUsernameFrag"));
            ret->mICEPassword = IMessageHelper::getElementTextAndDecode(findProofEl->findFirstChildElementChecked("icePassword"));

            try {
              get(ret->mFinal) = Numeric<bool>(IMessageHelper::getElementTextAndDecode(findProofEl->findFirstChildElementChecked("iceFinal")));
            } catch (Numeric<bool>::ValueOutOfRange &) {
              ZS_LOG_WARNING(Detail, slog("final value missing"))
            }

            ElementPtr locationEl = findProofEl->findFirstChildElement("location");
            if (locationEl) {
              ret->mLocationInfo = internal::MessageHelper::createLocation(locationEl, messageSource);
            }

            if (!ret->mLocationInfo) {
              ZS_LOG_ERROR(Detail, slog("missing location information in find request"))
              return PeerLocationFindNotifyPtr();
            }

            if (!ret->mLocationInfo->mLocation) {
              ZS_LOG_ERROR(Detail, slog("missing location information in find request"))
              return PeerLocationFindNotifyPtr();
            }

            UsePeerPtr peer = UseLocationPtr(Location::convert(ret->mLocationInfo->mLocation))->getPeer();

            if (!peer) {
              ZS_LOG_WARNING(Detail, slog("expected element is missing"))
              return PeerLocationFindNotifyPtr();
            }

            if (!peer->verifySignature(findProofEl)) {
              ZS_LOG_WARNING(Detail, slog("could not validate signature of find proof request"))
              return PeerLocationFindNotifyPtr();
            }

          } catch(CheckFailed &) {
            ZS_LOG_WARNING(Detail, slog("expected element is missing"))
            return PeerLocationFindNotifyPtr();
          }

          return ret;
        }

        //---------------------------------------------------------------------
        PeerLocationFindNotifyPtr PeerLocationFindNotify::create(PeerLocationFindRequestPtr request)
        {
          PeerLocationFindNotifyPtr ret(new PeerLocationFindNotify);

          ret->mDomain = request->domain();
          ret->mID = request->messageID();

          get(ret->mValidated) = request->didVerifySignature();

          if (request->hasAttribute(PeerLocationFindRequest::AttributeType_RequestfindProofBundleDigestValue)) {
            ret->mRequestFindProofBundleDigestValue = request->mRequestFindProofBundleDigestValue;
          }
          return ret;
        }

        //---------------------------------------------------------------------
        bool PeerLocationFindNotify::hasAttribute(AttributeTypes type) const
        {
          switch (type)
          {
            case AttributeType_RequestfindProofBundleDigestValue: return mRequestFindProofBundleDigestValue.hasData();
            case AttributeType_Context:                           return mContext.hasData();
            case AttributeType_ICEUsernameFrag:                   return mICEUsernameFrag.hasData();
            case AttributeType_ICEPassword:                       return mICEPassword.hasData();
            case AttributeType_ICEFinal:                          return mFinal;
            case AttributeType_LocationInfo:                      return mLocationInfo ? mLocationInfo->hasData() : false;
            case AttributeType_PeerFiles:                         return (bool)mPeerFiles;
            default:
              break;
          }
          return false;
        }

        //---------------------------------------------------------------------
        DocumentPtr PeerLocationFindNotify::encode()
        {
          DocumentPtr ret = IMessageHelper::createDocumentWithRoot(*this);
          ElementPtr root = ret->getFirstChildElement();

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

          ElementPtr findProofBundleEl = Element::create("findProofBundle");
          ElementPtr findProofEl = Element::create("findProof");

          if (hasAttribute(AttributeType_RequestfindProofBundleDigestValue))
          {
            findProofEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("requestFindProofBundleDigestValue", mRequestFindProofBundleDigestValue));
          }

          if (hasAttribute(AttributeType_Context)) {
            findProofEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("context", mContext));
          }

          findProofEl->adoptAsLastChild(IMessageHelper::createElementWithNumber("validated", mValidated ? "true":"false"));

          if (hasAttribute(AttributeType_ICEUsernameFrag)) {
            findProofEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("iceUsernameFrag", mICEUsernameFrag));
          }

          if (hasAttribute(AttributeType_ICEPassword)) {
            findProofEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("icePassword", mICEPassword));
          }

          findProofEl->adoptAsLastChild(IMessageHelper::createElementWithNumber("iceFinal", mFinal ? "true":"false"));

          if (hasAttribute(AttributeType_LocationInfo)) {
            findProofEl->adoptAsLastChild(MessageHelper::createElement(*mLocationInfo));
          }

          findProofBundleEl->adoptAsLastChild(findProofEl);
          peerFilePrivate->signElement(findProofEl);
          root->adoptAsLastChild(findProofBundleEl);

          return ret;
        }

      }
    }
  }
}
